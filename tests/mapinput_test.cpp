#include "../src/mapinput.h"
#include <iostream>

using namespace std;
using namespace lmr;

int main()
{
    MapInput mi;
    string key, value;
    mi.add_file("a.txt");
    mi.add_file("b.txt");
    mi.add_file("c.txt");
    mi.add_file("d.txt");
    while (mi.get_next(key, value))
        cout << key << ": " << value << endl;
    return 0;
}