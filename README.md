# ZED
zed是一个基于C++20在linux系统下实现的高性能分布式服务器框架，参考sylar以及muduo。

# 1.日志模块
参照sylar和muduo的日志。  

**日志风格**: 支持流式日志风格，例如：LOG_INFO << "hello world!";  
**日志级别**: 对日志进行分级。使用者可根据需要设置日志级别。  
**异步日志**: 实现异步打印日志信息，不阻塞业务线程。   
**日志颜色**: 实现不同日志不同颜色方便查看。 
**文件输出**: 支持输出日志信息到文件。  
**文件滚动**: 日志文件会自动滚动，当跨天又或者到达滚动上限，自动创建新文件。  
**性能**: 每条日志消息耗时约：1.4us-1.7us


# 2.协程模块  

ZED提供一个简易的**无栈协程并发框架**，虽然简易但是**性能优异**，**可靠**，同时具备**易用性**；

## 2.1.异步IO

对于网络编程需要用到基本IO操作进行了异步IO封装。
已经实现的有 read,write,recv,send,readv,writev,connect,accept

执行异步IO操作十分简单，例如：  
```
char buf[1024];
Socket socket(fd);
auto n = co_await socket.read(buf, sizeof(buf));
```
```
char buf[1024];
auto n = co_await asyn::Read(fd, buf, sizeof(buf));
```
   
利用以上提供的异步IO操作，很容易的搭建一个echo server服务器  
```
#include "zed/coroutine/task.hpp"
#include "zed/log/log.h"
#include "zed/net/address.h"
#include "zed/net/executor.h"
#include "zed/net/socket.h"

using namespace zed;
using namespace zed::net;
using namespace zed::coroutine;

Task<void> handleClient(int fd)
{
    Socket socket(fd);
    char   buf[1024];
    int    n = 0;
    while ((n = co_await socket.recv(buf, sizeof(buf))) > 0) {
        LOG_DEBUG << buf;
        co_await socket.send(buf, n);
    }
}

Task<void> server(Address::Ptr addr)
{
    auto accept = Socket::CreateTCP(addr->getFamily());
    accept.bind(addr);
    accept.listen();
    while (int fd = co_await accept.accept()) {
        Executor::GetCurrentExecutor()->addTask(handleClient(fd));
    }
}

int main()
{
    Address::Ptr  addr = IPv4Address::Create("127.0.0.1", 6666);
    net::Executor ex;
    ex.addTask(server(addr));
    ex.start();
}
```

## 2.2.协程模型

ZED的协程框架采用m:n模式，即m线程对应n协程。  
每个线程都有一个调度器，每个调度器都拥有独立的epoll，通过将时间和协程注册进epoll，当事件触发时将可以执行的协程，推送到全局任务队列，每个调度器从全局任务队获取协程并执行。保证每个协程都在工作，不会出现饥饿的情况。但是也导致更强的线程竞争关系，所以使用了互斥锁。

因为协程可以在不同的线程中运行，所以对于线程局部变量要十分小心，尽可能避免使用。

# 3.NET模块
采用主从Reactor模式，在ZED中reactor就是executor，父reactor负责把接受的连接通过轮询分发给子reactor，子reactor负责处理业务逻辑。因为是m:n模型，不用担心饥饿的情况。ZED中把每个连接当成一个协程处理，当处于阻塞的时候，会向当前的调度器注册事件，然后yeild。  

# 4.Http模块
类似Servlet的，每个http请求都会对应一个servlet。通过预先注册servlet，用于处理不同的http的请求。  
  
**工作流程**  
解析http请求报文，返回request对象   
查找path对应的servlet对象   
使用servlet对象处理request，并返回response对象   
对response对象进行处理，返回http响应报文。  

# 5.性能测试 与muduo对比
处理器: AMD Ryzen 5 3600 6-Core Processor   
服务器环境: Centos7虚拟机，内存4G，CPU4核心  
客户机环境: Centos7虚拟机，内存4G，CPU4核心  
测试工具: wrk: https://github.com/wg/wrk.git  
测试环境: 4IO线程  
测试命令:  
**zed**:  wrk -c 1000 -t 4 -d 30 --latency http://192.168.15.3:8888/qps?id=5  
**muduo**:  wrk -c 1000 -t 4 -d 30 --latency http://192.168.15.3:8888/

# 5.1.ZED性能
```
Running 30s test @ http://192.168.15.3:8888/qps?id=5
  4 threads and 1000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    10.46ms    9.06ms 211.74ms   90.25%
    Req/Sec    26.93k     3.70k   42.95k    80.35%
  Latency Distribution
     50%    8.34ms
     75%   10.07ms
     90%   17.47ms
     99%   48.20ms
  3214930 requests in 30.07s, 653.06MB read
Requests/sec: 106930.25
Transfer/sec:     21.72MB

```
# 5.2.muduo性能
```
Running 10s test @ http://192.168.15.3:8000/
  4 threads and 1000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    10.11ms    7.83ms 218.50ms   85.65%
    Req/Sec    25.49k     6.79k   52.87k    65.83%
  Latency Distribution
     50%    7.85ms
     75%   12.63ms
     90%   19.50ms
     99%   35.66ms
  1015864 requests in 10.09s, 209.26MB read
Requests/sec: 100636.63
Transfer/sec:     20.73MB


```
