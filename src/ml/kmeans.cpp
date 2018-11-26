#include "kmeans.h"


namespace lmr
{
    namespace ml
    {
        string KMeans_tmpdir = "lmr_kmeans_tmp/",
               KMeans_trainingfile = KMeans_tmpdir + "train_centroids.txt";

        void get_centroids(vector<vector<double>>& centroids)
        {
            centroids.clear();
            ifstream f(KMeans_trainingfile);
            string s, tmp;
            double tmp2;
            int len = -1;
            vector<double> value;
            centroids.clear();
            while (getline(f, s)) {
                value.clear();
                istringstream is(s);
                is >> tmp >> tmp;
                while (is >> tmp2)
                    value.push_back(tmp2);
                centroids.emplace_back(value);
                if (len < 0) len = value.size();
                else if (len != value.size()) {
                    fprintf(stderr, "invalid centroids file.\n");
                    exit(1);
                }
            }
            if (centroids.empty()) {
                fprintf(stderr, "invalid centroids file.\n");
                exit(1);
            }
        }

        inline double diff_centroids(vector<vector<double>>& a, vector<vector<double>>& b)
        {
            double res = 0.f;
            for (int i = 0; i < a.size(); ++i)
            {
                for (int j = 0; j < a[i].size(); ++j)
                    res += abs(a[i][j] - b[i][j]);
            }
            return res;
        }

        inline void default_KMeans_formatfunc(const string &input, vector<string> &x)
        {
            size_t index = input.find(',');
            string_to_vector(input, index + 1, input.size(), ',', x);
        }

        x_FormatFunc kmeans::func = default_KMeans_formatfunc;
        DistanceFunc kmeans::distfunc = l2_distance;

        kmeans::kmeans(lmr::MapReduce *_mr, bool _keep_training)
                :mr(_mr), keep_training(_keep_training)
        {
            spec = _mr->get_spec();
            index = spec->index;
        }

        kmeans::~kmeans()
        {
            if (index == 0 && !keep_training)
                system(("rm -rf " + KMeans_tmpdir).c_str());
        }


        class KMeans_Train_Mapper : public Mapper
        {
        public:
            virtual void init()
            {
                get_centroids(centroids);
                new_centroids.clear();
                new_centroids.resize(centroids.size());
                for (int i = 0; i < new_centroids.size(); ++i)
                    new_centroids[i].first.resize(centroids[0].size());
            }

            virtual void Map(const string& key, const string& value)
            {
                vector<string> x;
                kmeans::func(value, x);
                vector<double> v;
                int min_index = -1;
                double min_dist = 1e200, dist;
                for (auto& xi : x) {
                    try{ v.push_back(stod(xi)); }
                    catch (invalid_argument &e){ return; }
                }
                for (int i = 0; i < centroids.size(); ++i) {
                    dist = kmeans::distfunc(centroids[i], v);
                    if (dist < min_dist) {
                        min_index = i;
                        min_dist = dist;
                    }
                }
                auto& p = new_centroids[min_index];
                p.second++;
                for (int i = 0; i < p.first.size(); ++i)
                    p.first[i] += v[i];
            }

            virtual void combine()
            {
                string temp;
                for (int i = 0; i < new_centroids.size(); ++i)
                {
                    if (new_centroids[i].second == 0)  // discard this cluster
                        continue;
                    temp = to_string(new_centroids[i].second);
                    for (auto& p : new_centroids[i].first)
                        temp += " " + to_string(p);
                    emit(to_string(i), temp);
                }
            }

        private:
            vector<vector<double>> centroids;
            vector<pair<vector<double>, int>> new_centroids;
        };

        REGISTER_MAPPER(KMeans_Train_Mapper)

        class KMeans_Train_Reducer : public Reducer
        {
        public:
            virtual void Reduce(const string& key, ReduceInput* reduceInput)
            {
                vector<double> sum;
                string value;
                int total = 0, tmp, index = 0;
                double tmp2;
                while (reduceInput->get_next_value(value)){
                    istringstream is(value);
                    index = 0;
                    is >> tmp;
                    total += tmp;
                    if (sum.empty()) {
                        while (is >> tmp2)
                            sum.push_back(tmp2);
                    }else{
                        while (is >> tmp2)
                            sum[index++] += tmp2;
                    }
                }
                value.clear();
                for (int i = 0; i < sum.size(); ++i)
                    value += to_string(sum[i] / (double)total) + " ";
                output(key, value);
            }
        };

        REGISTER_REDUCER(KMeans_Train_Reducer)

        class KMeans_Test_Mapper : public Mapper {
        public:
            virtual void init()
            {
                get_centroids(centroids);
            }

            virtual void Map(const string &key, const string &value)
            {
                vector<string> x;
                kmeans::func(value, x);
                vector<double> v;
                int min_index = -1;
                double min_dist = 1e200, dist;
                for (auto& xi : x) {
                    try { v.push_back(stod(xi)); }
                    catch (invalid_argument &e) { return; }
                }
                for (int i = 0; i < centroids.size(); ++i) {
                    dist = kmeans::distfunc(centroids[i], v);
                    if (dist < min_dist) {
                        min_index = i;
                        min_dist = dist;
                    }
                }
                emit(key, to_string(min_index));
            }

        private:
            vector<vector<double>> centroids;
        };

        REGISTER_MAPPER(KMeans_Test_Mapper)

        class KMeans_Test_Reducer : public Reducer
        {
        public:
            virtual void Reduce(const string& key, ReduceInput* reduceInput)
            {
                string value;
                reduceInput->get_next_value(value);
                output(key, value);
            }
        };

        REGISTER_REDUCER(KMeans_Test_Reducer)

        void kmeans::train(const string& input, int num_input, const string& centroids, double eps, int max_iter, MapReduceResult& result)
        {
            vector<vector<double>> c1, c2;
            double time = 0.f;
            if (index == 0)
            {
                system(("mkdir -p " + KMeans_tmpdir).c_str());
                system(("cp -f " + centroids + " " + KMeans_trainingfile).c_str());
            }

            int tmp = spec->num_reducers;
            spec->num_reducers = 1;
            spec->mapper_class = "KMeans_Train_Mapper";
            spec->reducer_class = "KMeans_Train_Reducer";
            spec->input_format = input;
            spec->num_inputs = num_input;
            spec->output_format = KMeans_trainingfile;

            mr->set_spec(spec);

            get_centroids(c1);
            for (int i = 0; i < max_iter; ++i)
            {
                printf("iteration %d\n", i + 1);
                mr->work(result);
                time += result.timeelapsed;
                get_centroids(c2);
                if (diff_centroids(c1, c2) < eps)
                    break;
                c1 = c2;
            }

            spec->num_reducers = tmp;
            result.timeelapsed = time;
        }

        void kmeans::predict(const string& input, int num_input, const string& output, MapReduceResult& result)
        {
            spec->mapper_class = "KMeans_Test_Mapper";
            spec->reducer_class = "KMeans_Test_Reducer";
            spec->input_format = input;
            spec->num_inputs = num_input;
            spec->output_format = output;

            mr->set_spec(spec);
            mr->work(result);
        }
    }
}