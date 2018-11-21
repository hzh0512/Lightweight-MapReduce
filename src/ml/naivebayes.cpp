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


        void naivebayes::train(const string& input, int num_input, MapReduceResult& result)
        {
            spec->mapper_class = "NB_Train_Mapper";
            spec->reducer_class = "NB_Train_Reducer";
            spec->input_format = input;
            spec->num_inputs = num_input;
            spec->output_format = trainingformat;

            MapReduce mr(spec, index);
            mr.work(result);
        }

    }
}