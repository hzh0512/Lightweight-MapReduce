#include "../src/register.h"
#include <iostream>

using namespace std;
using namespace lmr;

class A
{
public:
    virtual void print(string s)
    {
        cout << "A: " << s << endl;
    }
};

class B : public A
{
public:
    void print(string s)
    {
        cout << "B: " << s << endl;
    }
};

BASE_CLASS_REGISTER(A)
CHILD_CLASS_REGISTER(A, B)

int main()
{
    A* b = CHILD_CLASS_CREATOR(A, "B");
    b->print("abc");
    delete b;
    return 0;
}
