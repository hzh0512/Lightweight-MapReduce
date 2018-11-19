#include "../src/reducer.h"
#include <map>

using namespace lmr;
using namespace std;


class WordCountReducer : public Reducer
{
public:
    virtual void Reduce(const string& key, const string& value)
    {
        if (key_ == key){
            value_ = to_string(stoi(value) + stoi(value_));
        } else {
            output(key_, value_);
            key_ = key;
            value_ = value;
        }
    }
};

REGISTER_REDUCER(WordCountReducer)

int main()
{
    //ReduceInput reduceinput;
    vector<string> inputfiles;
    Reducer *reducer = CREATE_REDUCER("WordCountReducer");

    reducer->set_nummapper(5);
    for (int i = 0; i < 5; i ++){
        char* filename = new char[20];
        sprintf(filename, "output_%d.txt", i);
        inputfiles.push_back(filename);
    }

    ReduceInput reduceinput;
    for (auto file:inputfiles){
        reduceinput.Add_file(file);
    }

    reducer->set_reduceinput(&reduceinput);
    reducer->set_outputfile("reducer_output.txt");

    reducer->reducework();

    delete reducer;
    return 0;
}
