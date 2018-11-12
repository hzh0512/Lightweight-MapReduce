#ifndef LMR_REDUCEINPUT_H
#define LMR_REDUCEINPUT_H

#include <string>
#include <vector>
#include <fstream>

namespace lmr
{
    using namespace std;
    class ReduceInput
    {
    public:
        void set_format(string _format) { format = _format; }
        string get_format() { return format; }
        void add_file(string filename) { files.push_back(filename); }
        bool get_next(string &key, string &value);

    private:
        string format = "text";
        int file_index = -1, line_index = 0;
        vector<string> files;
        ifstream f;
    };
}

#endif //LMR_REDUCEINPUT_H
