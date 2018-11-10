#include "mapper.h"

namespace lmr
{
    void Mapper::set_numreducer(int _num)
    {
        num_reducer = _num;
        out.clear();
        out.resize(_num);
    }

    void Mapper::emit(string key, string value)
    {
        out[hashfunc(key) % num_reducer].insert(make_pair(key, value));
    }

    void Mapper::mapwork()
    {
        string key, value;
        if (!mapinput)
        {
            fprintf(stderr, "no assigned mapinput.\n");
            exit(1);
        }
        init();
        while (mapinput->get_next(key, value))
            Map(key, value);
        combine();

        output();
    }

    void Mapper::output()
    {
        ofstream f;
        char *tmp = new char[100 + 2 * outputfile.size()];
        for (int i = 0; i < num_reducer; ++i)
        {
            sprintf(tmp, outputfile.c_str(), i);
            f.open(tmp);
            for (auto &j : out[i])
            {
                f << j.first.size() << "\t" << j.first << "\t" << j.second << "\n";
            }
            f.close();
        }
        delete[] tmp;
    }
}
