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
        void set_format(string _format) { format_ = _format; }
        string get_format() { return format_; }
        void add_file(string filename);
        bool get_next_key(string &key);
        bool get_next_value(string &value);

    private:
        tuple<string, string, int> parse_line(string line, int index);
        string format_ = "text";

        int file_index_ = -1;
        string key_;
        vector<string> file_names_;
        vector<ifstream> fs_;
        priority_queue<tuple<string, string, int>, vector<tuple<string, string, int> >, greater<tuple<string, string, int> > > pq_;
    };
}

#endif //LMR_REDUCEINPUT_H
