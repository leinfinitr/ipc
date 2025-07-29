#ifndef _WIN32
#include <memory>
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

message_queue::message_queue(std::string name, NodeType ntype, key_t key)
    : msgq_name_(std::move(name))
    , node_type_(ntype)
{
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
        ASSERT_EXIT(msgid_ == -1, "Receiver of node %s already exists", msgq_name_.c_str());
        break;
    case NodeType::Sender:
        msgid_ = msgget(key, 0666);
        ASSERT_EXIT(msgid_ == -1, "Receiver of node %s does not exist", msgq_name_.c_str());
        break;
    default:
        ASSERT_EXIT(true, "Unknown NodeType %d for node %s", static_cast<int>(ntype), msgq_name_.c_str());
        break;
    }

    // Get the maximum message size for this queue
    struct msqid_ds queue_info;
    ASSERT_EXIT(msgctl(msgid_, IPC_STAT, &queue_info) == -1, "msgctl(IPC_STAT) fail");
    max_msg_size_ = queue_info.msg_qbytes;
}

message_queue::~message_queue()
{
    message_queue::remove();
}

bool message_queue::send(const void* data, size_t data_size)
{
    ASSERT_RETURN(!data, false, "Data is null");

    size_t total_size = sizeof(msg) + data_size;
    ASSERT_RETURN(total_size > max_msg_size_, false, "Data size %zu exceeds maximum message size %zu", data_size, max_msg_size_);

    msg* message = static_cast<msg*>(malloc(total_size));
    ASSERT_RETURN(!message, false, "malloc fail");

    message->mtype = MESSAGE_TYPE;
    message->size = data_size;
    memcpy(message->data, data, data_size);

    if (msgsnd(msgid_, message, total_size, 0) == -1) {
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

    ssize_t received = msgrcv(msgid_, buffer.get(), max_msg_size_, 0, 0);
    if (received == -1) {
        switch (errno) {
        case EINTR:
            // Interrupted by a signal
            LOG_INFO("msgrcv interrupted by signal\n");
            return nullptr;
        default:
            ASSERT_RETURN(true, nullptr, "msgrcv fail");
        }
    }

    msg* message = reinterpret_cast<msg*>(buffer.get());
    ASSERT_RETURN(static_cast<size_t>(received) != sizeof(msg) + message->size, nullptr,
        "Received size %ld does not match expected size %zu", received, sizeof(msg) + message->size);

    std::shared_ptr<void> result(malloc(message->size), free);
    ASSERT_RETURN(!result, nullptr, "malloc fail");
    memcpy(result.get(), message->data, message->size);
    return result;
}

bool message_queue::remove()
{
    if (node_type_ == NodeType::Receiver) {
        ASSERT_RETURN(msgctl(msgid_, IPC_RMID, nullptr) == -1, false, "msgctl(IPC_RMID) fail");
        msgid_ = -1; // Reset msgid_ to indicate the queue has been removed
    }
    return true;
}

} // namespace msgq
#endif // _WIN32