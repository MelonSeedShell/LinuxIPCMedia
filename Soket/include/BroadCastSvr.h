#pragma once

#include <string.h>
#include <string>
#include <map>
#include <thread>
#include <memory>
#include <functional>

class BroadCastSvr
{
public:
    BroadCastSvr();
    ~BroadCastSvr();

    int init(const std::string& ip, const int& port);
    int deinit();
    int recv(char* data, int& recvLen, const int& maxLen, const int& timeSec);//timeSec, = -1:block, = 0:noblock, > 0:time out  
    int send(const char* data, const int& len, const int& timeSec);

private:
    /* data */
    std::string m_ip;
    int m_port = -1;
    int m_socketFd = -1;
    bool m_fdBlock = true;
    void* m_broadcastAddr = nullptr;

    bool m_init = false;
    int m_timeSec = -1;

    std::shared_ptr<std::thread> m_listenCnntThread;
    std::map<int, int> m_connetFdMap;
};
