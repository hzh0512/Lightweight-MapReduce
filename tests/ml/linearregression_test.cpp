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
    int num_mappers = 1;
    spec.num_mappers = num_mappers;
    spec.num_inputs = num_mappers;
    spec.num_reducers = 1;
    mr.set_spec(&spec);

    if (spec.index == 0)
        split_file_ascii("diabetes.tab.txt", "lr/input_%d.txt", num_mappers);

    ml::LinearRegression lr(&mr);

    printf("***Computing.***\n");
    lr.compute("lr/input_%d.txt", "lr/beta.txt", "simple.tab.txt", result);
    printf("%.3fs elapsed.\n", result.timeelapsed);
    return 0;
}

