#include "logistic_regression.h"


namespace lmr
{
    namespace ml
    {
        string LR_tmpdir = "lmr_lr_tmp/", LR_trainingfile = LR_tmpdir + "train_theta.txt";

        void read_theta(vector<float> &theta, string thetafile, int dimension)
        {
            ifstream f;
            f.open(thetafile);
            theta.resize(dimension);
            string line;
            for (int i = 0; i < dimension; i++){
                getline(f, line);
                try {
                    theta[i] = stof(line);
                } catch(...) {
                    fprintf(stderr, "Wrong line: %s.\n", line.c_str());
                    break;
                }
            }
            f.close();
        }

        void write_theta(const vector<float> &theta, string thetafile)
        {
            ofstream f(thetafile);
            string line;
            for (int i = 0; i < theta.size(); i++){
                f.write(to_string(theta[i]).c_str(), to_string(theta[i]).size());
                f.write("\n", 1);
            }
            f.close();
        }

        LogisticRegression::LogisticRegression(lmr::MapReduce *_mr, bool _keep_training){
            mr = _mr;
            spec = _mr->get_spec();
            index = spec->index;
        }

        LogisticRegression::~LogisticRegression()
        {
            if (index == 0 && !keep_training)
                system(("rm -rf " + LR_tmpdir).c_str());
        }


        class LogisticR_Mapper : public Mapper
        {
        public:
            virtual void init()
            {
                read_theta(theta, "lr/theta.txt", 39177);
                delta_theta.resize(theta.size(), 0.0);
                learning_rate = 0.1;
            }

            virtual void Map(const string& key, const string& value)
            {
                auto tokens = string_split(value, '\t');
                auto label = stof(tokens[0]);
                float sigma = theta[theta.size()-1];
                for (int i = 0; i < tokens.size() - 1; i++){
                    try {
                        auto p = string_split(tokens[i+1], ':');
                        sigma += theta[stoi(p[0])] * stof(p[1]);
                    } catch(...) {
                        sprintf("Invalid in Map: %s.\n", tokens[i+1].c_str());
                    }

                }

                float tmp = label - (1 / (1 + exp(-sigma)));
                delta_theta[delta_theta.size()-1] += tmp;
                for (int i = 0; i < tokens.size() - 1; i++){
                    auto p = string_split(tokens[i+1], ':');
                    auto pos = stoi(p[0]);
                    try {
                        delta_theta[pos] += tmp * stof(p[1]);
                    } catch (...) {
                        sprintf("Invalid in Delta: %s.\n", tokens[i+1].c_str());
                    }

                }
            }

            virtual void combine()
            {
                for (int i = 0; i < delta_theta.size(); i++){
                    emit(to_string(i), to_string(learning_rate * delta_theta[i]));
                }
            }

        private:
            vector<float> theta;
            vector<float> delta_theta;
            float learning_rate;
        };

        REGISTER_MAPPER(LogisticR_Mapper);

        class LogisticR_Reducer : public Reducer
        {
        public:
            virtual void Reduce(const string& key, ReduceInput* reduceInput)
            {
                float delta = 0.0;
                string value;
                while (reduceInput->get_next_value(value)){
                    delta += stof(value);
                }
                output(key, to_string(delta));
            }
        };

        REGISTER_REDUCER(LogisticR_Reducer)

        void update_theta(vector<float> &theta, const string &output_format, int num_reducers, int num_data, int dimension)
        {
            for (int j = 0; j < num_reducers; j++){
                ifstream f;
                char* filename = new char[20];
                sprintf(filename, output_format.c_str(), j);
                f.open(filename);
                for (int i = 0; i < dimension; i++) {
                    string line;
                    getline(f, line);
                    auto tokens = string_split(line, '\t');
                    auto key = stoi(tokens[1]);
                    theta[key] += stof(tokens[2]) / num_data;
                }
            }
        }

        void init_theta(const string &thetafile, int dimension)
        {
            ofstream f(thetafile);
            string line;
            for (int i = 0; i < dimension; i++){
                f.write("0.0\n", 4);
            }
            f.close();
        }

        void LogisticRegression::train(const string& input, int num_input, const string& thetafile, int num_data, int max_iter, MapReduceResult& result)
        {
            double time = 0.f;
            spec->num_reducers = 1;
            spec->mapper_class = "LogisticR_Mapper";
            spec->reducer_class = "LogisticR_Reducer";
            spec->input_format = input;
            spec->num_inputs = num_input;
            spec->output_format = "lr_tmp/output_%d.txt";

            mr->set_spec(spec);

            if (index == 0) {
                init_theta(thetafile, 39177);
            }

            for (int i = 0; i < max_iter; ++i)
            {
                printf("iteration %d\n", i + 1);
                mr->work(result);
                time += result.timeelapsed;
                if (index == 0) {
                    auto theta = vector<float>();
                    read_theta(theta, thetafile, 39177);
                    update_theta(theta, "lr_tmp/output_%d.txt", spec->num_reducers, num_data, 39177);
                    write_theta(theta, thetafile);
                }
            }

            result.timeelapsed = time;
        }

        void LogisticRegression::predict(const string &testfile, const string &thetafile, const string &output) {
            auto theta = vector<float>();
            read_theta(theta, thetafile, 39177);
            ifstream f;
            ofstream out;
            out.open(output);
            f.open(testfile);
            while (f.good()){
                string line;
                getline(f, line);
                if (line.empty()) break;
                auto tokens = string_split(line, '\t');
                float sigma = theta[theta.size()-1];
                for (int i = 1; i < tokens.size(); i++){
                    auto p = string_split(tokens[i], ':');
                    sigma += theta[stoi(p[0])] * stof(p[1]);
                }
                auto label = 1 / (1 + exp(-sigma)) > 0.5 ? 1 : 0;
                out.write(to_string(label).c_str(), 1);
                out.write("\n", 1);
            }
        }
    }
}