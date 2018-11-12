#include "mapreduce.h"

namespace lmr
{
    MapReduce* instance = nullptr;
    pthread_mutex_t mutex;
    int sum = 0, number_checkin = 1;

    void cb(header* h, char* data, netcomm* net)
    {
        switch (h->type)
        {
            case netcomm_type::LMR_CHECKIN:
                pthread_mutex_lock(&mutex);
                if (++number_checkin == net->gettotalnum())
                    instance->isready = true;
                pthread_mutex_unlock(&mutex);
                break;
            case netcomm_type::LMR_CLOSE:
                instance->stopflag = true;
                break;
            case 100:
                net->send(h->src, 101, to_string(instance->index));
                break;
            case 101:
                printf("%s\n", data);
                if (++sum == net->gettotalnum() - 1)
                {
                    for (int i = 1; i < net->gettotalnum(); ++i)
                        net->send(i, netcomm_type::LMR_CLOSE, nullptr, 0);
                    instance->stopflag = true;
                }
                break;
            default:
                break;
        }
    }

    MapReduce::MapReduce(MapReduceSpecification* _spec, int _index)
    {
        instance = this;
        index = _index;
        spec = _spec;
        net = new netcomm(_spec->config_file, _index, cb);
        total = net->gettotalnum();

        if (_index > 0)
        {
            type = workertype::worker;
            net->send(0, netcomm_type::LMR_CHECKIN, nullptr, 0);
            isready = true;
        }
        else
        {
            type = workertype::master;
            pthread_mutex_init(&mutex, 0);
            if (!dist_run_files())
            {
                fprintf(stderr, "distribution error. cannot run workers.\n");
                net->wait();
                exit(1);
            }
            while (!isready)
                sleep_us(1000);
            for (int i = 1; i < total; ++i)
                net->send(i, 100, nullptr, 0);
        }
    }

    MapReduce::~MapReduce()
    {
        delete net;
        if (index == 0) pthread_mutex_destroy(&mutex);
    }

    int MapReduce::work(MapReduceResult& result)
    {
        clock_t start = clock();
        while (!stopflag)
            sleep_us(1000);
        result.timeelapsed = ((double)(clock() - start)) / CLOCKS_PER_SEC;
        net->wait();
        return 0;
    }

    bool run_sshcmd(const string &ip, const string &username, const string &password, const string &cmd)
    {
        ssh_session my_ssh_session;
        ssh_channel channel;
        int rc = 0;
        my_ssh_session = ssh_new();
        ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, ip.c_str());
        ssh_options_set(my_ssh_session, SSH_OPTIONS_USER, username.c_str());
        rc = ssh_connect(my_ssh_session);
        if (rc != SSH_OK)
        {
            fprintf(stderr, "Error connecting to localhost: %s\n", ssh_get_error(my_ssh_session));
            ssh_free(my_ssh_session);
            return false;
        }
        if (password.empty())
            rc = ssh_userauth_publickey_auto(my_ssh_session, nullptr, nullptr);
        else
            rc = ssh_userauth_password(my_ssh_session, nullptr, password.c_str());
        if (rc != SSH_AUTH_SUCCESS)
        {
            fprintf(stderr, "Error authenticating with password: %s\n", ssh_get_error(my_ssh_session));
            ssh_disconnect(my_ssh_session);
            ssh_free(my_ssh_session);
            return false;
        }
        channel = ssh_channel_new(my_ssh_session);
        ssh_channel_open_session(channel);
        ssh_channel_request_exec(channel, cmd.c_str());

        ssh_channel_send_eof(channel);
        ssh_channel_close(channel);
        ssh_channel_free(channel);

        ssh_disconnect(my_ssh_session);
        ssh_free(my_ssh_session);
        return true;
    }

    bool MapReduce::dist_run_files()
    {
        // get cwd
        char *tmp = getcwd(nullptr, 0);
        string cwd(tmp), temp, username, password;
        free(tmp);

        // get username and password
        ifstream f(spec->config_file);
        getline(f, temp);
        size_t pos = temp.find(':');
        username = temp.substr(0, pos);
        password = temp.substr(pos + 1);
        if (!password.empty() && password.back() == '\r')
            password.pop_back();
        password = base64_decode(password);

        // run workers
        unordered_map<string, vector<pair<int,int>>> um;
        for (int i = 1; i < total; ++i)
            um[net->endpoints[i].first].push_back(make_pair(i, net->endpoints[i].second));

        for (auto &p : um)
        {
            string cmd = "cd " + cwd + " && mkdir -p output";
            for (auto &p2 : p.second)
            {
                cmd += " && (./" + spec->program_file + " " + to_string(p2.first) +
                       " &> output/output" + to_string(p2.first) + ".txt &)";
            }
            if (p.first == "127.0.0.1")
                system(cmd.c_str());
            else if (!run_sshcmd(p.first, username, password, cmd))
                return false;
        }
        return true;
    }
}