//
// Created by Shiming Zhuang on 12/2/18.
//

#ifndef LMR_LINEAR_REGRESSION_H
#define LMR_LINEAR_REGRESSION_H

#include "../mapreduce.h"

namespace lmr{
    namespace ml{
        using namespace std;

        class LinearRegression{
        public:
            LinearRegression(MapReduce *_mr);
            void compute(const string& input, int num_input, const string& beta, MapReduceResult& result);

        private:
            MapReduce* mr;
            MapReduceSpecification* spec;
        };
    }
}

#endif //LMR_LINEAR_REGRESSION_H
