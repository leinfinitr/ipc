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
        waitpid(0, nullptr, 0);
    }
}

void test_loop()
{
    const char* base_msg = "Hello, IPC";
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork fail");
        exit(1);
    } else if (pid == 0) {
        ipc::node node("loop");
        std::vector<char> buffer(256);

        for (int i = 0; i < 10; ++i) {
            node.receive(buffer.data(), buffer.size());
            std::string expected_msg = std::string(base_msg) + " - Message #" + std::to_string(i + 1);
            EXPECT_EQ(strcmp(buffer.data(), expected_msg.c_str()), 0) << "Received message: " << buffer.data();
        }
    } else {
        ipc::node node("loop");
        for (int i = 0; i < 10; ++i) {
            std::string full_msg = std::string(base_msg) + " - Message #" + std::to_string(i + 1);
            node.send(full_msg.c_str(), full_msg.size() + 1);
        }
        waitpid(0, nullptr, 0);
    }
}

void test_struct()
{
    struct meta {
        int id;
        char name[50];
        double value;
    };
    struct {
        meta meta_info;
        long mtype;
        char mtext[256];
    } msg;

    msg.meta_info.id = 1;
    strcpy(msg.meta_info.name, "Test Message");
    msg.meta_info.value = 42.0;
    msg.mtype = 1;
    strcpy(msg.mtext, "Hello, IPC with struct!");

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork fail");
        exit(1);
    } else if (pid == 0) {
        ipc::node node("struct");
        std::vector<char> buffer(sizeof(msg));
        node.receive(buffer.data(), sizeof(msg));

        auto* received_msg = reinterpret_cast<decltype(&msg)>(buffer.data());
        EXPECT_EQ(received_msg->meta_info.id, msg.meta_info.id);
        EXPECT_STREQ(received_msg->meta_info.name, msg.meta_info.name);
        EXPECT_DOUBLE_EQ(received_msg->meta_info.value, msg.meta_info.value);
        EXPECT_EQ(received_msg->mtype, msg.mtype);
        EXPECT_STREQ(received_msg->mtext, msg.mtext);
    } else {
        ipc::node node("struct");
        node.send(&msg, sizeof(msg));
        waitpid(0, nullptr, 0);
    }
}

TEST(MSGQ, basic)
{
    test_basic();
}

TEST(MSGQ, loop)
{
    test_loop();
}

TEST(MSGQ, struct)
{
    test_struct();
}