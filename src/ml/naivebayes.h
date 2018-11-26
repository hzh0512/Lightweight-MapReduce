#ifndef LMR_NAIVEBAYES_H
#define LMR_NAIVEBAYES_H

#include "../mapreduce.h"
#include <cmath>

namespace lmr
{
    namespace ml
    {
        using namespace std;
        typedef void (*xy_FormatFunc)(const string &input, vector<string> &y, vector<string> &x);

        class naivebayes
        {
        public:
            naivebayes(MapReduce* _mr, bool _keep_training = false);
            ~naivebayes();
            void set_formatfunc(xy_FormatFunc _func) {func = _func;}
            void train(const string& input, int num_input, MapReduceResult& result);
            void predict(const string& input, int num_input, const string& output, MapReduceResult& result);

            static xy_FormatFunc func;
        private:
            int index;
            MapReduce* mr;
            MapReduceSpecification* spec;
            bool keep_training;
        };
    }
}

#endif //LMR_NAIVEBAYES_H
