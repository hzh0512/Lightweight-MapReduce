#include "reduceinput.h"

namespace lmr
{
    void ReduceInput::add_file(std::string filename){
        int index = fs_.size();
        fs_.push_back(new ifstream());
        fs_[index]->open(filename);
        while (fs_[index]->good()){
            string line, key, value;
            getline(*fs_[index], line);
            if (line.back() == '\r')
                line.pop_back();
            if (!line.empty()){
                pq_.push(parse_line(line, index));
                return;
            }
        }
        fs_[index]->close();
    }

    bool ReduceInput::get_next_key(string &key) {
        if (pq_.empty()){
            return false;
        } else {
            key  = key_ = get<0>(pq_.top());
            return true;
        }
    }

    bool ReduceInput::get_next_value(string &value)
    {
        if (pq_.empty()){
            return false;
        }

        auto top = pq_.top();
        string key = get<0>(top);
        if (key != key_){
            return false;
        }

        pq_.pop();
        value = get<1>(top);
        int index = get<2>(top);
        while (fs_[index]->good()){
            string line;
            getline(*fs_[index], line);
            if (line.back() == '\r')
                line.pop_back();
            if (!line.empty()){
                // parse line in input
                pq_.push(parse_line(line, index));
                return true;
            }
        }

        fs_[index]->close();
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

    ReduceInput::~ReduceInput()
    {
        for (auto p : fs_)
            delete p;
    }
}