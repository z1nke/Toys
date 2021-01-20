#include "ttcp.hpp"

#include <cassert>

int main(int argc, char** argv) {
    Option opt;
    if (!parseCommandLine(argc, argv, opt)) {
        return 1;
    }

    if (opt.transmit) {
        transmit(opt);
        return 0;
    }

    if (opt.receive) {
        receive(opt);
        return 0;
    }
    assert(false);

    return 0;
}
