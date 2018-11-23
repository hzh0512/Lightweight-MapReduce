#include "../src/mapreduce.h"
#include "../src/ml/naivebayes.h"

using namespace lmr;
using namespace std;


int main(int argc, char **argv)
{
    MapReduce mr;
    MapReduceSpecification spec;
    MapReduceResult result;
    char *program_file = basename(argv[0]);
    int index = (argc == 2) ? atoi(argv[1]) : 0;

    spec.program_file = program_file;
    spec.config_file = "config.txt";
    spec.index = index;

    spec.num_mappers = 1;
    spec.num_reducers = 2;

    mr.set_spec(&spec);
    ml::naivebayes nb(&mr);

    nb.train("input_%d.txt", 2, result);
    fprintf(stderr, "%.3fs elapsed.\n", result.timeelapsed);

    fprintf(stderr, "***Predicing.***\n");
    nb.predict("test_%d.txt", 2, "output/result_%d.txt", result);
    fprintf(stderr, "%.3fs elapsed.\n", result.timeelapsed);

    return 0;
}
