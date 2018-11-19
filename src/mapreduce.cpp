#include "mapreduce.h"

namespace lmr
{
    MapReduce* instance = nullptr;
    pthread_mutex_t mutex;
    int number_checkin = 1;

    void cb(header* h, char* data, netcomm* net)
    {
        string input_format, output_format;
        vector<int> input_index, finished_indices;
        switch (h->type)
        {
            case netcomm_type::LMR_CHECKIN:
                pthread_mutex_lock(&mutex);
                if (++number_checkin == instance->total)
                    instance->isready = true;
                pthread_mutex_unlock(&mutex);
                break;
            case netcomm_type::LMR_CLOSE:
                instance->stopflag = true;
                break;
            case netcomm_type::LMR_ASSIGN_MAPPER:
                parse_assign_mapper(data, output_format, input_index);
                instance->assign_mapper(output_format, input_index);
                break;
            case netcomm_type::LMR_ASSIGN_REDUCER:
                parse_assign_reducer(data, input_format);
                instance->assign_reducer(input_format);
                break;
            case netcomm_type::LMR_MAPPER_DONE:
                parse_mapper_done(data, finished_indices);
                instance->mapper_done(h->src, finished_indices);
                break;
            case netcomm_type::LMR_REDUCER_DONE:
                instance->reducer_done(h->src);
                break;
            default:
                break;
        }
    }

    inline int MapReduce::net_mapper_index(int i)
    {
        return i - 1;
    }

    inline int MapReduce::net_reducer_index(int i)
    {
        return i - 1 - spec->num_mappers;
    }

    inline int MapReduce::mapper_net_index(int i)
    {
        return i + 1;
    }

    inline int MapReduce::reducer_net_index(int i)
    {
        return i + 1 + spec->num_mappers;
    }

    void MapReduce::assign_reducer(const string& input_format)
    {
        char tmp[1024];
        ReduceInput ri;
        Reducer *reducer = CREATE_REDUCER(spec->reducer_class);
        for (int i = 0; i < spec->num_inputs; ++i)
        {
            sprintf(tmp, input_format.c_str(), i);
            ri.add_file(tmp);
        }

        char tmp2[1024];
        strcpy(tmp2, spec->output_format.c_str());
        sprintf(tmp, "mkdir -p %s", dirname(tmp2));
        system(tmp); // make output directory

        string outputfile = spec->output_format;
        sprintf(tmp, outputfile.c_str(), net_reducer_index(index));

        reducer->set_reduceinput(&ri);
        reducer->set_outputfile(tmp);
        reducer->reducework();

        net->send(0, netcomm_type::LMR_REDUCER_DONE, nullptr, 0);
        delete reducer;
    }

    void MapReduce::reducer_done(int net_index)
    {
        int redece_index = net_reducer_index(net_index);
        bool finished = false;

        net->send(net_index, netcomm_type::LMR_CLOSE, nullptr, 0);

        pthread_mutex_lock(&mutex);
        for (int i = 0; i < status.size(); ++i)
            status[i][redece_index] = jobstatus::reducer_done;

        if (++reducer_finished_cnt == spec->num_reducers)
            finished = true;
        pthread_mutex_unlock(&mutex);

        if (finished)
        {
            fprintf(stderr, "ALL WORK DONE!\n");
            system("rm -rf tmp/");
            stopflag = finished;
        }
    }

    void MapReduce::mapper_done(int net_index, const vector<int>& finished_index)
    {
        int job_index = -1;
        for (int i : finished_index)
            for (int j = 0; j < spec->num_reducers; ++j)
                status[i][j] = jobstatus::mapper_done;

        pthread_mutex_lock(&mutex);
        if (!jobs.empty())
        {
            job_index = jobs.front();
            jobs.pop();
        }
        mapper_finished_cnt += finished_index.size();
        if (mapper_finished_cnt == spec->num_inputs) // all the mappers finished.
        {
            fprintf(stderr, "ALL MAPPER DONE!\n");
            for (int i = 0; i < spec->num_reducers; ++i)
                net->send(reducer_net_index(i), netcomm_type::LMR_ASSIGN_REDUCER,
                        form_assign_reducer(string("tmp/tmp_%d_") + to_string(i) +".txt"));
        }
        pthread_mutex_unlock(&mutex);

        if (job_index < 0)
            net->send(net_index, netcomm_type::LMR_CLOSE, nullptr, 0);
        else
            net->send(net_index, netcomm_type::LMR_ASSIGN_MAPPER,
                      form_assign_mapper("tmp/tmp_%d_%d.txt", {job_index}));
    }

