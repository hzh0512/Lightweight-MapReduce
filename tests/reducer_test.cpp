#include "../src/reducer.h"
#include <map>

using namespace lmr;
using namespace std;


class WordCountReducer : public Reducer
{
public:
    virtual void Reduce(const string& key, const string& value)
    {
        if (out.find(key) == out.end()){
            out[key] = '0';
        }
        out[key] = to_string(stoi(value) + stoi(out[key]));
    }
};

REGISTER_REDUCER(WordCountReducer)

int main()
{
    ReduceInput reduceinput;
    Reducer *reducer = CREATE_REDUCER("WordCountReducer");

    reducer->set_nummapper(5);
    for (int i = 0; i < 5; i ++){
        char* filename = new char[20];
        sprintf(filename, "output_%d.txt", i);
        reduceinput.add_file(filename);
    }

    reducer->set_reduceinput(&reduceinput);
    reducer->set_outputfile("reducer_output.txt");

    reducer->reducework();
    reducer->output();

    delete reducer;
    return 0;
}
