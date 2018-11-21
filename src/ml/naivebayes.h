#ifndef LMR_NAIVEBAYES_H
#define LMR_NAIVEBAYES_H

#include "../mapreduce.h"
#include <vector>
#include <string>
#include <unordered_map>

namespace lmr
{
    namespace ml
    {
        using namespace std;
        typedef void (*FormatFunc)(const string &input, vector<string> &y, vector<string> &x);

        class naivebayes
        {
        public:
            naivebayes(MapReduceSpecification* _spec, int _index):index(_index), spec(_spec){}
            void set_formatfunc(FormatFunc _func) {func = _func;}
            void train(const string& input, int num_input, MapReduceResult& result);

            static FormatFunc func;
        private:
            int index;
            MapReduceSpecification* spec;
            string trainingformat = "lmr_nb_tmp/train_%d.txt";
        };
    }
}

#endif //LMR_NAIVEBAYES_H
