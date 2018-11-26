#include "naivebayes.h"

namespace lmr
{
    namespace ml
    {
        string NB_tmpdir = "lmr_nb_tmp/",
               NB_trainingformat = NB_tmpdir + "train_%d.txt",
               NB_trainingformat2 = NB_tmpdir + "train2.txt",
               NB_testingformat = NB_tmpdir + "test_%d.txt";

        inline void default_NB_formatfunc(const string &input, vector<string> &y, vector<string> &x)
        {
            size_t index1 = input.find('\t'), index2 = input.find('\t', index1 + 1);
            string_to_vector(input, index1 + 1, index2, ',', y);
            string_to_vector(input, index2 + 1, input.size(), ' ', x);
        }

        xy_FormatFunc naivebayes::func = default_NB_formatfunc;

        class NB_Train_Mapper1 : public Mapper
        {
        public:
            virtual void Map(const string& key, const string& value)
            {
                vector<string> y, x;
                naivebayes::func(value, y, x);
                for (auto& xi : x)
                {
                    auto& p = um[xi];
                    for (auto& yi : y)
                        p[yi]++;
                }
            }

            virtual void combine()
            {
                string temp;
                for (auto &p : um)
                {
                    temp.clear();
                    for (auto &q : p.second)
                        temp += q.first + "\t" + to_string(q.second) + "\t";
                    emit(p.first, temp);
                }
            }

        private:
            unordered_map<string, unordered_map<string, int>> um;
        };

        REGISTER_MAPPER(NB_Train_Mapper1)

        class NB_Train_Reducer1 : public Reducer
        {
        public:
            virtual void Reduce(const string& key, ReduceInput* reduceInput)
            {
                string value, cls, temp;
                istringstream is;
                int cnt = 0;
                unordered_map<string, int> um;
                while (reduceInput->get_next_value(value)){
                    is.str(value);
                    is.clear();
                    while (is >> cls >> cnt)
                        um[cls] += cnt;
                }
                for (auto &q : um)
                    temp += q.first + "\t" + to_string(q.second) + "\t";
                output(key, temp);
            }
        };

        REGISTER_REDUCER(NB_Train_Reducer1)


        class NB_Train_Mapper2 : public Mapper
        {
        public:
            virtual void Map(const string& key, const string& value)
            {
                vector<string> y, x;
                naivebayes::func(value, y, x);
                for (auto& yi : y)
                    emit(yi, to_string(x.size()));
            }
        };

        REGISTER_MAPPER(NB_Train_Mapper2)

        class NB_Train_Reducer2 : public Reducer
        {
        public:
            virtual void Reduce(const string& key, ReduceInput* reduceInput)
            {
                int cnt = 0;
                string value;
                while (reduceInput->get_next_value(value)){
                    y_sum++;
                    yi[key]++;
                    yi_word[key] += stoi(value);
                }
            }

            virtual void combine()
            {
                for (auto& p : yi)
                    output(p.first, to_string((double)p.second / (double)y_sum) + "\t" + to_string(yi_word[p.first]));
            }

        private:
            int y_sum = 0;
            unordered_map<string, int> yi, yi_word;
        };

        REGISTER_REDUCER(NB_Train_Reducer2)


        class NB_Test_Mapper1 : public Mapper
        {
        public:
            virtual void Map(const string& key, const string& value)
            {
                int cur = 0, is_trainfile = 0, key_len = 0;
                while (cur < value.size() && isdigit(value[cur])) cur++;
                if (cur && cur < value.size() && value[cur] == '\t' &&
                    value[cur + 1 + stoi(value.substr(0, cur))] == '\t')
                {
                    is_trainfile = 1;
                    key_len = stoi(value.substr(0, cur));
                }

                if (is_trainfile)
                {
                    emit(value.substr(cur + 1, key_len), "0" + value.substr(cur + 2 + key_len));
                }else{
                    vector<string> y, x;
                    naivebayes::func(value, y, x);
                    for (auto& xi : x)
                        emit(xi, "1" + key);
                }
            }
        };

        REGISTER_MAPPER(NB_Test_Mapper1)

        class NB_Test_Reducer1 : public Reducer
        {
        public:
            virtual void Reduce(const string& key, ReduceInput* reduceInput)
            {
                string train_stat, value;
                istringstream is;

                reduceInput->get_next_value(train_stat);
                if (train_stat[0] == '1') // not training sample
                {
                    output(train_stat.substr(1), key + "\t");
                    train_stat = "";
                }
                else
                    train_stat = train_stat.substr(1);

                while (reduceInput->get_next_value(value)){
                    output(value.substr(1), key + "\t" + train_stat);
                }
            }
        };

        REGISTER_REDUCER(NB_Test_Reducer1)


