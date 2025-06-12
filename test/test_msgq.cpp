#include <cstring>
#include <gtest/gtest.h>
#include <unistd.h>
#include <vector>

#include "ipc.h"
#include "msgq/msgq.h"

using namespace ipc;

void test_basic()
{
    const char* msg = "Hello, IPC!";
    size_t msg_size = strlen(msg) + 1; // +1 for null terminator

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork fail");
        exit(1);
    } else if (pid == 0) {
        ipc::node node("basic");
        std::vector<char> buffer(msg_size);
        node.receive(buffer.data(), msg_size);

        EXPECT_EQ(strcmp(buffer.data(), msg), 0) << "Received message: " << buffer.data();
    } else {
        ipc::node node("basic");
        node.send(msg, msg_size);
        wait(nullptr);
    }
}

TEST(MSGQ, basic)
{
    test_basic();
}