#include "../src/mapreduce.h"

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
                emit(value.substr(start, i-start), "1");
        }
    }
};

REGISTER_MAPPER(WordCountMapper)

class WordCountReducer : public Reducer
{
public:
    virtual void Reduce(const string& key, ReduceInput* reduceInput)
    {
        string value;
        int result = 0;
        while (reduceInput->get_next_value(value)){
            result += stoi(value);
        }
        output(key, to_string(result));
    }
};

REGISTER_REDUCER(WordCountReducer)


int main(int argc, char **argv)
{
    MapReduceSpecification spec;
    MapReduceResult result;
    char *program_file = basename(argv[0]);
    int index = (argc == 2) ? atoi(argv[1]) : 0;

    spec.program_file = program_file;
    spec.config_file = "config.txt";
    spec.index = index;

    spec.input_format = "input_%d.txt";
    spec.output_format = "output/result_%d.txt";
    spec.num_inputs = 3;

    spec.mapper_class = "WordCountMapper";
    spec.num_mappers = 1;

    spec.reducer_class = "WordCountReducer";
    spec.num_reducers = 2;

    MapReduce mr(&spec);
    mr.work(result);
    printf("%.3fs elapsed.\n", result.timeelapsed);

    return 0;
}
