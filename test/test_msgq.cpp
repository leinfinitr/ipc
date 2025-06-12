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

    pid_t pid = fork();
    if (pid == -1) {
        ipc::node node("basic");
        node.send(msg, strlen(msg) + 1);
    } else if (pid == 0) {
        ipc::node node("basic");
        std::vector<char> buffer(strlen(msg));
        node.receive(buffer.data(), strlen(msg));

        EXPECT_EQ(strcmp(buffer.data(), msg), 0) << "Received message does not match sent message";
    }
}

TEST(IPC, Basic)
{
    test_basic();
}

int main(int argc, char** argv)
{
    printf("Running Google Test from %s\n", __FILE__);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}