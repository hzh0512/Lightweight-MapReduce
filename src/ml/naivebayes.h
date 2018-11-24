#ifndef LMR_NAIVEBAYES_H
#define LMR_NAIVEBAYES_H

#include "../mapreduce.h"
#include <cmath>

namespace lmr
{
    namespace ml
    {
        using namespace std;
        typedef void (*FormatFunc)(const string &input, vector<string> &y, vector<string> &x);

        class naivebayes
        {
        public:
            naivebayes(MapReduce* _mr):mr(_mr) { spec = _mr->get_spec(); index = spec->index; }
            void set_formatfunc(FormatFunc _func) {func = _func;}
            void train(const string& input, int num_input, MapReduceResult& result);
            void predict(const string& input, int num_input, const string& output, MapReduceResult& result, bool keep_training = false);

            static FormatFunc func;
        private:
            int index;
            MapReduce* mr;
            MapReduceSpecification* spec;
        };
    }
}

#endif //LMR_NAIVEBAYES_H
