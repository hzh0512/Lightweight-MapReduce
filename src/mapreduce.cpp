#include "mapreduce.h"

namespace lmr
{
    MapReduce* instance = nullptr;
    pthread_mutex_t mutex;
    int number_checkin = 0, real_total = -1;

    void cb(header* h, char* data, netcomm* net)
    {
        string input_format, output_format;
        vector<int> input_index, finished_indices;
        switch (h->type)
        {
            case netcomm_type::LMR_CHECKIN:
                pthread_mutex_lock(&mutex);
                ++number_checkin;
                if (number_checkin % (real_total - 1) == 0)
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
        bool finished = false;

        net->send(net_index, netcomm_type::LMR_CLOSE, nullptr, 0);

        pthread_mutex_lock(&mutex);
        if (++reducer_finished_cnt == spec->num_reducers)
            finished = true;
        pthread_mutex_unlock(&mutex);

        if (finished)
        {
            fprintf(stderr, "ALL WORK DONE!\n");
            system("rm -rf tmp/");
            stopflag = true;
        }
    }

    void MapReduce::mapper_done(int net_index, const vector<int>& finished_index)
    {
        int job_index = -1;

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

    MapReduce::MapReduce(MapReduceSpecification* _spec)
    {
        instance = this;
        setbuf(stdout, nullptr);
        pthread_mutex_init(&mutex, nullptr);
        set_spec(_spec);
    }

    void MapReduce::set_spec(MapReduceSpecification *_spec)
    {
        if (!_spec) return;
        spec = _spec;

        total = spec->num_mappers + spec->num_reducers + 1;
        if (firstspec)
        {
            firstspec = false;
            real_total = total;
            index = spec->index;
            net = new netcomm(spec->config_file, spec->index, cb);
        }

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
    }

    void MapReduce::start_work()
    {
        if (!spec)
        {
            fprintf(stderr, "No specification.\n");
            exit(1);
        }

        reducer_finished_cnt = mapper_finished_cnt = 0;

        if (index > 0)
        {
            firstrun = false;
            net->send(0, netcomm_type::LMR_CHECKIN, nullptr, 0);
        }
        else
        {
            if (firstrun && !dist_run_files())
            {
                fprintf(stderr, "distribution error. cannot run workers.\n");
                net->wait();
                exit(1);
            }
            firstrun = false;

            for (int i = 0; i < spec->num_inputs; ++i)
                jobs.push(i);
            system("rm -rf tmp/ && mkdir tmp");

            while (!isready)
                sleep_us(1000);
            isready = false;

            for (int i = total; i < real_total; ++i)
                net->send(i, netcomm_type::LMR_CLOSE, nullptr, 0);

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
        if (net)
        {
            net->wait();
            delete net;
        }
        pthread_mutex_destroy(&mutex);
    }

    int MapReduce::work(MapReduceResult& result)
    {
        time_point<chrono::high_resolution_clock> start = high_resolution_clock::now();
        start_work();
        while (!stopflag)
            sleep_us(1000);
        stopflag = false;
        result.timeelapsed = duration_cast<duration<double>>(high_resolution_clock::now() - start).count();
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
            fprintf(stderr, "Authentication failure.\n");
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

    int getch()
    {
        struct termios oldattr, newattr;
        int ch;
        tcgetattr(STDIN_FILENO, &oldattr);
        newattr = oldattr;
        newattr.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
        ch = getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
        return ch;
    }

    void getpass(string& pass)
    {
        char c;
        pass.clear();
        printf("please input your password: ");
        while ((c = getch()) != '\n')
            pass.push_back(c);
        putchar('\n');
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
        if (password == "*")
            getpass(password);
        else
            password = base64_decode(password);

        // run workers
        unordered_map<string, vector<pair<int,int>>> um;
        for (int i = 1; i < real_total; ++i)
            um[net->endpoints[i].first].push_back(make_pair(i, net->endpoints[i].second));

        for (auto &p : um)
        {
            string cmd = "cd " + cwd + " && mkdir -p output";
            for (auto &p2 : p.second)
            {
                cmd += " && (./" + spec->program_file + " " + to_string(p2.first) +
                       " >& output/output_" + to_string(p2.first) + ".txt &)";
            }
            if (p.first == "127.0.0.1")
                system(cmd.c_str());
            else if (!run_sshcmd(p.first, username, password, cmd))
                return false;
        }
        return true;
    }
}