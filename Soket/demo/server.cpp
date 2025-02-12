#include <iostream>
#include <stdarg.h>
#include <stdio.h>
#include <sys/time.h>

#include "BroadCastSvr.h"

static void testLog(const char *func, int line, char *fmt, ...)
{
    va_list args;
    fprintf(stdout, "[%s-%d]:", func, line);
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
}

#define LOG(fmt, args...) testLog(__func__, __LINE__, fmt, ##args)



int main(int argc, char const *argv[])
{
    /* code */
    int ret = 0;
    BroadCastSvr svr;
    ret = svr.init("192.168.3.255", 9000);
    if (ret < 0) {
        LOG("cli init failed\n");
        return -1;
    }
    bool startRecv = true;
    std::shared_ptr<std::thread> recvThread = std::make_shared<std::thread>([&svr, &startRecv](){
        int ret = 0;
        char* buf = new char[1024];
        while (startRecv) {
            int recvLen = 0;
            ret = svr.recv(buf, recvLen, sizeof(buf), -1);
            if (ret < 0) {
                LOG("ERR: recv failed\n");
                continue;
            }
            LOG("buf:%s\n", buf);
        }
        delete[] buf;
    });

    while (1) {
        std::string line;
        std::cin >> line;
        if ("quit" == line) {
            break;
        }
        ret = svr.send(line.c_str(), line.size(), 0);
        if (ret < 0) {
            LOG("send failed\n");
        }
        LOG("send line:%s\n", line.c_str());
    }
    startRecv = false;
    recvThread->join();
    svr.deinit();


    return 0;
}
