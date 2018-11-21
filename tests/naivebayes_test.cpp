#include "../src/mapreduce.h"
#include "../src/ml/naivebayes.h"

using namespace lmr;
using namespace std;


int main(int argc, char **argv)
{
    MapReduceSpecification spec;
    MapReduceResult result;
    char *program_file = basename(argv[0]);
    int index = (argc == 2) ? atoi(argv[1]) : 0;

    spec.program_file = program_file;
    spec.config_file = "config.txt";
    spec.index = index;

    spec.output_format = "output/result_%d.txt";

    spec.num_mappers = 1;
    spec.num_reducers = 2;

    ml::naivebayes nb(&spec, index);
    nb.train("input_%d.txt", 2, result);

    printf("%.3fs elapsed.\n", result.timeelapsed);

    return 0;
}
