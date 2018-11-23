#include "libgen.h"
#include "../src/mapreduce.h"

using namespace lmr;

int main(int argc, char **argv)
{
    MapReduceSpecification spec;
    MapReduceResult result;
    char *program_file = basename(argv[0]);
    int index = (argc == 2) ? atoi(argv[1]) : 0;

    spec.program_file = program_file;
    spec.config_file = "config.txt";
    spec.index = index;

    MapReduce mr(&spec);
    mr.work(result);

    return 0;
}