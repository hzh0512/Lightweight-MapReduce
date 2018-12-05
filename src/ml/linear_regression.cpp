#include "linear_regression.h"
#include <iostream>

namespace lmr
{
    namespace ml
    {
        string LR_tmpdir = "lmr_lr_tmp/", LR_datafile = LR_tmpdir + "data.txt";

        vector<float> gaussin(vector<vector<float> > a, vector<float> b, int n)
        {
            for (int i = 0; i < n; i++) {
                if (a[i][i] == 0) {
                    cout << "can't use gaussin meathod" << endl;
                    return vector<float>();
                }
            }

            auto c = vector<float>(n);
            for (int k = 0; k < n - 1; k++) {
                for (int i = k + 1; i < n; i++)
                    c[i] = a[i][k] / a[k][k];

                for (int i = k + 1; i < n; i++) {
                    for (int j = 0; j < n; j++) {
                        a[i][j] = a[i][j] - c[i] * a[k][j];
                    }
                    b[i] = b[i] - c[i] * b[k];
                }
            }

            auto x = vector<float>(n);
            x[n - 1] = b[n - 1] / a[n - 1][n - 1];
            for (int i = n - 2; i >= 0; i--) {
                double sum = 0;
                for (int j = i + 1; j < n; j++) {
                    sum += a[i][j] * x[j];
                }
                x[i] = (b[i] - sum) / a[i][i];
            }

            return x;
        }

        vector<float> compute_beta(int dimension, int num_reducers, string output_format) {
            auto x_t_x = vector<vector<float> >(dimension);
            for (int i = 0; i < dimension; i++){
                x_t_x[i].resize(dimension);
            }
            auto x_t_y = vector<float>(dimension);

            for (int i = 0; i < num_reducers; i++){
                ifstream f;
                char* filename = new char[20];
                sprintf(filename, output_format.c_str(), i);
                f.open(filename);
                while (f.good()){
                    string line;
                    getline(f, line);
                    if (line.empty()) break;
                    auto tokens = string_split(line, '\t');
                    auto keys = string_split(tokens[1], '_');
                    if (keys.size() == 2){
                        x_t_y[stoi(keys[1])] = stof(tokens[2]);
                    } else {
                        x_t_x[stoi(keys[1])][stoi(keys[2])] = stof(tokens[2]);
                    }
                }
            }

            return gaussin(x_t_x, x_t_y, dimension);
        }

        void output_beta(vector<float> bt, string beta) {
            ofstream out;
            out.open(beta);
            for (int i = 0; i < bt.size() - 1; i++){
                out.write(to_string(bt[i]).c_str(), to_string(bt[i]).size());
                out.write(" ", 1);
            }
            out.write(to_string(bt[9]).c_str(), to_string(bt[9]).size());
            out.write("\n", 1);
        }

        void compute_test(string testfile, vector<float> bt) {
            ifstream f;
            f.open(testfile.c_str());
            while (f.good()){
                string line;
                getline(f, line);
                if (line.empty()) break;
                auto tokens = string_split(line, '\t');
                float label = 0;
                for (int i = 0; i < tokens.size() - 1; i++){
                    label += stof(tokens[i]) * bt[i];
                    printf("%s\t", tokens[i].c_str());
                }
                printf("%s\n", to_string(label).c_str());
            }
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
                string value;
                float result = 0.0;
                while (reduceInput->get_next_value(value)){
                    result += stof(value);
                }
                output(key, to_string(result));
            }
        };

        REGISTER_REDUCER(LR_Reducer);

        LinearRegression::LinearRegression(MapReduce* _mr){
            mr = _mr;
            spec = _mr->get_spec();
        }

        void LinearRegression::compute(const string &input, const string &beta, const string &testfile,
                                       lmr::MapReduceResult &result) {
            vector<vector<double>> c1, c2;
            double time = 0.f;
            spec->mapper_class = "LR_Mapper";
            spec->reducer_class = "LR_Reducer";
            spec->input_format = input;
            spec->output_format = "output_%d.txt";

            mr->set_spec(spec);
            mr->work(result);

            auto bt = compute_beta(10, spec->num_reducers, spec->output_format);
            output_beta(bt, beta);

            compute_test(testfile, bt);

            result.timeelapsed = time;
        }
    }
}