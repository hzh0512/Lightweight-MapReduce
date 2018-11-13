#include "reducer.h"

namespace lmr
{
    void Reducer::emit(string key, string value)
    {
        out.insert(make_pair(key, value));
    }

    void Reducer::reducework()
    {
        string key, value;
        if (!reduceinput)
        {
            fprintf(stderr, "no assigned mapinput.\n");
            exit(1);
        }
        while (reduceinput->get_next(key, value))
            Reduce(key, value);
    }

    void Reducer::output()
    {
        ofstream f;
        f.open(outputfile);
        for (auto &j : out)
        {
            f << j.first.size() << "\t" << j.first << "\t" << j.second << "\n";
        }
        f.close();
    }
}
