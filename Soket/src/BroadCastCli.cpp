#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <errno.h>
#include <cerrno>
#include <fcntl.h>
#include "BroadCastCli.h"

BroadCastCli::BroadCastCli(/* args */)
{
}

BroadCastCli::~BroadCastCli()
{
}

int BroadCastCli::init(const std::string& ip, const int& port)
{
    if (m_socketFd > 0) {
        return -1;
    }

    m_ip = ip;
    m_port = port;

    m_socketFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_socketFd < 0) {
        std::cerr << "创建套接字失败: " << strerror(errno) << std::endl;
        return -1;
    }

    // 绑定本地地址和端口（客户端也需要绑定，以便接收响应）
    // struct sockaddr_in localAddr;
    m_localAddr = new struct sockaddr_in;
    struct sockaddr_in *localAddr = static_cast<struct sockaddr_in *>(m_localAddr);
    memset(localAddr, 0, sizeof(localAddr));
    localAddr->sin_family = AF_INET;
    localAddr->sin_addr.s_addr = INADDR_ANY;
    localAddr->sin_port = htons(0);  // 让系统自动分配端口
    if (bind(m_socketFd, (struct sockaddr*)localAddr, sizeof(*localAddr)) < 0) {
        std::cerr << "绑定地址失败: " << strerror(errno) << std::endl;
        close(m_socketFd);
        return -1;
    }

    // 配置广播地址结构体
    // struct sockaddr_in broadcastAddr;
    m_broadcastAddr = new struct sockaddr_in;
    struct sockaddr_in *broadcastAddr = static_cast<struct sockaddr_in *>(m_broadcastAddr);
    memset(broadcastAddr, 0, sizeof(sockaddr_in));
    broadcastAddr->sin_family = AF_INET;
    broadcastAddr->sin_port = htons(m_port);
    broadcastAddr->sin_addr.s_addr = inet_addr(m_ip.c_str());  // 对应网段 192.168.0.1 的广播地址
    // broadcastAddr->sin_addr.s_addr = INADDR_BROADCAST;
    
    m_init = true;
    return 0;
}
int BroadCastCli::deinit()
{
    m_init = false;
    if (m_broadcastAddr) {
        delete m_broadcastAddr;
    }

    if (m_localAddr) {
        delete m_localAddr;
    }

    if (m_socketFd) {
        close(m_socketFd);
        m_socketFd = -1;
    }

    return 0;
}
int BroadCastCli::recv(char* data, int& recvLen, const int& maxLen, const int& timeSec)
{
    if (!m_init) {
        return -1;
    }

    if (timeSec < 0) {
        if (!m_fdBlock) {
            int flags = fcntl(m_socketFd, F_GETFL, 0);
            if (flags == -1) {
                std::cerr << "获取套接字标志失败: " << strerror(errno) << std::endl;
                return -1;
            }

            if (fcntl(m_socketFd, F_SETFL, flags & ~O_NONBLOCK) == -1) {
                std::cerr << "设置套接字为阻塞模式失败: " << strerror(errno) << std::endl;
                return -1;
            }

            m_fdBlock = true;
        }
    } else if (timeSec == 0) {
        if (m_fdBlock) {
            int flags = fcntl(m_socketFd, F_GETFL, 0);
            if (flags == -1) {
                std::cerr << "获取套接字标志失败: " << strerror(errno) << std::endl;
                return -1;
            }
            if (fcntl(m_socketFd, F_SETFL, flags | O_NONBLOCK) == -1) {
                std::cerr << "设置套接字为非阻塞模式失败: " << strerror(errno) << std::endl;
                return -1;
            }
            m_fdBlock = false;
        }
    } else {
        if (m_fdBlock) {
            int flags = fcntl(m_socketFd, F_GETFL, 0);
            if (flags == -1) {
                std::cerr << "获取套接字标志失败: " << strerror(errno) << std::endl;
                return -1;
            }
            if (fcntl(m_socketFd, F_SETFL, flags | O_NONBLOCK) == -1) {
                std::cerr << "设置套接字为非阻塞模式失败: " << strerror(errno) << std::endl;
                return -1;
            }
            m_fdBlock = false;
        }

        // 设置超时时间结构体
        struct timeval timeout;
        timeout.tv_sec = timeSec;
        timeout.tv_usec = 0;

        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(m_socketFd, &readfds);

        int activity = select(m_socketFd + 1, &readfds, nullptr, nullptr, &timeout);
        if (activity < 0) {
            std::cerr << "select错误: " << strerror(errno) << std::endl;
            return -1;
        } else if (activity == 0) {
            std::cerr << "接收超时" << std::endl;
            return -1;
        } else if (!FD_ISSET(m_socketFd, &readfds)) {
            std::cerr << "FD_ISSET 失败: " << strerror(errno) << std::endl;
            return -1;
        }
    }
    struct sockaddr_in *broadcastAddr = static_cast<struct sockaddr_in *>(m_broadcastAddr);
    socklen_t addrLen = sizeof(*broadcastAddr);
    ssize_t recvSize = recvfrom(m_socketFd, data, maxLen - 1, 0, (struct sockaddr*)broadcastAddr, &addrLen);
    if (recvSize < 0) {
        std::cerr << "接收失败: " << strerror(errno) << std::endl;
    }
    recvLen = recvSize;
    return 0;
}
int BroadCastCli::send(const char* data, const int& len, const int& timeSec)
{
    if (!m_init) {
        return -1;
    }

    if (!data) {
        return -1;
    }

    if (timeSec < 0) {
        if (!m_fdBlock) {
            int flags = fcntl(m_socketFd, F_GETFL, 0);
            if (flags == -1) {
                std::cerr << "获取套接字标志失败: " << strerror(errno) << std::endl;
                return -1;
            }

            if (fcntl(m_socketFd, F_SETFL, flags & ~O_NONBLOCK) == -1) {
                std::cerr << "设置套接字为阻塞模式失败: " << strerror(errno) << std::endl;
                return -1;
            }

            m_fdBlock = true;
        }
    } else if (timeSec == 0) {
        if (m_fdBlock) {
            int flags = fcntl(m_socketFd, F_GETFL, 0);
            if (flags == -1) {
                std::cerr << "获取套接字标志失败: " << strerror(errno) << std::endl;
                return -1;
            }
            if (fcntl(m_socketFd, F_SETFL, flags | O_NONBLOCK) == -1) {
                std::cerr << "设置套接字为非阻塞模式失败: " << strerror(errno) << std::endl;
                return -1;
            }
            m_fdBlock = false;
        }
    } else {
        if (m_fdBlock) {
            int flags = fcntl(m_socketFd, F_GETFL, 0);
            if (flags == -1) {
                std::cerr << "获取套接字标志失败: " << strerror(errno) << std::endl;
                return -1;
            }
            if (fcntl(m_socketFd, F_SETFL, flags | O_NONBLOCK) == -1) {
                std::cerr << "设置套接字为非阻塞模式失败: " << strerror(errno) << std::endl;
                return -1;
            }
            m_fdBlock = false;
        }

        // 设置超时时间结构体
        struct timeval timeout;
        timeout.tv_sec = timeSec;
        timeout.tv_usec = 0;

        fd_set writefds;
        FD_ZERO(&writefds);
        FD_SET(m_socketFd, &writefds);

        int activity = select(m_socketFd + 1, &writefds, nullptr, nullptr, &timeout);
        if (activity < 0) {
            std::cerr << "select错误: " << strerror(errno) << std::endl;
            return -1;
        } else if (activity == 0) {
            std::cerr << "接收超时" << std::endl;
            return -1;
        } else if (!FD_ISSET(m_socketFd, &writefds)) {
            std::cerr << "FD_ISSET 失败: " << strerror(errno) << std::endl;
            return -1;
        }
    }

    struct sockaddr_in sendAddr;
    memset(&sendAddr, 0, sizeof(sendAddr));
    sendAddr.sin_family = AF_INET;
    sendAddr.sin_port = htons(m_port);
    if (m_svrIp == "" || m_svrIp.empty()) {
        sendAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  // 这里假设服务端在本地，根据实际修改
    } else {
        sendAddr.sin_addr.s_addr = inet_addr(m_svrIp.c_str());
    }
    
    socklen_t sendAddrLen = sizeof(sendAddr);
    ssize_t sendSize = sendto(m_socketFd, data, len, 0, (struct sockaddr*)&sendAddr, sendAddrLen);
    if (sendSize < 0) {
        std::cerr << "发送失败: " << strerror(errno) << std::endl;
        return -1;
    }

    return 0;
}

int BroadCastCli::setSvrIp(const std::string& svrip)
{
    m_svrIp = svrip;
    return 0;
}