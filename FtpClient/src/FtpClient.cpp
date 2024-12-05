#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>

#include "FtpClient.h"



FtpClient::FtpClient(/* args */)
{
}

FtpClient::~FtpClient()
{
}

int FtpClient::init(const std::string& addr, int& port)
{
    if (m_socket != -1) {
        std::cerr << "has already inited, not need reinit" << std::endl;
        return -1;
    }
    m_port = port;
    m_addr = addr;

    // 创建套接字
    struct sockaddr_in server_addr;
    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket < 0) {
        perror("socket creation failed\n");
        return -1;
    }
#if 0
    // 将套接字设置为非阻塞模式
    int flags = fcntl(m_socket, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl F_GETFL failed");
        close(m_socket);
        return -1;
    }

    if (fcntl(m_socket, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL failed");
        close(m_socket);
        return -1;
    }
#endif
    // 设置服务器地址结构体
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(m_port);
    if (inet_pton(AF_INET, m_addr.c_str(), &server_addr.sin_addr) <= 0) {
        perror("inet_pton failed\n");
        close(m_socket);
        return -1;
    }

    connect(m_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
#if 0
    if (connectState() < 0) {
        std::cerr << "init failed, Socket connect failed" << std::endl;
        return -1;
    }
#endif
    return 0;
}

int FtpClient::deinit()
{
    if (m_socket == -1) {
        perror("not init\n");
        return 0;
    }

    close(m_socket);
    m_socket = 0;
    m_port = -1;
    return 0;
}

int FtpClient::connectState()
{
    fd_set readFds, writeFds, errorFds;
    FD_ZERO(&readFds);
    FD_ZERO(&writeFds);
    FD_ZERO(&errorFds);
    FD_SET(m_socket, &readFds);
    FD_SET(m_socket, &writeFds);
    FD_SET(m_socket, &errorFds);
    struct timeval timeout;
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;
    int selectResult = select(m_socket + 1, &readFds, &writeFds, &errorFds, &timeout);
    if (selectResult == -1) {
        perror("select failed");
        return -1;
    } else if (selectResult == 0) {
        std::cout << "Connection timed out." << std::endl;
        return -1;
    }

    if (FD_ISSET(m_socket, &errorFds)) {
        std::cerr << "Socket error detected, connection may be lost." << std::endl;
        return -1;
    }

    return 0;
}

int FtpClient::sendData(const char* data, const int& size)
{
    int ret = 0;
    // ret = connectState();
    if (ret < 0) {
        std::cerr << "send failed, Socket connect failed" << std::endl;
        return -1;
    }

    ssize_t bytesSent = send(m_socket, data, size, 0);
    if (bytesSent == -1) {
        if (errno == EPIPE || errno == ECONNRESET) {
            std::cerr << "send failed, Connection was broken before data sent." << std::endl;
        } else {
            perror("send failed");
        }
        return -1;
    }

    std::cout << "Data sent successfully." << std::endl;

    return 0;
}
int FtpClient::recvData(char* buf, const int& bufLen)
{
    int ret = 0;
    // ret = connectState();
    // if (ret < 0) {
    //     std::cerr << "recv failed, Socket connect failed" << std::endl;
    //     return -1;
    // }
    printf("\n");
    int idx = 0;
    while (idx < bufLen) {
        // ret = connectState();
        char c;
        ssize_t currentRecv = recv(m_socket, &c, 1, 0);
        if (currentRecv == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 在非阻塞模式下，暂时没有数据可读，等待一段时间后再次尝试（这里省略等待逻辑）
                perror("recv failed, no data\n");
            } else {
                perror("recv failed\n");
            }

            break;
        } else if (currentRecv == 0) {
            std::cout << "Connection closed before receiving all data." << std::endl;
            break;
        }
        printf("%c", c);
        if (c == '\n') {
            c = ' ';
        }

        buf[idx] = c;
        idx ++;
        // sleep(1);
    }
    printf("\n");
    return 0;
}

int FtpClient::login(const std::string& user, const std::string& pwd)
{
    int ret = 0;
    char *buf = new char[512];
    std::string cmdAuth = "AUTH TLS\n";
    std::string cmdUsr = "USER " + user + "\n";
    std::string cmdPwd = "PASS " + pwd + "\n";

    ret = sendData(cmdAuth.c_str(), cmdAuth.size());
    if (ret < 0) {
        std::cerr << "login ftp send failed" << std::endl;
        return ret;
    }
    sleep(1);
    ret = recvData(buf, sizeof(buf));
    if (ret < 0) {
        std::cerr << "login ftp recv failed" << std::endl;
        return ret;
    }
    std::cout << "cmdAuth recv:" << buf << std::endl;
    sleep(1);
    ret = sendData(cmdUsr.c_str(), cmdUsr.size());
    if (ret < 0) {
        std::cerr << "login ftp send failed" << std::endl;
        return ret;
    }
    sleep(1);
    // while (1) {
        ret = recvData(buf, sizeof(buf));
        if (ret < 0) {
            std::cerr << "login ftp recv failed" << std::endl;
            return ret;
        }
        std::cout << "user recv:" << buf << std::endl;

        // if (strstr(buf, "Please") != NULL) {
        //     break;
        // }


    // }
    
sleep(1);
    ret = sendData(cmdPwd.c_str(), cmdPwd.size());
    if (ret < 0) {
        std::cerr << "login ftp send failed" << std::endl;
        return ret;
    }
sleep(1);
    ret = recvData(buf, sizeof(buf));
    if (ret < 0) {
        std::cerr << "login ftp recv failed" << std::endl;
        return ret;
    }
    std::cout << "pwd recv:" << buf << std::endl;

    delete[] buf;
    return 0;
}
int FtpClient::upload(const std::string& srcFile, const std::string& dstFile)
{
    int ret = 0;
    char *buf = new char[512];
    std::string cmdTrans = "TYPE I\n";
    std::string cmdSTOR = "STOR " + dstFile + "\n";
    ret = sendData(cmdTrans.c_str(), cmdTrans.size());
    if (ret < 0) {
        std::cerr << "ftp send failed" << std::endl;
        return ret;
    }

    ret = recvData(buf, sizeof(buf));
    if (ret < 0) {
        std::cerr << "ftp recv failed" << std::endl;
        return ret;
    }

    std::cout << "Trans recv:" << buf << std::endl;

    ret = sendData(cmdSTOR.c_str(), cmdSTOR.size());
    if (ret < 0) {
        std::cerr << "ftp send failed" << std::endl;
        return ret;
    }

    ret = recvData(buf, sizeof(buf));
    if (ret < 0) {
        std::cerr << "ftp recv failed" << std::endl;
        return ret;
    }

    std::cout << "stor recv:" << buf << std::endl;
    delete[] buf;

    return 0;
}