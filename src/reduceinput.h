#ifndef LMR_REDUCEINPUT_H
#define LMR_REDUCEINPUT_H

#include <string>
#include <vector>
#include <fstream>
#include <queue>
#include <tuple>

namespace lmr
{
    using namespace std;
    class ReduceInput
    {
    public:
        void Set_format(string _format) { format = _format; }
        string Get_format() { return format; }
        void Add_file(string filename);
        bool Get_next(string &key, string &value);

    private:
        tuple<string, string, int> parse_line(string line, int index);
        string format = "text";
        int file_index = -1;
        vector<string> files;
        vector<ifstream> fs;
        priority_queue<tuple<string, string, int>, vector<tuple<string, string, int> >, greater<tuple<string, string, int> > > pq;
    };
}

#endif //LMR_REDUCEINPUT_H
