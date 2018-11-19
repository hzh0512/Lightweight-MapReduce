#include "reduceinput.h"

namespace lmr
{
    void ReduceInput::Add_file(std::string filename){
        int index = fs.size();
        fs.push_back(ifstream());
        fs[index].open(filename);
        while (fs[index].good()){
            string line, key, value;
            getline(fs[index], line);
            if (line.back() == '\r')
                line.pop_back();
            if (!line.empty()){
                // parse line in input
                pq.push(parse_line(line, index));
                return;
            }
        }

        fs[index].close();
        return;
    }


    bool ReduceInput::Get_next(string &key, string &value)
    {
        if (pq.empty()){
            return false;
        }

        int index;
        auto top = pq.top();
        pq.pop();
        key = get<0>(top);
        value = get<1>(top);
        index = get<2>(top);
        while (fs[index].good()){
            string line;
            getline(fs[index], line);
            if (line.back() == '\r')
                line.pop_back();
            if (!line.empty()){
                // parse line in input
                pq.push(parse_line(line, index));
                return true;
            }
        }

        fs[index].close();
        return true;
    }

    tuple<string, string, int> ReduceInput::parse_line(std::string line, int index) {
        int cur = 0, key_len;
        while (cur < line.size() && isdigit(line[cur])) { cur++; }
        key_len = stoi(line.substr(0, cur));
        string key = line.substr(cur + 1, key_len);
        string value = line.substr(cur + 2 + key_len);
        return make_tuple(key, value, index);
    }
}