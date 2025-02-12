#include "BroadcastClient.h"
#include <iostream>
#include <string>
#include <csignal>
#include <cstdlib>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <condition_variable>
#include <mutex>

// 全局的条件变量和互斥锁
std::condition_variable cv;
std::mutex mtx;
bool stopFlag = false;

static void testLog(const char *func, int line, char *fmt,...)
{
    va_list args;
    fprintf(stdout, "[%s-%d]:", func, line);
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
}

#define LOG(fmt, args...) testLog(__func__, __LINE__, fmt, ##args)

void signal_handler(int signum, siginfo_t* info, void* context) {
    std::cout << "Received SIGTERM signal. Exiting gracefully." << std::endl;

    LOG("info.si_signo:%d\n", info->si_signo);
    LOG("info.si_code:%d\n", info->si_code);
    LOG("info.si_errno:%d\n", info->si_errno);
    LOG("info.si_addr:%p \n", info->si_addr);
    LOG("info.si_status:%s\n", info->si_status);
    LOG("info.si_band:%s\n", info->si_band);
    // 设置停止标志为true，并通知条件变量
    std::lock_guard<std::mutex> guard(mtx);
    stopFlag = true;
    cv.notify_all();
    // 清理资源等操作
    std::exit(0);
}

void signal_cap(void)
{
    struct sigaction stAct;
    sigemptyset(&stAct.sa_mask);
    stAct.sa_flags = SA_SIGINFO;
    stAct.sa_sigaction = signal_handler;

    sigaction(SIGSEGV, &stAct, NULL);
    sigaction(SIGFPE, &stAct, NULL);
    sigaction(SIGABRT, &stAct, NULL);
    sigaction(SIGBUS, &stAct, NULL);
}

int main(int argc, char const *argv[]) {
    signal_cap();
    std::string inputArgv;
    if (argc >= 2 ) {
        inputArgv = std::string(argv[1]);
    } else {
        std::cout << "e.g:./devDiscovery dev_name@192.168.0.1:9000" << std::endl;
        return 0;
    }
    auto devLen = inputArgv.find("@");
    std::string devName = inputArgv.substr(0, devLen);
    auto ipLen = inputArgv.find(":");
    std::string ip = inputArgv.substr(devLen + 1, ipLen - devLen - 1);
    int port = std::stoi(inputArgv.substr(ipLen + 1));

    BroadcastClient client;
    while (!client.init(ip, port)) {
        std::cerr << "Client initialization failed" << std::endl;
        sleep(5);
    }
    client.setRepDevName(devName);
    client.start();

    // 等待停止标志被设置
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, []{ return stopFlag; });
    }

    client.stop();
    return 0;
}