#include "tests/test_util.h"
#include "zed/log/log.h"

#include <sys/resource.h>

using namespace zed;

void bench(bool longLog) {
    int cnt = 0;
    const int kBatch = 1000;
    std::string empty = " ";
    std::string longStr(3000, 'X');
    longStr += " ";

    for (int t = 0; t < 30; ++t) {
        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < kBatch; ++i) {
            LOG_INFO << "Hello 0123456789"
                     << " abcdefghijklmnopqrstuvwxyz " << (longLog ? longStr : empty) << cnt;
            ++cnt;
        }
        auto end = std::chrono::steady_clock::now();
        std::cout << (end - start) << '\n';

        struct timespec ts = {0, 500 * 1000 * 1000};
        nanosleep(&ts, NULL);
    }
}

void test(int n, bool flag) {
    std::vector<Thread> vec;
    for (int i = 0; i < n; ++i) {
        vec.emplace_back(std::bind(bench, flag));
    }
    for (int i = 0; i < n; ++i) {
        vec[i].join();
    }
}

int main(int argc, char **argv) {
    size_t kOneGB = 1000 * 1024 * 1024;
    rlimit rl = {2 * kOneGB, 2 * kOneGB};
    setrlimit(RLIMIT_AS, &rl);

    // StdoutLogAppender::Ptr p{new StdoutLogAppender};
    // LoggerManager::Getinstance()->addAppender(p);
    FileLogAppender::Ptr fp(new FileLogAppender("log_test"));
    LoggerManager::GetInstance()->addAppender(fp);
    CalTime(test, 1, argc > 1);
}