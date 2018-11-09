#include "hash.h"

namespace lmr
{
    unsigned int JSHash(const string& str)
    {
        unsigned int hash = 1315423911;
        for (char c : str) {
            hash ^= ((hash << 5) + c + (hash >> 2));
        }
        return hash;
    }
}