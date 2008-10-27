#include <cassert>
#include "logWriter.h"
#include "mapMsg.h"

int main(int, char**)
{
    std::vector<double> v(10),v2;
    v[1] = 1.12;
    v[2] = 21.12;
    v[3] = 31.12;
    v[4] = 41.12;
    MapMsg msg("try");
    msg["vector"] = v;
    v2 = msg["vector"];
    assert(v == v2);
    return 0;
}