    void MapReduce::assign_mapper(const string& output_format, const vector<int>& input_index)
    {
        char *tmp = new char[spec->input_format.size() + 1024];
        MapInput mi;
        Mapper *mapper = CREATE_MAPPER(spec->mapper_class);
        for (int i : input_index)
        {
            sprintf(tmp, spec->input_format.c_str(), i);
            mi.add_file(tmp);
        }
        string outputfile = output_format;
        outputfile.replace(outputfile.find("%d"), 2, to_string(input_index[0]));

        mapper->set_mapinput(&mi);
        mapper->set_numreducer(spec->num_reducers);
        mapper->set_outputfile(outputfile);
        mapper->mapwork();

        net->send(0, netcomm_type::LMR_MAPPER_DONE, form_mapper_done(input_index));

        delete mapper;
        delete[] tmp;
    }

    MapReduce::MapReduce(MapReduceSpecification* _spec, int _index)
    {
        instance = this;
        index = _index;
        spec = _spec;
        setbuf(stdout, nullptr);
        net = new netcomm(_spec->config_file, _index, cb);
        total = spec->num_mappers + spec->num_reducers + 1;

        if (total > net->gettotalnum())
        {
            fprintf(stderr, "Too many mappers and reducers. Please add workers in configuration file.\n");
            exit(1);
        }

        if (spec->num_mappers < 1 || spec->num_reducers < 1)
        {
            fprintf(stderr, "Number of both mappers and reducers must be at least one.\n");
            exit(1);
        }

        if (_index > 0)
        {
            type = workertype::worker;
            net->send(0, netcomm_type::LMR_CHECKIN, nullptr, 0);
            isready = true;
        }
        else
        {
            type = workertype::master;
            pthread_mutex_init(&mutex, nullptr);
            if (!dist_run_files())
            {
                fprintf(stderr, "distribution error. cannot run workers.\n");
                net->wait();
                exit(1);
            }
            for (int i = 0; i < spec->num_inputs; ++i)
                jobs.push(i);
            status.resize(spec->num_inputs);
            for (int i = 0; i < spec->num_inputs; ++i)
                status[i].resize(spec->num_reducers);
            system("rm -rf tmp/ && mkdir tmp");

            while (!isready)
                sleep_us(1000);

            pthread_mutex_lock(&mutex); // protect jobs queue
            for (int i = 0; i < spec->num_mappers; ++i)
            {
                if (jobs.empty())
                    net->send(mapper_net_index(i), netcomm_type::LMR_CLOSE, nullptr, 0);
                else
                {
                    net->send(mapper_net_index(i), netcomm_type::LMR_ASSIGN_MAPPER,
                              form_assign_mapper("tmp/tmp_%d_%d.txt", {jobs.front()}));
                    jobs.pop();
                }
            }
            pthread_mutex_unlock(&mutex);
        }
    }

    MapReduce::~MapReduce()
    {
        delete net;
        if (index == 0) pthread_mutex_destroy(&mutex);
    }

    int MapReduce::work(MapReduceResult& result)
    {
        time_point<chrono::high_resolution_clock> start = high_resolution_clock::now();
        while (!stopflag)
            sleep_us(1000);
        result.timeelapsed = duration_cast<duration<double>>(high_resolution_clock::now() - start).count();
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
                cmd += " && (nohup ./" + spec->program_file + " " + to_string(p2.first) +
                       " >& output/output" + to_string(p2.first) + ".txt &)";
            }
            if (p.first == "127.0.0.1")
                system(cmd.c_str());
            else if (!run_sshcmd(p.first, username, password, cmd))
                return false;
        }
        return true;
    }
}