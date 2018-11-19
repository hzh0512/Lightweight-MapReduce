#ifndef LMR_REDUCER_H
#define LMR_REDUCER_H

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
        virtual void Reduce(const string& key, ReduceInput* reduceInput) = 0;
        virtual ~Reducer(){}

        void set_reduceinput(ReduceInput* _reduceinput) { reduceinput_ = _reduceinput; }
        void set_hashfunc(HashFunction _hashfunc) { hashfunc_ = _hashfunc; }
        void set_outputfile(const string& _outputfile) { outputfile_ = _outputfile; }
        void set_nummapper(int _num) { num_mapper_ = _num; }

        void reducework();
        void output(string key, string value);

    protected:
        //void emit(string key, string value);

    private:
        HashFunction hashfunc_ = JSHash;
        int num_mapper_ = 0;
        string outputfile_;
        ofstream out_;
        ReduceInput* reduceinput_ = nullptr;
    };

    BASE_CLASS_REGISTER(Reducer)

    #define REGISTER_REDUCER(reducer_name) \
        CHILD_CLASS_REGISTER(Reducer,reducer_name)

    #define CREATE_REDUCER(reducer_name)   \
        CHILD_CLASS_CREATOR(Reducer,reducer_name)
}

#endif //LMR_REDUCER_H
