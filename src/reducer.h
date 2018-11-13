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
        virtual void Reduce(const string& key, const string& value) = 0;
        virtual ~Reducer(){}

        void set_reduceinput(ReduceInput* _reduceinput) { reduceinput = _reduceinput; }
        void set_outputfile(const string& _outputfile) { outputfile = _outputfile; }

        void reducework();
        void output();
        map<string, string> out;

    protected:
        void emit(string key, string value);

    private:
        string outputfile;
        ReduceInput* reduceinput = nullptr;
    };

    BASE_CLASS_REGISTER(Reducer)

    #define REGISTER_REDUCER(reducer_name) \
        CHILD_CLASS_REGISTER(Reducer,reducer_name)

    #define CREATE_REDUCER(reducer_name)   \
        CHILD_CLASS_CREATOR(Reducer,reducer_name)
}

#endif //LMR_REDUCER_H
