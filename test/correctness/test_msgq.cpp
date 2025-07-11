#ifndef _WIN32
#include <cstring>
#include <gtest/gtest.h>
#include <unistd.h>
#include <vector>

#include "ipc/ipc.h"

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
        const char* res = static_cast<const char*>(rec.get());
        EXPECT_STREQ(res, msg);
    } else {
        testing::GTEST_FLAG(output) = "";
        testing::GTEST_FLAG(print_time) = false;

        ipc::node node("basic");
        EXPECT_TRUE(node.send(msg, strlen(msg)));

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
            const char* res = static_cast<const char*>(rec.get());
            std::string expected_msg = std::string(base_msg) + " - Message #" + std::to_string(i + 1);
            EXPECT_STREQ(res, expected_msg.c_str());
        }
    } else {
        testing::GTEST_FLAG(output) = "";
        testing::GTEST_FLAG(print_time) = false;

        ipc::node node("loop");
        for (int i = 0; i < 20; ++i) {
            std::string full_msg = std::string(base_msg) + " - Message #" + std::to_string(i + 1);
            EXPECT_TRUE(node.send(full_msg.c_str(), full_msg.size() + 1)); // +1 for null terminator
        }

        exit(0);
    }
}

void test_struct_no_fork()
{
    struct meta {
        int id;
        char name[50];
        double value;
    };
    struct message {
        meta meta_info;
        long num;
        char mtext[256];
    };

    message msg;
    msg.meta_info.id = 1;
    strcpy(msg.meta_info.name, "Test Message");
    msg.meta_info.value = 42.0;
    msg.num = 1;
    strcpy(msg.mtext, "Hello, IPC with struct!");

    ipc::node node("struct_no_fork");
    EXPECT_TRUE(node.send(&msg, sizeof(msg)));

    auto rec = node.receive();
    if (!rec) {
        fprintf(stderr, "Failed to receive message\n");
        exit(1);
    }
    auto received_msg = static_cast<message*>(rec.get());
    EXPECT_EQ(received_msg->meta_info.id, msg.meta_info.id);
    EXPECT_STREQ(received_msg->meta_info.name, msg.meta_info.name);
    EXPECT_DOUBLE_EQ(received_msg->meta_info.value, msg.meta_info.value);
    EXPECT_EQ(received_msg->num, msg.num);
    EXPECT_STREQ(received_msg->mtext, msg.mtext);
}

void test_struct()
{
    struct meta {
        int id;
        char name[50];
        double value;
    };
    struct message {
        meta meta_info;
        long num;
        char mtext[256];
    };

    message msg;
    msg.meta_info.id = 1;
    strcpy(msg.meta_info.name, "Test Message");
    msg.meta_info.value = 42.0;
    msg.num = 1;
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
        auto received_msg = static_cast<message*>(rec.get());
        EXPECT_EQ(received_msg->meta_info.id, msg.meta_info.id);
        EXPECT_STREQ(received_msg->meta_info.name, msg.meta_info.name);
        EXPECT_DOUBLE_EQ(received_msg->meta_info.value, msg.meta_info.value);
        EXPECT_EQ(received_msg->num, msg.num);
        EXPECT_STREQ(received_msg->mtext, msg.mtext);
    } else {
        testing::GTEST_FLAG(output) = "";
        testing::GTEST_FLAG(print_time) = false;

        ipc::node node("struct");
        EXPECT_TRUE(node.send(&msg, sizeof(msg)));

        exit(0);
    }
}

void test_class_no_fork()
{
    class MyClass {
    public:
        int id;
        char name[50];
        double value;

        MyClass(int i, const char* n, double v)
            : id(i)
            , value(v)
        {
            strncpy(name, n, sizeof(name) - 1);
        }
    };

    MyClass obj(1, "Test Class", 3.14);

    ipc::node node("class_no_fork");
    EXPECT_TRUE(node.send(&obj, sizeof(MyClass)));

    auto rec = node.receive();
    if (!rec) {
        fprintf(stderr, "Failed to receive message\n");
        exit(1);
    }
    auto received_obj = static_cast<MyClass*>(rec.get());
    EXPECT_EQ(received_obj->id, obj.id);
    EXPECT_STREQ(received_obj->name, obj.name);
    EXPECT_DOUBLE_EQ(received_obj->value, obj.value);
}

void test_class()
{
    class MyClass {
    public:
        int id;
        char name[50];
        double value;

        MyClass(int i, const char* n, double v)
            : id(i)
            , value(v)
        {
            strncpy(name, n, sizeof(name) - 1);
        }
    };

    MyClass obj(1, "Test Class", 3.14);

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork fail");
        exit(1);
    } else if (pid != 0) {
        ipc::node node("class");
        auto rec = node.receive();
        if (!rec) {
            fprintf(stderr, "Failed to receive message\n");
            exit(1);
        }
        auto received_obj = static_cast<MyClass*>(rec.get());
        EXPECT_EQ(received_obj->id, obj.id);
        EXPECT_STREQ(received_obj->name, obj.name);
        EXPECT_DOUBLE_EQ(received_obj->value, obj.value);
    } else {
        testing::GTEST_FLAG(output) = "";
        testing::GTEST_FLAG(print_time) = false;

        ipc::node node("class");
        EXPECT_TRUE(node.send(&obj, sizeof(MyClass)));

        exit(0);
    }
}

void test_subclass_no_fork()
{
    class Base {
    public:
        int id;
        char name[50];

        Base(int i, const char* n)
            : id(i)
        {
            strncpy(name, n, sizeof(name) - 1);
        }
    };

    class Derived : public Base {
    public:
        double value;

        Derived(int i, const char* n, double v)
            : Base(i, n)
            , value(v)
        {
        }
    };

    Derived obj(1, "Test Subclass", 2.718);

    ipc::node node("subclass_no_fork");
    EXPECT_TRUE(node.send(&obj, sizeof(Derived)));

    auto rec = node.receive();
    if (!rec) {
        fprintf(stderr, "Failed to receive message\n");
        exit(1);
    }
    auto received_obj = static_cast<Derived*>(rec.get());
    EXPECT_EQ(received_obj->id, obj.id);
    EXPECT_STREQ(received_obj->name, obj.name);
    EXPECT_DOUBLE_EQ(received_obj->value, obj.value);
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
    test_struct_no_fork();
    test_struct();
}

TEST(MSGQ, class)
{
    test_class_no_fork();
    test_class();
    test_subclass_no_fork();
}
#endif