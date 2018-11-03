#ifndef LMR_NETCOMM_H
#define LMR_NETCOMM_H

#include <string>
#include <fstream>
#include <memory>
#include <vector>
#include <cstring>
#include <unordered_map>
#include "event2/event.h"
#include "event2/thread.h"
#include "event2/bufferevent.h"
#include "event2/buffer.h"
#include "event2/listener.h"
#include "event2/util.h"
#include <arpa/inet.h>

#include "common.h"

namespace lmr
{
    class netcomm;
    using namespace std;
    const short CURVERSION = 1;

    typedef struct{
        unsigned short version, type;
        unsigned int length, src, dst;
    } header;

    typedef void (*pcbfun)(header, char*, netcomm*);

    enum netcomm_type {
        LMR_HELLO, // type 0: hello message
    };

    class netcomm
    {
        friend void listener_cb(struct evconnlistener *listener, evutil_socket_t fd,
                                struct sockaddr *sa, int socklen, void *ctx);
        friend void read_cb(struct bufferevent *bev, void *ctx);
        friend void event_cb(struct bufferevent *bev, short events, void *ctx);
    public:
        netcomm(string configfile, int index, pcbfun f);
        ~netcomm();
        void send(int dst, unsigned short type, char *src, int size);
        void send(int dst, unsigned short type, string data);
        int gettotalnum();

    private:
        void readconfig(string &configfile);
        void net_init();
        void net_connect(int index);

        vector<struct bufferevent*> net_buffer;
        unordered_map<struct bufferevent*, int> net_um;
        struct event_base *net_base = nullptr;
        struct evconnlistener *listener = nullptr;
        int myindex;
        pcbfun cbfun;
        vector<pair<string, uint16_t>> endpoints;
    };

}

#endif //LMR_NETCOMM_H
