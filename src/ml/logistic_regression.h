#ifndef LMR_LOGISTIC_REGRESSION_H
#define LMR_LOGISTIC_REGRESSION_H

#include "../mapreduce.h"
#include <cmath>

namespace lmr
{
    namespace ml
    {
        using namespace std;

        class LogisticRegression
        {
        public:
            LogisticRegression(MapReduce* _mr, bool _keep_training = false);
            ~LogisticRegression();
            void train(const string& input, int num_input, const string& theta, int num_data, int max_iter, MapReduceResult& result);
            void predict(const string &testfile, const string &thetafile, const string &output);
        private:
            int index;
            MapReduce* mr;
            MapReduceSpecification* spec;
            bool keep_training;
        };
    }
}

#endif //LMR_LOGISTIC_REGRESSION_H
