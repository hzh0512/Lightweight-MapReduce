#include "../../src/mapreduce.h"
#include "../../src/ml/naivebayes.h"

using namespace lmr;
using namespace std;


int main(int argc, char **argv)
{
    MapReduce mr;
    MapReduceSpecification spec;
    MapReduceResult result;

    int num_mappers = 8;
    int num_input = num_mappers * 1;
    spec.program_file = basename(argv[0]);
    spec.config_file = "config.txt";
    spec.index = (argc == 2) ? atoi(argv[1]) : 0;
    spec.num_mappers = num_mappers;
    spec.num_reducers = num_mappers;
    mr.set_spec(&spec);

    if (spec.index == 0)
        split_file_ascii("input.txt", "nb/input_%d.txt", num_input);

    ml::naivebayes nb(&mr);

    printf("***Training.***\n");
    nb.train("nb/input_%d.txt", num_input, result);
    printf("%.3fs elapsed.\n", result.timeelapsed);

    printf("***Predicing.***\n");
    nb.predict("nb/input_%d.txt", num_input, "nb/result_%d.txt", result);
    printf("%.3fs elapsed.\n", result.timeelapsed);

    return 0;
}
