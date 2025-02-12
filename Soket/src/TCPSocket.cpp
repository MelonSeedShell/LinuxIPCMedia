#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <algorithm>
#include "TCPSocket.h"


TCPSocket::TCPSocket(const bool& isSvr, const int& maxListen = 5):m_isSvr(isSvr),m_maxListen(maxListen)
{
    m_socketFd = -1;

}

TCPSocket::~TCPSocket()
{
}

int TCPSocket::init(const std::string& address, const int& port) {
    
    m_socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socketFd < 0) {
        std::cerr << "Error creating TCP socket" << std::endl;
        return -1;
    }

    if (m_isSvr) {
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = inet_addr(address.c_str());

        if (bind(m_socketFd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            std::cerr << "Error binding TCP socket" << std::endl;
            return -1;
        }

        if (listen(m_socketFd, m_maxListen) < 0) {
            std::cerr << "Error listening on TCP socket" << std::endl;
            return -1;
        }
    }
    m_address = address;
    m_port = port;
    // 这里可以添加更多服务器相关逻辑，比如监听、接受连接等（示例暂简化）
    return 0;
}

int TCPSocket::sendData(const int& handle, const char* data, const int& size) {
    if (m_socketFd < 0) {
        std::cerr << "Socket not initialized or already closed" << std::endl;
        return -1;
    }

    int socket = -1;
    if (m_isSvr) {
        socket = handle;
    } else {
        socket = m_socketFd;
    }

    if (socket < 0) {
        std::cerr << "Socket not initialized or already closed" << std::endl;
        return -1;
    }

    ssize_t bytesSent = ::send(socket, data, size, 0);
    if (bytesSent < 0) {
        std::cerr << "Error sending data" << std::endl;
        if (m_isSvr) {
            close(socket);
            auto it = std::find(m_accepts.begin(), m_accepts.end(), socket);
            if (it!= m_accepts.end()) {
                m_accepts.erase(it);
            }
        }
        return -1;
    }
    return static_cast<int>(bytesSent);
}

int TCPSocket::receiveData(const int& handle, char* buffer, const int& size) {
    if (m_socketFd < 0) {
        std::cerr << "Socket not initialized or already closed" << std::endl;
        return -1;
    }

    int socket = -1;
    if (m_isSvr) {
        socket = handle;
    } else {
        socket = m_socketFd;
    }

    ssize_t bytesReceived = ::recv(m_socketFd, buffer, size, 0);
    if (bytesReceived < 0) {
        std::cerr << "Error receiving data" << std::endl;
        if (m_isSvr) {
            close(socket);
            auto it = std::find(m_accepts.begin(), m_accepts.end(), socket);
            if (it!= m_accepts.end()) {
                m_accepts.erase(it);
            }
        }
        return -1;
    }
    return static_cast<int>(bytesReceived);
}

void TCPSocket::deinit() 
{
    for (auto it = m_accepts.begin(); it!= m_accepts.end();) {
        if (*it > 0) {
            ::close(*it);
        }
        it = m_accepts.erase(it);
    }
    if (m_socketFd >= 0) {
        ::close(m_socketFd);
        m_socketFd = -1;
    }
}

int TCPSocket::svrAccept()
{
    if (!m_isSvr) {
        std::cerr << "This function can only be used in server mode" << std::endl;
        return -1;
    }

    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    int clientSocket = accept(m_socketFd, (struct sockaddr *)&clientAddr, &clientAddrLen);
    if (clientSocket < 0) {
        std::cerr << "Error accepting client connection" << std::endl;
        return -1;
    }
    m_accepts.push_back(clientSocket);
    return clientSocket;
}

int TCPSocket::cliConnect()
{
    if (m_socketFd < 0) {
        std::cerr << "Socket not initialized or already closed" << std::endl;
        return -1;
    }

    if (m_isSvr) {
        std::cerr << "This function can only be used in client mode" << std::endl;
        return -1;
    }
    int ret = 0;
    // 作为客户端，准备连接服务器
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(m_port);
    serverAddr.sin_addr.s_addr = inet_addr(m_address.c_str());
    // 连接服务器
    if ((ret = connect(m_socketFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr))) < 0) {
        std::cerr << "Error connecting to server" << std::endl;
        return -1;
    }

    return ret;
}