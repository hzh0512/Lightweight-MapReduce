#ifndef LMR_LINEAR_REGRESSION_H
#define LMR_LINEAR_REGRESSION_H

#include "../mapreduce.h"
#include <fstream>
#include <stdio.h>

namespace lmr{
    namespace ml{
        using namespace std;

        class LinearRegression{
        public:
            LinearRegression(MapReduce *_mr);
            void compute(const string& input, const string& beta, const string &testfile, MapReduceResult& result);

        private:
            MapReduce* mr;
            MapReduceSpecification* spec;
        };
    }
}

#endif //LMR_LINEAR_REGRESSION_H
