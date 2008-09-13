#include <cassert>
#include "logWriter.h"

int main(int, char**)
{
    LOG("hello",ERROR);
    assert(1);
    assert(0);

}
