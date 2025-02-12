#include "BroadcastClient.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <thread>
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

static int getJsonIntVal(const char* msg, const char* key)
{
	const char* pS = msg;
	const char* pE = 0;
	int ret = 0;
	std::string tmpKey;
	tmpKey = key;
	tmpKey += "\":";

	pS = strstr(msg, tmpKey.c_str());
	if (pS != 0)
	{
		pE = strstr(pS, ",");
		if (pE != 0)
		{
			char tmp[64] = { 0 };
			memcpy(tmp, pS + strlen(key) + 2, pE - pS - (strlen(key) + 2));
			ret = atoi(tmp);
		}
	}
	return ret;
}

BroadcastClient::BroadcastClient() : m_socket(-1), m_running(false), m_port(0) {}

BroadcastClient::~BroadcastClient() {
    stop();
}

bool BroadcastClient::getLocalIP() {
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

bool BroadcastClient::init(const std::string& broadcast_addr, int port) {
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

void BroadcastClient::receiveAndResponse() {
    char buffer[1024];
    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(server_addr);

    while (m_running) {
        int recv_len = recvfrom(m_socket, buffer, sizeof(buffer)-1, 0,
                               (struct sockaddr*)&server_addr, &addr_len);
        if (recv_len > 0) {
            buffer[recv_len] = '\0';
            std::string received_msg(buffer);
            std::cout << "Received: " << received_msg << std::endl;
            std::string msg = getJsonStringVal(received_msg.c_str(), "msg");
            std::string type = getJsonStringVal(received_msg.c_str(), "type");
            std::cout << "msg:" << msg << ", type:" << type <<std::endl;
            if (msg == "search_device" || type == "SEARCH") {
                std::string response = "{\"dev\":\"" + m_dev_name + "\", \"ipaddr\":\""+ m_local_ip + "\"}";
                server_addr.sin_port = htons(m_port);
                sendto(m_socket, response.c_str(), response.length(), 0,
                       (struct sockaddr*)&server_addr, sizeof(server_addr));
                std::cout << "Sent response: " << response << std::endl;
            }
        }
    }
}

void BroadcastClient::start() {
    m_running = true;
    std::thread receive_t(&BroadcastClient::receiveAndResponse, this);
    receive_t.detach();
}

void BroadcastClient::stop() {
    m_running = false;
    if (m_socket >= 0) {
        close(m_socket);
        m_socket = -1;
    }
}

void BroadcastClient::setRepDevName(const std::string& dev_name)
{
    m_dev_name = dev_name;
}