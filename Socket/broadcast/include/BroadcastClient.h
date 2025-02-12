#pragma once
#include <string>

class BroadcastClient {
public:
    BroadcastClient();
    ~BroadcastClient();

    bool init(const std::string& broadcast_addr, int port);
    void start();
    void stop();
    void setRepDevName(const std::string& dev_name);
private:
    int m_socket;
    bool m_running;
    std::string m_broadcast_addr;
    int m_port;
    std::string m_local_ip;
    std::string m_dev_name;

    bool getLocalIP();
    void receiveAndResponse();
}; 