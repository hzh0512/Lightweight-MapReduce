#include "reduceinput.h"

namespace lmr
{
    bool ReduceInput::get_next(string &key, string &value)
    {
        if (format != "text")
            return false;

        string line;
        while (true)
        {
            while (!f.good())
            {
                if (++file_index >= files.size())
                    return false;
                f.close();
                f.open(files[file_index]);
            }

            while (getline(f, line))
            {
                if (line.back() == '\r')
                    line.pop_back();
                if (!line.empty()){
                    // parse line in input
                    int i = 0, key_len;
                    while (i < line.size() && isdigit(line[i])) { i++; }
                    key_len = stoi(line.substr(0, i));
                    key = line.substr(i + 1, key_len);
                    value = line.substr(i + 2 + key_len);
                    return true;
                }
            }
        }
    }
}