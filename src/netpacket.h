#ifndef LMR_NETPACKET_H
#define LMR_NETPACKET_H

#include <string>
#include <vector>
#include <sstream>

namespace lmr
{
    using namespace std;

    inline string form_assign_mapper(string output_format, vector<int> input_indices)
    {
        for (int i : input_indices)
        {
            output_format.push_back('\n');
            output_format += to_string(i);
        }
        return output_format;
    }

    inline void parse_assign_mapper(char* data, string& output_format, vector<int>& input_indices)
    {
        istringstream is(data);
        int i;
        input_indices.clear();
        is >> output_format;
        while (is >> i)
            input_indices.push_back(i);
    }

    inline string form_assign_reducer(string input_format)
    {
        return input_format;
    }

    inline void parse_assign_reducer(char* data, string& input_format)
    {
        istringstream is(data);
        is >> input_format;
    }

    inline string form_mapper_done(vector<int> finished_indices)
    {
        string tmp;
        for (int i : finished_indices)
        {
            tmp += to_string(i);
            tmp.push_back('\n');
        }
        tmp.pop_back();
        return tmp;
    }

    inline void parse_mapper_done(char* data, vector<int>& finished_indices)
    {
        istringstream is(data);
        int i;
        finished_indices.clear();
        while (is >> i)
            finished_indices.push_back(i);
    }
}

#endif //LMR_NETPACKET_H
