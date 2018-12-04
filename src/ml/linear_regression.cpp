//
// Created by Shiming Zhuang on 12/2/18.
//

#include "linear_regression.h"

namespace lmr
{
    namespace ml
    {
        string LR_tmpdir = "lmr_lr_tmp/", LR_datafile = LR_tmpdir + "data.txt";

        vector<string> string_split(const string &s, char delimiter)
        {
            vector<std::string> tokens;
            string token;
            istringstream tokenStream(s);
            while (getline(tokenStream, token, delimiter))
            {
                tokens.push_back(token);
            }
            return tokens;
        }

        class LR_Mapper : public Mapper{
        public:
            virtual void Map(const string& key, const string& value)
            {
                auto tokens = string_split(value, '\t');
                auto x = vector<float>(tokens.size() - 1);
                float y = stof(tokens[tokens.size()-1]);
                for (int i = 0; i < tokens.size() - 1; i++){
                    x[i] = stof(tokens[i]);
                }

                if (x_t_x.empty()){
                    x_t_x.resize(x.size());
                    for (int i = 0; i < x.size(); i++){
                        x_t_x[i].resize(x.size(), 0);
                    }
                    x_t_y.resize(x.size(), 0);
                }

                for (int i = 0; i < x.size(); i++){
                    x_t_y[i] += x[i] * y;
                    for (int j = 0; j < x.size(); j++){
                        x_t_x[i][j] += x[i] * x[j];
                    }
                }
            }

            virtual void combine(){
                string key;
                for (int i = 0; i < x_t_y.size(); i++){
                    for (int j = 0; j < x_t_y.size(); j++){
                        key = "x_" + to_string(i) + '_' + to_string(j);
                        emit("x_" + to_string(i) + '_' + to_string(j), to_string(x_t_x[i][j]));
                    }
                    emit("y_" + to_string(i), to_string(x_t_y[i]));
                }
            }

        private:
            vector<vector<float> > x_t_x;
            vector<float> x_t_y;
        };

        REGISTER_MAPPER(LR_Mapper);

        class LR_Reducer : public Reducer {
            virtual void Reduce(const string& key, ReduceInput* reduceInput)
            {
                string value, result = "0";
                while (reduceInput->get_next_value(value)){
                    result = to_string(stoi(value) + stoi(result));
                }
                output(key, result);
            }
        };

        REGISTER_REDUCER(LR_Reducer);

        LinearRegression::LinearRegression(MapReduce* _mr){
            mr = _mr;
            spec = _mr->get_spec();
        }

        void LinearRegression::compute(const string &input, int num_input, const string &beta,
                                       lmr::MapReduceResult &result) {
            vector<vector<double>> c1, c2;
            double time = 0.f;
            if (index == 0)
            {
                system(("mkdir -p " + LR_tmpdir).c_str());
                system(("cp -f " + LR_tmpdir + " " + LR_datafile).c_str());
            }

            spec->mapper_class = "LR_Mapper";
            spec->reducer_class = "LR_Reducer";
            spec->input_format = input;
            spec->num_inputs = num_input;
            spec->output_format = "output_%d.txt";

            mr->set_spec(spec);
            mr->work(result);

            result.timeelapsed = time;
        }
    }
}