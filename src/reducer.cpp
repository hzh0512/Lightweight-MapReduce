#include "reducer.h"

namespace lmr
{
    void Reducer::reducework()
    {
        string key, value;
        out.open(outputfile);
        if (!reduceinput)
        {
            fprintf(stderr, "no assigned mapinput.\n");
            exit(1);
        }
        while (reduceinput->Get_next(key, value))
            (Reduce(key, value));
        out.close();
    }

    void Reducer::output(string key, string value)
    {
        if (key.size() > 0){
            out << key.size() << "\t" << key << "\t" << value << "\n";
        }
    }
}
