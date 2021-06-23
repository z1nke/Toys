#include <cassert>
#include <iostream>
#include <string>

class ProgressBar {
public:
    ProgressBar(std::size_t total, std::string prefix, std::string suffix) :
        total(total), prefix(std::move(prefix)), suffix(std::move(suffix)) { }

    void show(std::size_t progress) const {
        // prefix: |    bar    | III.DD% suffix
        double percent = 100.0 * progress / total;
        std::size_t finishLen = 
            static_cast<std::size_t>(percent / 100.0 * kBarLen);
        assert(finishLen <= kBarLen);

        // write stdout buffer
        std::string barbuf;
        barbuf.reserve(kBarLen + 1);
        for (int i = 0; i < finishLen; ++i) {
            barbuf += '#';
        }
        for (int i = 0; i < kBarLen - finishLen; ++i) {
            barbuf += '-';
        }

        int outLen = std::printf("%s: |%s| %3.2lf%% %s", prefix.c_str(),
                                 barbuf.c_str(), percent, suffix.c_str());
        std::fflush(stdout);

        // clear stdout buffer
        std::string backspaceBuf(outLen, '\b');
        printf("%s", backspaceBuf.c_str());

        if (progress == total) {
            printf("\n");
        }
    }

private:
    static const std::size_t kBarLen = 50;

    std::size_t total;
    std::string prefix;
    std::string suffix;
};

