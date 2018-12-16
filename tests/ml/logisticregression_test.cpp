#include "../../src/mapreduce.h"
#include "../../src/ml/logistic_regression.h"

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
    int num_mappers = 3;
    spec.num_mappers = num_mappers;
    spec.num_inputs = num_mappers;
    spec.num_reducers = 3;
    mr.set_spec(&spec);
    if (spec.index == 0)
        split_file_ascii("train.txt", "lr/input_%d.txt", num_mappers);

    ml::LogisticRegression lr(&mr);

    printf("***Computing.***\n");
    lr.train("lr/input_%d.txt", num_mappers, "lr/theta.txt", 350, 30, result);
    printf("%.3fs elapsed.\n", result.timeelapsed);

    lr.predict("train.txt", "lr/theta.txt", "lr/labels.txt");
    return 0;
}

