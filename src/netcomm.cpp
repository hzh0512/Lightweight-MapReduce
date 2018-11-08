#include "netcomm.h"

namespace lmr
{
    typedef struct
    {
        char *data;
        pcbfun cbfun;
        netcomm *net;
    } pthread_data;

    void* pthread_cb(void* _data)
    {
        pthread_data *pd = (pthread_data*) _data;
        pd->cbfun(*(header*)(pd->data), pd->data + sizeof(header), pd->net);
        delete[] pd->data;
        delete (char*)_data;
        return nullptr;
    }

    void read_cb(struct bufferevent *bev, void *ctx)
    {
        header h;
        int len = 0;
        netcomm *net = (netcomm*)ctx;
        struct evbuffer *input = bufferevent_get_input(bev);
        while (len = evbuffer_get_length(input))
        {
            if (len < sizeof(header))
                break;

            evbuffer_copyout(input, &h, sizeof(header));
            if (len < sizeof(header) + h.length)
                break;
            len = sizeof(header) + h.length;

            char *data = new char[len];
            bufferevent_read(bev, data, len);

            if (h.type == netcomm_type::LMR_HELLO)
            {
                int remote_index = h.src;
                net->net_buffer[remote_index] = bev;
                net->net_um[bev] = remote_index;
                delete[] data;
                fprintf(stderr, "connected from %d\n", h.src);
            }else if (net->cbfun){
                pthread_t ntid;
                pthread_data *pd = new pthread_data;
                pd->data = data;
                pd->cbfun = net->cbfun;
                pd->net = net;
                pthread_create(&ntid, nullptr, pthread_cb, pd);
            }
        }
    }


    void event_cb(struct bufferevent *bev, short events, void *ctx)
    {
        netcomm *net = (netcomm*)ctx;
        int remote_index = net->net_um[bev];

        if (events & BEV_EVENT_CONNECTED) {
            fprintf(stderr, "connected to target %d.\n", remote_index);
            header h;
            h.src = net->myindex;
            h.dst = remote_index;
            h.length = 0;
            h.type = netcomm_type::LMR_HELLO;
            h.version = CURVERSION;

            bufferevent_write(bev, &h, sizeof(h));

            net->net_buffer[remote_index] = bev;
            return;
        }

        if (events & BEV_EVENT_EOF) {
            fprintf(stderr, "Connection to %d closed.\n", remote_index);
        }else if (events & BEV_EVENT_ERROR) {
            fprintf(stderr, "Got an error %s, connection to %d closed.\n",
                    evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()), remote_index);
            exit(1);
        }else{
            return;
        }
        net->net_um.erase(bev);
        net->net_buffer[remote_index] = nullptr;
        bufferevent_free(bev);
    }


    void listener_cb(struct evconnlistener *listener, evutil_socket_t fd,
                     struct sockaddr *sa, int socklen, void *ctx)
    {
        netcomm *net = (netcomm*)ctx;
        struct bufferevent *bev;

        bev = bufferevent_socket_new(net->net_base, fd, BEV_OPT_CLOSE_ON_FREE);

        bufferevent_setcb(bev, read_cb, nullptr, event_cb, net);
        bufferevent_enable(bev, EV_READ|EV_WRITE);
    }

    void accept_error_cb(struct evconnlistener *listener, void *ctx)
    {
        struct event_base *base = (struct event_base *) ctx;
        int err = EVUTIL_SOCKET_ERROR();
        fprintf(stderr, "Got an error %d (%s) on the listener. "
                        "Shutting down.\n", err, evutil_socket_error_to_string(err));

        exit(1);
    }

    netcomm::netcomm(string configfile, int index, pcbfun f)
    {
        myindex = index;
        cbfun = f;
        readconfig(configfile);
        net_init();
    }

    netcomm::~netcomm()
    {
        for (int i = 0; i < net_buffer.size(); ++i)
        {
            if (net_buffer[i]) {
                bufferevent_free(net_buffer[i]);
            }
        }
        event_base_loopexit(net_base, NULL);
        event_base_free(net_base);
    }

    int netcomm::gettotalnum()
    {
        return net_buffer.size();
    }

    void netcomm::send(int dst, unsigned short type, char *src, int size)
    {
        header h;
        if (!net_buffer[dst])
            net_connect(dst);
        h.src = myindex;
        h.dst = dst;
        h.version = CURVERSION;
        h.length = size;
        h.type = type;
        bufferevent_write(net_buffer[dst], &h, sizeof(header));
        if (size && src)
            bufferevent_write(net_buffer[dst], src, size);
    }

    void netcomm::send(int dst, unsigned short type, string data)
    {
        header h;
        if (!net_buffer[dst])
            net_connect(dst);
        h.src = myindex;
        h.dst = dst;
        h.version = CURVERSION;
        h.length = data.size() + 1;
        h.type = type;
        bufferevent_write(net_buffer[dst], &h, sizeof(header));
        if (data.size())
            bufferevent_write(net_buffer[dst], data.c_str(), data.size() + 1);
    }


    void netcomm::readconfig(string &configfile)
    {
        ifstream f(configfile);
        uint16_t port;
        string ip;
        while (f >> ip >> port)
            endpoints.emplace_back(make_pair(ip, port));
        net_buffer.resize(endpoints.size());
        if (endpoints.empty())
        {
            fprintf(stderr, "invalid endpoints configure file.\n");
            exit(1);
        }
    }

    void netcomm::net_connect(int index)
    {
        struct bufferevent *bev;
        struct sockaddr_in sin;

        if (net_buffer[index])
            return;

        memset((char*)&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_addr.s_addr = inet_addr(endpoints[index].first.c_str());
        sin.sin_port = htons(endpoints[index].second);

        bev = bufferevent_socket_new(net_base, -1, BEV_OPT_CLOSE_ON_FREE);
        net_um[bev] = index;
        bufferevent_enable(bev, EV_READ|EV_WRITE);
        bufferevent_setcb(bev, read_cb, nullptr, event_cb, this);
        bufferevent_socket_connect(bev, (struct sockaddr *)&sin, sizeof(sin));

        while (!net_buffer[index])
            sleep_us(1000);
    }

    void netcomm::net_init()
    {
        pthread_t ntid;
        struct sockaddr_in sin;

        evthread_use_pthreads();
        net_base = event_base_new();

        memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_port = htons(endpoints[myindex].second);

        listener = evconnlistener_new_bind(net_base, listener_cb, this,
                                           LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE|LEV_OPT_THREADSAFE, -1,
                                           (struct sockaddr*)&sin, sizeof(sin));

        if(!listener)
        {
            perror("could't not create listener");
            exit(1);
        }
        evconnlistener_set_error_cb(listener, accept_error_cb);

        pthread_create (&ntid, nullptr, [] (void *arg) -> void * {
                            event_base_dispatch((struct event_base *)arg);
                            return nullptr;
                        }, net_base);
    }

}