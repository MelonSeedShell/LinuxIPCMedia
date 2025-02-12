#include "BroadcastServer.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <thread>
#include <iostream>
#include <ifaddrs.h>

static std::string getJsonStringVal(const char* msg, const char* key)
{
	std::string method;
	const char* pS = msg;
	const char* pE = 0;
	std::string tmpKey;
	tmpKey = key;
	tmpKey += "\":";

	pS = strstr(msg, tmpKey.c_str());
	if (pS != 0)
	{
		pE = strstr(pS, "\",");
		if (pE != 0)
		{
			char tmp[64] = { 0 };
			memcpy(tmp, pS + strlen(key) + 3, pE - pS - (strlen(key) + 3));
			method = tmp;
		}
	}

	return method;
}

BroadcastServer::BroadcastServer() : m_socket(-1), m_running(false), m_port(0) {}

BroadcastServer::~BroadcastServer() {
    stop();
}

bool BroadcastServer::getLocalIP() {
    struct ifaddrs *ifAddrStruct = nullptr;
    void *tmpAddrPtr = nullptr;

    getifaddrs(&ifAddrStruct);
    for (struct ifaddrs *ifa = ifAddrStruct; ifa != nullptr; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) continue;
        if (ifa->ifa_addr->sa_family == AF_INET) {
            tmpAddrPtr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            if (strcmp(addressBuffer, "127.0.0.1") != 0) {
                m_local_ip = addressBuffer;
                return true;
            }
        }
    }
    if (ifAddrStruct != nullptr) freeifaddrs(ifAddrStruct);
    return false;
}

bool BroadcastServer::init(const std::string& broadcast_addr, int port) {
    m_broadcast_addr = broadcast_addr;
    m_port = port;

    if (!getLocalIP()) {
        std::cerr << "Failed to get local IP" << std::endl;
        return false;
    }

    m_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_socket < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return false;
    }

    int broadcast_enable = 1;
    if (setsockopt(m_socket, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable)) < 0) {
        std::cerr << "Failed to set broadcast option" << std::endl;
        close(m_socket);
        return false;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(m_port);

    if (bind(m_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Failed to bind socket" << std::endl;
        close(m_socket);
        return false;
    }

    return true;
}

void BroadcastServer::broadcastThread() {
    struct sockaddr_in broadcast_addr;
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_addr.s_addr = inet_addr(m_broadcast_addr.c_str());
    broadcast_addr.sin_port = htons(m_port);

    while (m_running) {
        std::string message = "{\"msg\":\"search_device\",\"msgId\":\"1734508358056\",\"type\":\"SEARCH\"}";
        sendto(m_socket, message.c_str(), message.length(), 0,
               (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));
        std::cout << "Broadcast: " << message << std::endl;
        sleep(5);
    }
}

void BroadcastServer::receiveThread() {
    char buffer[1024];
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    while (m_running) {
        int recv_len = recvfrom(m_socket, buffer, sizeof(buffer)-1, 0,
                               (struct sockaddr*)&client_addr, &addr_len);
        if (recv_len > 0) {
            buffer[recv_len] = '\0';
            std::cout << "Received from " << inet_ntoa(client_addr.sin_addr)
                      << ": " << buffer << std::endl;
            if (m_on_receive_callback) {
                m_on_receive_callback(std::string(buffer));
            }
        }
    }
}

void BroadcastServer::start() {
    m_running = true;
    // std::thread broadcast_t(&BroadcastServer::broadcastThread, this);
    std::thread receive_t(&BroadcastServer::receiveThread, this);
    // broadcast_t.detach();
    receive_t.detach();
}

void BroadcastServer::stop() {
    m_running = false;
    if (m_socket >= 0) {
        close(m_socket);
        m_socket = -1;
    }
} 

void BroadcastServer::setOnReceiveCallback(std::function<void(const std::string&)> callback) {
    m_on_receive_callback = callback;
}

void BroadcastServer::broadcast(const std::string& message) 
{
    if (m_socket < 0) {
        std::cerr << "Socket not initialized" << std::endl;
        return;
    }
    struct sockaddr_in broadcast_addr;
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_addr.s_addr = inet_addr(m_broadcast_addr.c_str());
    broadcast_addr.sin_port = htons(m_port);
    sendto(m_socket, message.c_str(), message.length(), 0,
           (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));
}