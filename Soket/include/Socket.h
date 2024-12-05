#pragma once
#include <string>

class Socket
{
public:
    Socket(/* args */);
    ~Socket();

    int init(const std::string& addr, int& port);
    int deinit();
    int send(const char* data, const int& size);
    int recv(char* buf, const int& bufLen);
private:
    /* data */
};

Socket::Socket(/* args */)
{
}

Socket::~Socket()
{
}
