#include "BroadcastServer.h"
#include <iostream>
#include <string>

int main() {
    BroadcastServer server;
    
    if (!server.init("192.168.3.255", 9000)) {
        std::cerr << "Server initialization failed" << std::endl;
        return 1;
    }

    server.setOnReceiveCallback([](const std::string& message) {
        std::cout << "Received message: " << message << std::endl;
    });
    server.start();
    std::cout << "Server started. Press 'q' to quit." << std::endl;

    char input;
    while (std::cin >> input) {
        if (input == 'q') break;
        server.broadcast("{\"msg\":\"search_device\",\"msgId\":\"1734508358056\",\"type\":\"SEARCH\"}");
    }

    server.stop();
    return 0;
} 