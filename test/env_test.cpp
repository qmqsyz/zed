#include "zed/comm/env.h"
#include "zed/log/log.h"
#include <fstream>
#include <iostream>
#include <unistd.h>

int main(int argc, char** argv)
{
    zed::StdoutLogAppender::Ptr p {new zed::StdoutLogAppender};
    zed::LoggerManager::GetInstance().addAppender(p);

    std::cout << "argc=" << argc << std::endl;
    zed::EnvironmentManager::GetInstance().addHelp("s", "start with the terminal");
    zed::EnvironmentManager::GetInstance().addHelp("d", "run as daemon");
    zed::EnvironmentManager::GetInstance().addHelp("p", "print help");
    if (!zed::EnvironmentManager::GetInstance().init(argc, argv)) {
        std::cout << "?" << std::endl;
        zed::EnvironmentManager::GetInstance().printHelp();
        return 0;
    }

    std::cout << "exe=" << zed::EnvironmentManager::GetInstance().getExe() << std::endl;
    std::cout << "cwd=" << zed::EnvironmentManager::GetInstance().getCwd() << std::endl;

    std::cout << "path=" << zed::EnvironmentManager::GetInstance().getEnv("PATH", "xxx")
              << std::endl;
    std::cout << "test=" << zed::EnvironmentManager::GetInstance().getEnv("TEST", "") << std::endl;
    std::cout << "set env " << zed::EnvironmentManager::GetInstance().setEnv("TEST", "yy")
              << std::endl;
    std::cout << "test=" << zed::EnvironmentManager::GetInstance().getEnv("TEST", "") << std::endl;
    if (zed::EnvironmentManager::GetInstance().have("p")) {
        zed::EnvironmentManager::GetInstance().printHelp();
    }
    return 0;
}