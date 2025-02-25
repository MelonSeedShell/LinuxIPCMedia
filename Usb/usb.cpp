#include <iostream>
#include <string>
#include <string.h>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <thread>
#include <mutex>

#include <stdarg.h>
#include <stdio.h>
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
    int ret = 0;
    std::string node = "/dev/usbss";
    std::mutex mtx;
    int fd = -1;
    if (fd <= 0) {
        struct stat buf;
        ret = stat(node.c_str(), &buf);
        fd = open(node.c_str(), O_RDWR);
        if (fd <= 0) {
            LOG("open %s failed \n", node.c_str());
            return -1;
        }
    }

    std::thread t([&fd, &mtx](){
        char recvBuf[128];
        while (fd > 0) {
            memset(recvBuf, 0, sizeof(recvBuf));
            mtx.lock();
            int readLen = read(fd, recvBuf, sizeof(recvBuf));
            if (readLen > 0) {
                LOG("recvBuf:%s\n", recvBuf);
            } else if (readLen == -1) {
                close(fd);
                fd = -1;
            }
            mtx.unlock();

            sleep(1);
        }
    });
    
    while (1) {
        std::string line;
        std::getline(std::cin, line);
        if (line == "q") {
            break;
        }
        LOG("\n");
        // mtx.lock();
        int ret = write(fd, line.c_str(), line.size());
        LOG("\n");
        ret = write(fd, "\r\n", sizeof("\r\n"));
        // mtx.unlock();
        LOG("\n");
        if (ret < 0) {
            LOG("send failed\n");
        }
        LOG("send:%s\n", line.c_str());
    }
    mtx.lock();
    close(fd);
    fd = -1;
    mtx.unlock();
    t.join();
    return 0;
}
