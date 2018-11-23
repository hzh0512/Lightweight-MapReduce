#ifndef LMR_NAIVEBAYES_H
#define LMR_NAIVEBAYES_H

#include "../mapreduce.h"


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
            void predict(const string& input, int num_input, const string& output, MapReduceResult& result);

            static FormatFunc func;
        private:
            int index;
            MapReduce* mr;
            MapReduceSpecification* spec;
            string trainingformat = "lmr_nb_tmp/train_%d.txt";
        };
    }
}

#endif //LMR_NAIVEBAYES_H
