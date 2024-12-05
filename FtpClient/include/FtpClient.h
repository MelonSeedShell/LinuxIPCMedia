#pragma once
#include <string>

class FtpClient
{
public:
    FtpClient(/* args */);
    ~FtpClient();
    int init(const std::string& addr, int& port);
    int deinit();
    int login(const std::string& user, const std::string& pwd);
    int upload(const std::string& srcFile, const std::string& dstFile);
private:
    int sendData(const char* data, const int& size);
    int recvData(char* buf, const int& bufLen);
    int connectState();

private:
    int m_port = -1;
    std::string m_addr;

    int m_socket = -1;
    bool m_cnnet = false;
};

