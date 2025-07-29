#ifndef _WIN32
#include <atomic>
#include <memory>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <unistd.h>

#include "ipc/msgq/msgq.h"
#include "utils/assert.h"
#include "utils/log.h"

namespace msgq {

// Global flag to indicate if the program is interrupted by a signal
static std::atomic<bool> g_interrupted { false };
static void handle_interrupt(int signal)
{
    LOG_DEBUG("Received signal %d, setting interrupted flag", signal);
    g_interrupted.store(true);
}

message_queue::message_queue(std::string name, NodeType ntype, key_t key)
    : msgq_name_(name)
    , node_type_(ntype)
    , key_(key)
{
    signal(SIGINT, handle_interrupt);
    signal(SIGQUIT, handle_interrupt);
    signal(SIGKILL, handle_interrupt);
    signal(SIGTERM, handle_interrupt);

    switch (ntype) {
    case NodeType::Receiver:
        // IPC_CREAT: create the message queue if it does not exist
        // IPC_EXCL: fail if key exists, must be used together with IPC_CREAT
        // Octal number determines the access permissions to the message queue
        // 6 (110 binary) represents read and write permissions (4 read + 2 write)
        // 0666 indicates that all users (owners, groups, and others) can read and write to this message queue
        msgid_ = msgget(key, IPC_EXCL | IPC_CREAT | 0666);
        // -1 indicates that msgq already exists
        // But only one receiver can exist for a message queue
        ASSERT_EXIT(msgid_ == -1, "Receiver of node '%s' (key: 0x%x) already exists", msgq_name_.c_str(), key);

        struct msqid_ds queue_info;
        ASSERT_EXIT(msgctl(msgid_, IPC_STAT, &queue_info) == -1, "msgctl(IPC_STAT) fail");
        max_msg_size_ = queue_info.msg_qbytes;
        LOG_DEBUG("Receiver (MessageQueue) '%s' (key: 0x%x) created with ID %d", msgq_name_.c_str(), key, msgid_);
        break;
    case NodeType::Sender:
        // Move connection establishment to send method
        // Prevent errors caused by not creating a receiver during initialization
        break;
    default:
        ASSERT_EXIT(true, "Unknown NodeType %d for node %s", static_cast<int>(ntype), msgq_name_.c_str());
        break;
    }
}

message_queue::~message_queue()
{
    message_queue::remove();
}

bool message_queue::send(const void* data, size_t data_size)
{
    if (msgid_ == -1) {
        msgid_ = msgget(key_, 0666);
        ASSERT_RETURN(msgid_ == -1, false, "Receiver of node '%s' (key: 0x%x) does not exist", msgq_name_.c_str(), key_);
        LOG_DEBUG("Sender (MessageQueue) '%s' (key: 0x%x) created with ID %d", msgq_name_.c_str(), key_, msgid_);

        // Get the maximum message size for this queue
        struct msqid_ds queue_info;
        ASSERT_RETURN(msgctl(msgid_, IPC_STAT, &queue_info) == -1, false, "msgctl(IPC_STAT) fail");
        max_msg_size_ = queue_info.msg_qbytes;
    }

    ASSERT_RETURN(!data, false, "Data is null");

    size_t total_size = sizeof(msg) + data_size;
    ASSERT_RETURN(total_size > max_msg_size_, false, "Data size %zu exceeds maximum message size %zu", data_size, max_msg_size_);

    msg* message = static_cast<msg*>(malloc(total_size));
    ASSERT_RETURN(!message, false, "malloc fail");

    message->mtype = MESSAGE_TYPE;
    message->size = data_size;
    memcpy(message->data, data, data_size);

    if (msgsnd(msgid_, message, total_size, 0) == -1) {
        // Fail reasons:
        // 1. Receiver restart makes the msgid_ invalid
        ASSERT(true, "msgsnd fail");
        free(message);
        return false;
    }

    free(message);
    return true;
}

std::shared_ptr<void> message_queue::receive()
{
    std::unique_ptr<char[]> buffer(new char[max_msg_size_]);
    ASSERT_RETURN(!buffer, nullptr, "malloc fail");

    while (!g_interrupted.load()) {
        ssize_t received = msgrcv(msgid_, buffer.get(), max_msg_size_, 0, 0);
        if (received == -1) {
            if (errno == EINTR && g_interrupted.load())
                goto Interruption;
            ASSERT_RETURN(true, nullptr, "msgrcv fail");
        }

        msg* message = reinterpret_cast<msg*>(buffer.get());
        ASSERT_RETURN(static_cast<size_t>(received) != sizeof(msg) + message->size, nullptr,
            "Received size %ld does not match expected size %zu", received, sizeof(msg) + message->size);

        std::shared_ptr<void> result(malloc(message->size), free);
        ASSERT_RETURN(!result, nullptr, "malloc fail");
        memcpy(result.get(), message->data, message->size);
        return result;
    }

Interruption:
    LOG_INFO("msgrcv interrupted by signal, exiting Receiver '%s' (key: 0x%x)", msgq_name_.c_str(), key_);
    remove();
    exit(EXIT_SUCCESS);
}

bool message_queue::remove()
{
    if (node_type_ == NodeType::Receiver) {
        LOG_DEBUG("Removing message queue '%s' (key: 0x%x) with ID %d", msgq_name_.c_str(), key_, msgid_);
        // msgctl will return -1 and set errno to EINVAL if remove repeatedly
        ASSERT_RETURN(msgctl(msgid_, IPC_RMID, nullptr) == -1 && errno != EINVAL, false, "msgctl(IPC_RMID) fail, msgid: %d", msgid_);
    }
    return true;
}

} // namespace msgq
#endif // _WIN32