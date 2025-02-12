#pragma once
#include <string>
#include <functional>

class BroadcastServer {
public:
    BroadcastServer();
    ~BroadcastServer();

    bool init(const std::string& broadcast_addr, int port);
    void start();
    void stop();
    void broadcast(const std::string& message);
    void setOnReceiveCallback(std::function<void(const std::string&)> callback);
private:
    int m_socket;
    bool m_running;
    std::string m_broadcast_addr;
    int m_port;
    std::string m_local_ip;
    std::function<void(const std::string&)> m_on_receive_callback;

    bool getLocalIP();
    void broadcastThread();
    void receiveThread();
}; 