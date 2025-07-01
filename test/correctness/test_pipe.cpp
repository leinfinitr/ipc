#ifdef _WIN32
#include <cstring>
#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <windows.h>

#include "ipc/ipc.h"

using namespace ipc;

void test_basic()
{
    const char* msg = "Hello, IPC!";

    std::thread server_thread([msg]() {
        ipc::node server_node("basic", ipc::LinkType::Receiver, ipc::ChannelType::NamedPipe);
        auto rec = server_node.receive();
        if (!rec) {
            fprintf(stderr, "Server failed to receive message\n");
            exit(1);
        }
        const char* res = static_cast<const char*>(rec.get());
        EXPECT_STREQ(res, msg);
    });

    // Ensure that the server is started and waiting for connection
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ipc::node client_node("basic", ipc::LinkType::Sender, ipc::ChannelType::NamedPipe);
    // +1 for null terminator
    // In some versions of compilers and dynamic library dependencies, errors may occur if there is no "+ 1" for strlen
    // But sometimes even without "+ 1", there won't be any errors
    EXPECT_TRUE(client_node.send(msg, strlen(msg) + 1));

    server_thread.join();
}

void test_loop()
{
    const char* msg = "Hello, IPC";

    std::thread server_thread([msg]() {
        ipc::node server_node("loop", ipc::LinkType::Receiver, ipc::ChannelType::NamedPipe);
        for (int i = 0; i < 100; ++i) {
            auto rec = server_node.receive();
            if (!rec) {
                fprintf(stderr, "Failed to receive message\n");
                exit(1);
            }
            const char* res = static_cast<const char*>(rec.get());
            std::string expected_msg = std::string(msg) + " - Message #" + std::to_string(i + 1);
            EXPECT_STREQ(res, expected_msg.c_str());
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ipc::node client_node("loop", ipc::LinkType::Sender, ipc::ChannelType::NamedPipe);
    for (int i = 0; i < 100; ++i) {
        std::string full_msg = std::string(msg) + " - Message #" + std::to_string(i + 1);
        EXPECT_TRUE(client_node.send(full_msg.c_str(), full_msg.size() + 1));
    }

    server_thread.join();
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
    strcpy_s(msg.meta_info.name, "Test Message");
    msg.meta_info.value = 42.0;
    msg.num = 1;
    strcpy_s(msg.mtext, "Hello, IPC with struct!");

    std::thread server_thread([msg]() {
        ipc::node server_node("struct", ipc::LinkType::Receiver, ipc::ChannelType::NamedPipe);
        auto rec = server_node.receive();
        if (!rec) {
            fprintf(stderr, "Server failed to receive message\n");
            exit(1);
        }
        auto received_msg = static_cast<message*>(rec.get());
        EXPECT_EQ(received_msg->meta_info.id, msg.meta_info.id);
        EXPECT_STREQ(received_msg->meta_info.name, msg.meta_info.name);
        EXPECT_DOUBLE_EQ(received_msg->meta_info.value, msg.meta_info.value);
        EXPECT_EQ(received_msg->num, msg.num);
        EXPECT_STREQ(received_msg->mtext, msg.mtext);
    });

    // Ensure that the server is started and waiting for connection
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ipc::node client_node("struct", ipc::LinkType::Sender, ipc::ChannelType::NamedPipe);
    EXPECT_TRUE(client_node.send(&msg, sizeof(msg)));

    server_thread.join();
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
            strncpy_s(name, n, sizeof(name) - 1);
        }
    };

    MyClass obj(1, "Test Class", 3.14);

    std::thread server_thread([obj]() {
        ipc::node server_node("class", ipc::LinkType::Receiver, ipc::ChannelType::NamedPipe);
        auto rec = server_node.receive();
        if (!rec) {
            fprintf(stderr, "Server failed to receive message\n");
            exit(1);
        }
        auto received_obj = static_cast<MyClass*>(rec.get());
        EXPECT_EQ(received_obj->id, obj.id);
        EXPECT_STREQ(received_obj->name, obj.name);
        EXPECT_DOUBLE_EQ(received_obj->value, obj.value);
    });

    // Ensure that the server is started and waiting for connection
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ipc::node client_node("class", ipc::LinkType::Sender, ipc::ChannelType::NamedPipe);
    EXPECT_TRUE(client_node.send(&obj, sizeof(MyClass)));

    server_thread.join();
}

void test_subclass()
{
    class Base {
    public:
        int id;
        char name[50];

        Base(int i, const char* n)
            : id(i)
        {
            strncpy_s(name, n, sizeof(name) - 1);
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

    std::thread server_thread([obj]() {
        ipc::node server_node("subclass", ipc::LinkType::Receiver, ipc::ChannelType::NamedPipe);
        auto rec = server_node.receive();
        if (!rec) {
            fprintf(stderr, "Server failed to receive message\n");
            exit(1);
        }
        auto received_obj = static_cast<Derived*>(rec.get());
        EXPECT_EQ(received_obj->id, obj.id);
        EXPECT_STREQ(received_obj->name, obj.name);
        EXPECT_DOUBLE_EQ(received_obj->value, obj.value);
    });

    // Ensure that the server is started and waiting for connection
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ipc::node client_node("subclass", ipc::LinkType::Sender, ipc::ChannelType::NamedPipe);
    EXPECT_TRUE(client_node.send(&obj, sizeof(Derived)));

    server_thread.join();
}

void test_multiterminal()
{
    const char* msg = "Hello, IPC";

    std::thread client_thread_1([msg]() {
        ipc::node client_node_1("multiterminal", ipc::LinkType::Sender, ipc::ChannelType::NamedPipe);
        std::string full_msg = std::string(msg) + " - Message #1";

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        EXPECT_TRUE(client_node_1.send(full_msg.c_str(), full_msg.size() + 1));
    });

    std::thread client_thread_2([msg]() {
        ipc::node client_node_2("multiterminal", ipc::LinkType::Sender, ipc::ChannelType::NamedPipe);
        std::string full_msg = std::string(msg) + " - Message #2";

        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        EXPECT_TRUE(client_node_2.send(full_msg.c_str(), full_msg.size() + 1));
    });

    std::thread client_thread_3([msg]() {
        ipc::node client_node_3("multiterminal", ipc::LinkType::Sender, ipc::ChannelType::NamedPipe);
        std::string full_msg = std::string(msg) + " - Message #3";

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        EXPECT_TRUE(client_node_3.send(full_msg.c_str(), full_msg.size() + 1));
    });

    ipc::node server_node("multiterminal", ipc::LinkType::Receiver, ipc::ChannelType::NamedPipe);
    for (int i = 0; i < 3; ++i) {
        auto rec = server_node.receive();
        if (!rec) {
            fprintf(stderr, "Failed to receive message\n");
            exit(1);
        }
        const char* res = static_cast<const char*>(rec.get());
        std::string expected_msg = std::string(msg) + " - Message #" + std::to_string(i + 1);
        EXPECT_STREQ(res, expected_msg.c_str());
    }

    client_thread_1.join();
    client_thread_2.join();
    client_thread_3.join();
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

TEST(MSGQ, class)
{
    test_class();
    test_subclass();
}

TEST(MSGQ, multiterminal)
{
    test_multiterminal();
}
#endif