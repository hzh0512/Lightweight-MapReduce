#include "mapinput.h"

namespace lmr
{
    bool MapInput::get_next(string &key, string &value)
    {
        while (true)
        {
            while (!f.good())
            {
                if (++file_index >= files.size())
                    return false;
                f.close();
                f.open(files[file_index]);
            }

            while (getline(f, value))
            {
                if (value.back() == '\r')
                    value.pop_back();
                if (!value.empty())
                {
                    key = files[file_index] + "_" + to_string(line_index++);
                    return true;
                }
            }
        }
    }
}