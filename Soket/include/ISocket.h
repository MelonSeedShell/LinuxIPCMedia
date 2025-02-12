#pragma once
#include <string>

class ISocket
{

public:
    ISocket(/* args */) = default;
    virtual ~ISocket() = default;

    // 纯虚函数，用于初始化套接字，由具体子类实现
    virtual int init(const std::string& address, const int& port) = 0;

    // 纯虚函数，用于发送数据，由具体子类实现
    virtual int sendData(const int& handle, const char* data, const int& size) = 0;

    // 纯虚函数，用于接收数据，由具体子类实现
    virtual int receiveData(const int& handle, char* buffer, const int& size) = 0;

    // 纯虚函数，用于关闭套接字，由具体子类实现
    virtual void deinit() = 0;

    virtual int svrAccept() = 0;//return handle

    virtual int cliConnect() = 0;//return handle
private:
    /* data */
};



