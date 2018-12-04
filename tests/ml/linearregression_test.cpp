//
// Created by Shiming Zhuang on 12/3/18.
//

#include "../../src/mapreduce.h"
#include "../../src/ml/linear_regression.h"

using namespace lmr;
using namespace std;


int main(int argc, char **argv)
{
    MapReduce mr;
    MapReduceSpecification spec;
    MapReduceResult result;

    spec.program_file = basename(argv[0]);
    spec.config_file = "config.txt";
    spec.index = (argc == 2) ? atoi(argv[1]) : 0;
    spec.num_mappers = 2;
    spec.num_reducers = 2;
    mr.set_spec(&spec);

    if (spec.index == 0)
        split_file_ascii("lr/diabetes.tab.txt", "lr/input_%d.txt", 2);

    ml::LinearRegression lr(&mr);

    printf("***Computing.***\n");
    lr.compute("lr/input_%d.txt", 2, "lr/beta.txt", result);
    printf("%.3fs elapsed.\n", result.timeelapsed);
    return 0;
}

