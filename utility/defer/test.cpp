#include <iostream>

#include "defer.hpp"

using namespace std;

int main() {
    DEFER {
        cout << "defer1\n";
    };

    DEFER {
        cout << "defer2\n";
    };

    cout << 42 << endl;

    return 0;
}