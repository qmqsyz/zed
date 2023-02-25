# ZED
zed是一个基于C++20在linux系统下实现的高性能分布式服务器框架，参考sylar以及muduo。

# 线程模块
zed::Thread只是对std::thread的封装。参考std::this_thread添加了current_thread::getName()/getTid()/getThread()  
锁使用std::mutex和std::shared_mutex  
同步机制采用std::future实现

# 日志模块
支持流式日志风格写日志，例如：LOG_INFO << "hello world!";  
支持日志级别。  
支持同步和异步写日志。  
支持日志颜色。  
每条日志消息耗时约：1.4us-1.7us

# 配置模块

