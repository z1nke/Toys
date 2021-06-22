#include "progressbar.hpp"

#include <chrono>
#include <thread>

using namespace std::chrono_literals;

int main() {
    std::size_t total = 200;
    ProgressBar bar{total, "Progress", "Complete"};
    bar.show(0);
    for (int i = 1; i <= 200; ++i) {
        bar.show(i);
        std::this_thread::sleep_for(10ms);
    }
    return 0;
}