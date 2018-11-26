#ifndef LMR_KMEANS_H
#define LMR_KMEANS_H

#include "../mapreduce.h"

namespace lmr
{
    namespace ml
    {
        using namespace std;
        typedef void (*x_FormatFunc)(const string &input, vector<string> &x);
        typedef double (*DistanceFunc)(const vector<double> &x1, const vector<double> &x2);

        class kmeans
        {
        public:
            kmeans(MapReduce* _mr, bool _keep_training = false);
            ~kmeans();
            void set_formatfunc(x_FormatFunc _func) {func = _func;}
            void set_distancefunc(DistanceFunc _func) {distfunc = _func;}
            void train(const string& input, int num_input, const string& centroids, double eps, int max_iter, MapReduceResult& result);
            void predict(const string& input, int num_input, const string& output, MapReduceResult& result);

            static x_FormatFunc func;
            static DistanceFunc distfunc;
        private:
            int index;
            MapReduce* mr;
            MapReduceSpecification* spec;
            bool keep_training;
        };
    }
}
#endif //LMR_KMEANS_H
