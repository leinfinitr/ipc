#include <chrono>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>

#include "ipc/ipc.h"

int main()
{
    // Create separate channels for receiving and sending
    ipc::node receiver("ipc-latency-request");
    ipc::node sender("ipc-latency-response");

    std::cout << "IPC echo server is running" << std::endl;
    std::cout << "Receiving on channel: ipc-latency-request" << std::endl;
    std::cout << "Sending on channel: ipc-latency-response" << std::endl;

    char data[64];

    while (true) {
        // Wait for incoming message
        auto request = receiver.receive();

        if (!request) {
            std::cerr << "Error receiving message" << std::endl;
            continue;
        }
        // Process the received message
        char* msg = static_cast<char*>(request.get());
        memcpy(data, msg, strlen(msg));
        // Echo the message back with the same content
        sender.send(data, strlen(msg));
    }

    return 0;
}
