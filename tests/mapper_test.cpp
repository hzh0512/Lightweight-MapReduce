#include "../src/mapper.h"
#include <map>

using namespace lmr;
using namespace std;


class WordCountMapper : public Mapper
{
public:
    virtual void Map(const string& key, const string& value)
    {
        int n = value.size();
        for (int i = 0; i < n; ) {
            while ((i < n) && isspace(value[i]))
                i++;

            int start = i;
            while ((i < n) && !isspace(value[i]))
                i++;

            if (start < i)
                combined_results[value.substr(start, i-start)]++;
        }
    }

    void combine() {
        for (auto &p : combined_results)
            emit(p.first, to_string(p.second));
        combined_results.clear();
    }

private:
    map<string, int> combined_results;
};

REGISTER_MAPPER(WordCountMapper)

int main()
{
    MapInput mapinput;
    Mapper *mapper = CREATE_MAPPER("WordCountMapper");

    mapinput.add_file("input.txt");

    mapper->set_mapinput(&mapinput);
    mapper->set_numreducer(5);
    mapper->set_outputfile("output_%d.txt");

    mapper->mapwork();

    delete mapper;
    return 0;
}
