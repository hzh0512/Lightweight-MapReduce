#ifndef LMR_HASH_H
#define LMR_HASH_H

#include <string>

namespace lmr
{
    using namespace std;

    typedef unsigned int (*HashFunction)(const std::string&);

    unsigned int JSHash(const std::string& str);
}

#endif //LMR_HASH_H
