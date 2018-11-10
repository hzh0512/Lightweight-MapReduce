#ifndef LMR_SPEC_H
#define LMR_SPEC_H

#include <string>
#include "mapinput.h"
#include "hash.h"

namespace lmr
{
    using namespace std;

    typedef struct
    {
        HashFunction hashfunc = JSHash;
        MapInput* mapinput = nullptr;
        string mapper_class, reducer_class, config_file, program_file;
        int num_reducers = 0, index = 0;
    } MapReduceSpecification;
}

#endif //LMR_SPEC_H
