#ifndef LMR_MAPPER_H
#define LMR_MAPPER_H

#include <string>
#include <vector>
#include <set>
#include <fstream>
#include <algorithm>
#include <map>
#include "register.h"
#include "reduceinput.h"
#include "hash.h"

namespace lmr
{
    using namespace std;

    class Reducer
    {
    public:
        virtual void init() {}
        virtual void Reduce(const string& key, const string& value) = 0;
        virtual ~Reducer(){}

        void set_reduceinput(ReduceInput* _reduceinput) { reduceinput = _reduceinput; }
        void set_hashfunc(HashFunction _hashfunc) { hashfunc = _hashfunc; }
        void set_outputfile(const string& _outputfile) { outputfile = _outputfile; }
        void set_nummapper(int _num) { num_mapper = _num; }
        string key_, value_;

        void reducework();
        void output(string key, string value);

    protected:
        //void emit(string key, string value);

    private:

        HashFunction hashfunc = JSHash;
        int num_mapper = 0;
        string outputfile;
        ofstream out;
        ReduceInput* reduceinput = nullptr;
    };

    BASE_CLASS_REGISTER(Reducer)

    #define REGISTER_REDUCER(reducer_name) \
        CHILD_CLASS_REGISTER(Reducer,reducer_name)

    #define CREATE_REDUCER(reducer_name)   \
        CHILD_CLASS_CREATOR(Reducer,reducer_name)
}

#endif //LMR_MAPPER_H