        class NB_Test_Mapper2 : public Mapper
        {
        public:
            virtual void Map(const string& key, const string& value)
            {
                int cur = 0, key_len = 0;
                while (cur < value.size() && isdigit(value[cur])) cur++;
                key_len = stoi(value.substr(0, cur));

                emit(value.substr(cur + 1, key_len), value.substr(cur + 2 + key_len));
            }
        };

        REGISTER_MAPPER(NB_Test_Mapper2)

        class NB_Test_Reducer2 : public Reducer
        {
        public:
            virtual void init()
            {
                string cls;
                int length, number_word, sum = 0;
                double yi_ratio;
                ifstream f(NB_trainingformat2);
                while (f >> length >> cls >> yi_ratio >> number_word)
                {
                    yi_word[cls] = number_word;
                    yi[cls] = yi_ratio;
                    sum += number_word;
                }
                p0 = 1.f / (double)sum;
            }

            virtual void Reduce(const string& key, ReduceInput* reduceInput)
            {
                string str, tmp, cls;
                istringstream is;
                int val = 0, sum = 0;
                unordered_map<string, double> um, um_tmp;

                for (auto& p : yi)
                    um[p.first] = log(p.second);

                while (reduceInput->get_next_value(str)){
                    um_tmp.clear();
                    is.str(str);
                    is.clear();
                    is >> tmp;  // word
                    while (is >> cls >> val)
                        um_tmp[cls] = val;
                    for (auto& p : yi_word)
                        um[p.first] += log((um_tmp[p.first] + p0)/(double)(p.second + 1));
                }

                string max_cls;
                double max_p = -1.e200, sum_p = 0.f;
                char tmp2[10];
                for (auto& p : um)
                {
                    if (p.second > max_p)
                    {
                        max_p = p.second;
                        max_cls = p.first;
                    }
                }

                for (auto& p : um)
                    sum_p += exp(p.second - max_p);
                snprintf(tmp2, 10, "\t%.3f", 1.f / sum_p);
                output(key, max_cls + tmp2);
            }

        private:
            double p0 = 0.f;
            unordered_map<string, int> yi_word;
            unordered_map<string, double> yi;
        };

        REGISTER_REDUCER(NB_Test_Reducer2)

        naivebayes::naivebayes(lmr::MapReduce *_mr, bool _keep_training)
                :mr(_mr), keep_training(_keep_training)
        {
            spec = _mr->get_spec();
            index = spec->index;
        }

        naivebayes::~naivebayes()
        {
            if (index == 0 && !keep_training)
                system(("rm -rf " + NB_tmpdir).c_str());
        }

        void naivebayes::train(const string& input, int num_input, MapReduceResult& result)
        {
            double time = 0.f;
            // get (Y=y and W=w)
            spec->mapper_class = "NB_Train_Mapper1";
            spec->reducer_class = "NB_Train_Reducer1";
            spec->input_format = input;
            spec->num_inputs = num_input;
            spec->output_format = NB_trainingformat;

            mr->set_spec(spec);
            mr->work(result);
            time += result.timeelapsed;

            // get (Y=any, Y=y, Y=y and W=any)
            int tmp = spec->num_reducers;
            spec->num_reducers = 1;
            spec->mapper_class = "NB_Train_Mapper2";
            spec->reducer_class = "NB_Train_Reducer2";
            spec->input_format = input;
            spec->num_inputs = num_input;
            spec->output_format = NB_trainingformat2;

            mr->set_spec(spec);
            mr->work(result);
            time += result.timeelapsed;
            spec->num_reducers = tmp;

            result.timeelapsed = time;
        }

        void naivebayes::predict(const string& input, int num_input, const string& output, MapReduceResult& result)
        {
            double time = 0.f;
            spec->mapper_class = "NB_Test_Mapper1";
            spec->reducer_class = "NB_Test_Reducer1";
            spec->input_format = input;
            spec->num_inputs = num_input + spec->num_reducers;
            spec->output_format = NB_testingformat;

            if (index == 0)
            {
                char tmp1[1024], tmp2[1024];
                for (int i = 0; i < spec->num_reducers; ++i) {
                    snprintf(tmp1, 1024, NB_trainingformat.c_str(), i);
                    snprintf(tmp2, 1024, input.c_str(), i + num_input);
                    unlink(tmp2);
                    symlink(tmp1, tmp2);
                }
            }

            mr->set_spec(spec);
            mr->work(result);
            time += result.timeelapsed;

            if (index == 0)
            {
                char tmp1[1024];
                for (int i = 0; i < spec->num_reducers; ++i) {
                    snprintf(tmp1, 1024, input.c_str(), i + num_input);
                    unlink(tmp1);
                }
            }

            spec->mapper_class = "NB_Test_Mapper2";
            spec->reducer_class = "NB_Test_Reducer2";
            spec->input_format = NB_testingformat;
            spec->num_inputs = spec->num_reducers;
            spec->output_format = output;

            mr->set_spec(spec);
            mr->work(result);
            time += result.timeelapsed;

            result.timeelapsed = time;
        }
    }
}