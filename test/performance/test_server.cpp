#include <chrono>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>

#include "ipc/ipc.h"

int main()
{
    // Create separate channels for receiving and sending
#ifdef _WIN32
    ipc::node receiver("ipc-latency-request", ipc::NodeType::Receiver, ipc::ChannelType::NamedPipe);
    ipc::node sender("ipc-latency-response", ipc::NodeType::Sender, ipc::ChannelType::NamedPipe);
#else
    ipc::node receiver("ipc-latency-request");
    ipc::node sender("ipc-latency-response");
#endif

    std::cout << "IPC echo server is running" << std::endl;
    std::cout << "Receiving on channel: ipc-latency-request" << std::endl;
    std::cout << "Sending on channel: ipc-latency-response" << std::endl;

    while (true) {
        // Wait for incoming message
        auto request = receiver.receive();

        if (!request) {
            std::cout << "Error receiving message" << std::endl;
            continue;
        }
        // Process the received message
        char* msg = static_cast<char*>(request.get());
        // Echo the message back with the same content
        sender.send(msg, strlen(msg) + 1);
    }

    return 0;
}
