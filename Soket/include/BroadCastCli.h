#pragma once
#include <string.h>
#include <string>
#include <map>
#include <thread>
#include <memory>
#include <functional>

class BroadCastCli
{
public:
    BroadCastCli(/* args */);
    ~BroadCastCli();

    int init(const std::string& ip, const int& port);
    int deinit();
    int setSvrIp(const std::string& svrip);
    int recv(char* data, int& recvLen, const int& maxLen, const int& timeSec);//timeSec, = -1:block, = 0:noblock, > 0:time out  
    int send(const char* data, const int& len, const int& timeSec);
private:
    /* data */
    std::string m_ip;
    std::string m_svrIp;
    int m_port = -1;
    int m_socketFd = -1;
    void* m_localAddr = nullptr;
    void* m_broadcastAddr = nullptr;
    bool m_fdBlock = true;

    bool m_init = false;
};


