#include "../../src/mapreduce.h"
#include "../../src/ml/kmeans.h"

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

    split_file_ascii("kmeans/USCensus1990.full.txt", "kmeans/input_%d.txt", 5);

    ml::kmeans km(&mr);

    printf("***Training.***\n");
    km.train("kmeans/input_%d.txt", 5, "kmeans/centroids.txt", 0.01, 20, result);
    printf("%.3fs elapsed.\n", result.timeelapsed);

    printf("***Predicing.***\n");
    km.predict("kmeans/input_%d.txt", 5, "output/result_%d.txt", result);
    printf("%.3fs elapsed.\n", result.timeelapsed);

    return 0;
}

