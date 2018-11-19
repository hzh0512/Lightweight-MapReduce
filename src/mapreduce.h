#ifndef LMR_MAPREDUCE_H
#define LMR_MAPREDUCE_H

#include <unordered_map>
#include <vector>
#include <queue>
#include <chrono>
#include "libgen.h"
#include "libssh/libssh.h"
#include "unistd.h"
#include "spec.h"
#include "netcomm.h"
#include "netpacket.h"
#include "mapper.h"
#include "reducer.h"

namespace lmr
{
    using namespace std;
    using namespace std::chrono;

    enum workertype
    {
        master, worker, mapper, reducer
    };

    enum jobstatus
    {
        nonexist, mapper_done, reducer_done
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
        inline int net_mapper_index(int i);
        inline int net_reducer_index(int i);
        inline int mapper_net_index(int i);
        inline int reducer_net_index(int i);

        void assign_mapper(const string& output_format, const vector<int>& input_index);
        void mapper_done(int net_index, const vector<int>& finished_index);
        void assign_reducer(const string& input_format);
        void reducer_done(int net_index);

        bool stopflag = false, isready = false;
        int index, total, mapper_finished_cnt = 0, reducer_finished_cnt = 0;
        MapReduceSpecification* spec = nullptr;
        workertype type;
        netcomm *net = nullptr;
        vector<vector<jobstatus>> status;
        queue<int> jobs;
    };
}

#endif //LMR_MAPREDUCE_H
