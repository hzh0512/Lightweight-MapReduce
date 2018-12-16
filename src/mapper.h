#ifndef LMR_MAPPER_H
#define LMR_MAPPER_H

#include <string>
#include <vector>
#include <set>
#include <chrono>
#include <fstream>
#include <algorithm>
#include "register.h"
#include "mapinput.h"
#include "hash.h"

namespace lmr
{
    using namespace std;

    class Mapper
    {
    public:
        virtual void init() {}
        virtual void Map(const string& key, const string& value) = 0;
        virtual void combine() {}
        virtual ~Mapper(){}

        void set_mapinput(MapInput* _mapinput) { mapinput = _mapinput; }
        void set_hashfunc(HashFunction _hashfunc) { hashfunc = _hashfunc; }
        void set_outputfile(const string& _outputfile) { outputfile = _outputfile; }
        void set_numreducer(int _num);

        void mapwork();

    protected:
        void emit(string key, string value);

    private:
        void output();

        HashFunction hashfunc = JSHash;
        int num_reducer = 0;
        string outputfile;
        MapInput* mapinput = nullptr;
        vector<multiset<pair<string, string>>> out;
    };

    BASE_CLASS_REGISTER(Mapper)

    #define REGISTER_MAPPER(mapper_name) \
        CHILD_CLASS_REGISTER(Mapper,mapper_name)

    #define CREATE_MAPPER(mapper_name)   \
        CHILD_CLASS_CREATOR(Mapper,mapper_name)
}

#endif //LMR_MAPPER_H
