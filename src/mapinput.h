#ifndef LMR_MAPINPUT_H
#define LMR_MAPINPUT_H

#include <string>
#include <vector>
#include <fstream>

namespace lmr
{
    using namespace std;
    class MapInput
    {
    public:
        void add_file(string filename) { files.push_back(filename); }
        bool get_next(string &key, string &value);

    private:
        int file_index = -1, line_index = 0;
        vector<string> files;
        ifstream f;
    };
}

#endif //LMR_MAPINPUT_H
