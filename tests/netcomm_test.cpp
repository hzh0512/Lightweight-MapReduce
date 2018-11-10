#include "../src/netcomm.h"

using namespace std;
using namespace lmr;

void cb(header* h, char* data, int len);

int sum = 0;

void cb(header* h, char* data, netcomm* net)
{
    if (h->type == 100)
    {
        sum += stoi(data);
        printf("%d\n", sum);
        net->send(h->src, 101, to_string(sum));
    }else if (h->type == 101){
        printf("%s\n", data);
        exit(0);
    }
}

int main(int argc, char *argv[])
{
    int index;

    index = atoi(argv[1]);
    netcomm net("config.txt", index, cb);

    if (index)
    {
        net.send(0, 100, to_string(index));
    }

    while (1)
        sleep_us(1000);
    return 0;
}