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
        perror("fork fail");
        exit(1);
    } else if (pid != 0) {
        ipc::node node("basic");
        auto rec = node.receive();
        if (!rec) {
            fprintf(stderr, "Failed to receive message\n");
            exit(1);
        }
        std::vector<char> res(static_cast<char*>(rec.get()), static_cast<char*>(rec.get()) + strlen(static_cast<char*>(rec.get())) + 1);
        EXPECT_STREQ(res.data(), msg);
    } else {
        testing::GTEST_FLAG(output) = "";
        testing::GTEST_FLAG(print_time) = false;

        ipc::node node("basic");
        EXPECT_TRUE(node.send(msg));

        exit(0);
    }
}

void test_loop()
{
    const char* base_msg = "Hello, IPC";
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork fail");
        exit(1);
    } else if (pid != 0) {
        ipc::node node("loop");
        std::shared_ptr<void> rec = nullptr;
        for (int i = 0; i < 20; ++i) {
            rec = node.receive();
            if (!rec) {
                fprintf(stderr, "Failed to receive message\n");
                exit(1);
            }
            std::vector<char> buffer(static_cast<char*>(rec.get()), static_cast<char*>(rec.get()) + strlen(static_cast<char*>(rec.get())) + 1);
            std::string expected_msg = std::string(base_msg) + " - Message #" + std::to_string(i + 1);
            EXPECT_STREQ(buffer.data(), expected_msg.c_str());
        }
    } else {
        testing::GTEST_FLAG(output) = "";
        testing::GTEST_FLAG(print_time) = false;

        ipc::node node("loop");
        for (int i = 0; i < 20; ++i) {
            std::string full_msg = std::string(base_msg) + " - Message #" + std::to_string(i + 1);
            EXPECT_TRUE(node.send(full_msg.c_str()));
        }

        exit(0);
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
    } else if (pid != 0) {
        ipc::node node("struct");
        auto rec = node.receive();
        if (!rec) {
            fprintf(stderr, "Failed to receive message\n");
            exit(1);
        }
        auto received_msg = std::static_pointer_cast<decltype(msg)>(rec);
        EXPECT_EQ(received_msg->meta_info.id, msg.meta_info.id);
        EXPECT_STREQ(received_msg->meta_info.name, msg.meta_info.name);
        EXPECT_DOUBLE_EQ(received_msg->meta_info.value, msg.meta_info.value);
        EXPECT_EQ(received_msg->mtype, msg.mtype);
        EXPECT_STREQ(received_msg->mtext, msg.mtext);
    } else {
        testing::GTEST_FLAG(output) = "";
        testing::GTEST_FLAG(print_time) = false;

        ipc::node node("struct");
        EXPECT_TRUE(node.send(&msg));

        exit(0);
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