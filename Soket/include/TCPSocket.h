#pragma once
#include <string>
#include <thread>
#include <mutex>
#include <memory>
#include <vector>
#include "ISocket.h"

class TCPSocket : public ISocket
{

public:
    TCPSocket(const bool& isSvr, const int& maxListen);
    ~TCPSocket();

    int init(const std::string& address, const int& port) override;
    int sendData(const int& handle, const char* data, const int& size) override;
    int receiveData(const int& handle, char* buffer, const int& size) override;
    void deinit() override;

    int svrAccept() override;

    int cliConnect() override;

private:
    

private:
    /* data */
    int m_socketFd;
    std::string m_address;
    int m_port = -1;
    bool m_isSvr;
    int m_maxListen;
    std::vector<int> m_accepts;
    std::shared_ptr<std::thread> m_listenThread;
};

