#include <ipc/ipc.h>

int main()
{
    // Create an IPC node named "example"
    ipc::node receiver("example");

    // receive the data
    auto rec = receiver.receive();
    if (!rec) {
        fprintf(stderr, "Failed to receive message\n");
        return 1;
    }
    const char* data = static_cast<const char*>(rec.get());
    // Print the received data
    printf("Received message: %s\n", data);
    return 0;
}