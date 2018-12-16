#include "../../src/mapreduce.h"
#include "../../src/ml/kmeans.h"

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
    spec.num_reducers = 1;
    mr.set_spec(&spec);

    if (spec.index == 0)
        split_file_ascii("kmeans/USCensus1990.full.txt", "kmeans/input_%d.txt", num_mappers);

    ml::kmeans km(&mr);

    printf("***Training.***\n");
    km.train("kmeans/input_%d.txt", num_input, "kmeans/centroids.txt", 0.1, 20, result);
    printf("%.3fs elapsed.\n", result.timeelapsed);

    printf("***Predicing.***\n");
    km.predict("kmeans/input_%d.txt", num_input, "output/result_%d.txt", result);
    printf("%.3fs elapsed.\n", result.timeelapsed);

    return 0;
}

