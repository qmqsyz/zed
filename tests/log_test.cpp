#include "zed/log/log.h"
#include "zed/comm/test_util.h"

using namespace zed;

void f(int n){
    for (int i { 0 }; i < n; ++i) {
        LOG_DEBUG << i;
        LOG_INFO << i;
        LOG_WARN << i;
        LOG_ERROR << i;
        LOG_FATAL << i;
    }
}

void test(int n,int m)
{
    std::vector<Thread> vec;
    for (int i = 0; i < n;++i){
        vec.emplace_back(std::bind(f, m));
    }
    for (int i = 0; i < n;++i){
        vec[i].join();
    }
}

int main()
{
    StdoutLogAppender::ptr p { new StdoutLogAppender };
    FileLogAppender::ptr fp(new FileLogAppender("log_test"));
    LOG::instance() ->addAppender(fp);
    LOG::instance()->addAppender(p);
    CalTime(test, 10, 100000);
}