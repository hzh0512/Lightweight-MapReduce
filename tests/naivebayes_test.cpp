#include "../src/mapreduce.h"
#include "../src/ml/naivebayes.h"

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
    spec.num_mappers = 5;
    spec.num_reducers = 5;
    mr.set_spec(&spec);

    ml::naivebayes nb(&mr);

    printf("***Training.***\n");
    nb.train("input_%d.txt", 2, result);
    printf("%.3fs elapsed.\n", result.timeelapsed);

    printf("***Predicing.***\n");
    nb.predict("test_%d.txt", 2, "output/result_%d.txt", result);
    printf("%.3fs elapsed.\n", result.timeelapsed);

    return 0;
}
