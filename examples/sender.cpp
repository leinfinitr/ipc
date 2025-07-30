#include <ipc/ipc.h>
#include <string>

int main()
{
    // Example of sending a message using IPC
    std::string data = "Hello, IPC!";

    // Create an IPC node named "example"
    ipc::node sender("example", ipc::NodeType::Sender);

    // Send the data
    if (!sender.send(data.c_str(), data.size() + 1)) {
        fprintf(stderr, "Failed to send message\n");
        return 1;
    }
}