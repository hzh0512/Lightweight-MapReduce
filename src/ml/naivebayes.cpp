#include "naivebayes.h"

namespace lmr
{
    namespace ml
    {
        void string_to_vector(const string& s, size_t left, size_t right, const char c, vector<string>& res)
        {
            size_t index = left, temp = left;
            res.clear();
            while (true)
            {
                while (index < right && s[index] != c) index++;
                if (index == right) break;
                if (index > temp)
                    res.push_back(s.substr(temp, index - temp));
                temp = ++index;
            }
            if (temp < right)
                res.push_back(s.substr(temp, right - temp));
        }

        void default_NB_formatfunc(const string &input, vector<string> &y, vector<string> &x)
        {
            size_t index1 = input.find('\t'), index2 = input.find('\t', index1 + 1);
            string_to_vector(input, index1 + 1, index2, ',', y);
            string_to_vector(input, index2 + 1, input.size(), ' ', x);
        }

        FormatFunc naivebayes::func = default_NB_formatfunc;

        class NB_Train_Mapper : public Mapper
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

        REGISTER_MAPPER(NB_Train_Mapper)

        class NB_Train_Reducer : public Reducer
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

        REGISTER_REDUCER(NB_Train_Reducer)


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


        void naivebayes::train(const string& input, int num_input, MapReduceResult& result)
        {
            spec->mapper_class = "NB_Train_Mapper";
            spec->reducer_class = "NB_Train_Reducer";
            spec->input_format = input;
            spec->num_inputs = num_input;
            spec->output_format = trainingformat;

            mr->set_spec(spec);
            mr->work(result);
        }

        void naivebayes::predict(const string& input, int num_input, const string& output, MapReduceResult& result)
        {
            spec->mapper_class = "NB_Test_Mapper1";
            spec->reducer_class = "NB_Test_Reducer1";
            spec->input_format = input;
            spec->num_inputs = num_input + spec->num_reducers;
            spec->output_format = output;

            if (index == 0)
            {
                char tmp1[1024], tmp2[1024];
                for (int i = 0; i < spec->num_reducers; ++i) {
                    snprintf(tmp1, 1024, trainingformat.c_str(), i);
                    snprintf(tmp2, 1024, input.c_str(), i + num_input);
                    unlink(tmp2);
                    symlink(tmp1, tmp2);
                }
            }

            mr->set_spec(spec);
            mr->work(result);

            if (index == 0)
            {
                char tmp1[1024];
                for (int i = 0; i < spec->num_reducers; ++i) {
                    snprintf(tmp1, 1024, input.c_str(), i + num_input);
                    unlink(tmp1);
                }
            }
        }
    }
}