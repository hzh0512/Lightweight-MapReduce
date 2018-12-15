# Lightweight MapReduce framework
Final project for CMU 15-618 Parallel Computer Architecture and Programming ([website](http://www.cs.cmu.edu/~418/))

For more background and introduction, please see our project [website](https://git.hzh0512.com/p/mapreduce) for details.

## Introduction

We implemented a lightweight MapReduce framework using **C++**, and integrated several built-in **machine learning** algorithms (e.g. naive bayes, logistic regression etc.) on top of it. It's a **cross-platform** framework which has been validated on Windows(cygwin), Linux(Redhat, Ubuntu) and Mac OS. The framework itself is based on a distributed file system (like AFS) for temporary file sharing, but *sshfs* may work as well but not yet tested. 

The final setting comes down to a cluster configuration file and a **single** binary executable which would *ssh* to other machines and call itself.

## Example code

### KMeans

```C++
#include "../../src/mapreduce.h"
#include "../../src/ml/kmeans.h"

using namespace lmr;
using namespace std;


int main(int argc, char **argv)
{
    MapReduce mr;
    MapReduceSpecification spec;
    MapReduceResult result;

    spec.program_file = basename(argv[0]);
    spec.config_file = "config.txt";
    spec.index = (argc == 2) ? atoi(argv[1]) : 0;
    spec.num_mappers = 10;
    spec.num_reducers = 10;
    
    mr.set_spec(&spec);
    ml::kmeans km(&mr);
    
    // utility function for splitting the input file
    if (spec.index == 0)
        split_file_ascii("kmeans/USCensus1990.full.txt", "kmeans/input_%d.txt", 10);

    printf("***Training.***\n");
    km.train("kmeans/input_%d.txt", 10, "kmeans/centroids.txt", 0.1, 20, result);
    printf("%.3fs elapsed.\n", result.timeelapsed);

    printf("***Predicing.***\n");
    km.predict("kmeans/input_%d.txt", 10, "output/result_%d.txt", result);
    printf("%.3fs elapsed.\n", result.timeelapsed);

    return 0;
}


```

### Word Counting

We have also implemented a MapReduce framework for general purposes. Users should implement their own logic by inheriting from the Mapper and Reducer classes. See example as follows.

```C++
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
                combined_results[value.substr(start, i-start)]++;
        }
    }

    virtual void combine() {
        for (auto &p : combined_results)
            emit(p.first, to_string(p.second));
        combined_results.clear();
    }

private:
    map<string, int> combined_results;
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

    spec.program_file = basename(argv[0]);
    spec.config_file = "config.txt";
    spec.index = (argc == 2) ? atoi(argv[1]) : 0;

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
```
