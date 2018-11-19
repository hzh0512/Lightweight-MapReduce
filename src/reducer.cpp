#include "reducer.h"

namespace lmr
{
    void Reducer::reducework()
    {
        string key, value;
        remove(outputfile_.c_str());
        out_.open(outputfile_);
        if (!reduceinput_)
        {
            fprintf(stderr, "no assigned mapinput.\n");
            exit(1);
        }
        init();
        while (reduceinput_->get_next_key(key))
            (Reduce(key, reduceinput_));
        out_.close();
    }

    void Reducer::output(string key, string value)
    {
        if (key.size() > 0){
            out_ << key.size() << "\t" << key << "\t" << value << "\n";
        }
    }
}
