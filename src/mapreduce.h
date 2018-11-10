#ifndef LMR_MAPREDUCE_H
#define LMR_MAPREDUCE_H

#include <ctime>
#include "libssh/libssh.h"
#include "unistd.h"
#include "spec.h"
#include "netcomm.h"

namespace lmr
{
    using namespace std;

    enum workertype
    {
        master, worker, mapper, reducer
    };

    typedef struct
    {
        double timeelapsed;
    } MapReduceResult;

    class MapReduce
    {
        friend void cb(header* h, char* data, netcomm* net);
    public:
        MapReduce(MapReduceSpecification* _spec, int _index = 0);
        ~MapReduce();
        int work(MapReduceResult& result);

    private:
        bool dist_run_files();

        bool stopflag = false, isready = false;
        int index, total;
        MapReduceSpecification* spec = nullptr;
        workertype type;
        netcomm *net = nullptr;
    };
}

#endif //LMR_MAPREDUCE_H
