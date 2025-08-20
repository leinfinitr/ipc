#ifndef _WIN32
#include <iostream>
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

MessageQueue::MessageQueue(std::string name, NodeType ntype, key_t key)
    : msgq_name_(name)
    , node_type_(ntype)
    , key_(key)
{
    switch (ntype) {
    case NodeType::kReceiver:
        // IPC_CREAT: create the message queue if it does not exist
        // IPC_EXCL: fail if key exists, must be used together with IPC_CREAT
        // Octal number determines the access permissions to the message queue
        // 6 (110 binary) represents read and write permissions (4 read + 2 write)
        // 0666 indicates that all users (owners, groups, and others) can read and write to this message queue
        msgid_ = msgget(key, IPC_EXCL | IPC_CREAT | 0666);
        if (msgid_ == -1) {
            // -1 indicates that msgq already exists
            // But only one receiver can exist for a message queue
            XINFO("kReceiver of Node '%s' (key: 0x%x) already exists, do you want to delete it? (y/n)", msgq_name_.c_str(), key);
            char response;
            std::cin >> response;
            XASSERT_EXIT(response != 'y' && response != 'Y', "kReceiver already exists, exiting");

            // Delete the existing message queue
            XDEBG("Deleting existing message queue '%s' (key: 0x%x)", msgq_name_.c_str(), key);
            msgid_ = msgget(key, 0666);
            XASSERT_EXIT(msgid_ == -1, "Failed to get existing message queue, key: 0x%x", key);
            // IPC_RMID: Remove the message queue
            // If the message queue is not empty, it will be deleted after all messages are read
            // If the message queue is empty, it will be deleted immediately
            XASSERT_EXIT(msgctl(msgid_, IPC_RMID, nullptr) == -1, "Delete fail, msgid: %d", msgid_);
            msgid_ = msgget(key, IPC_EXCL | IPC_CREAT | 0666);
            XASSERT_EXIT(msgid_ == -1, "Failed to create new message queue, key: 0x%x", key);
        } else {
            // Successfully created a new message queue
            XDEBG("kReceiver (MessageQueue) '%s' (key: 0x%x) created with ID %d", msgq_name_.c_str(), key, msgid_);
        }

        struct msqid_ds queue_info;
        XASSERT_EXIT(msgctl(msgid_, IPC_STAT, &queue_info) == -1, "msgctl(IPC_STAT) fail");
        max_msg_size_ = queue_info.msg_qbytes;
        XDEBG("kReceiver (MessageQueue) '%s' (key: 0x%x) created with ID %d", msgq_name_.c_str(), key, msgid_);
        break;
    case NodeType::kSender:
        // Move connection establishment to Send method
        // Prevent errors caused by not creating a receiver during initialization
        break;
    default:
        XASSERT_EXIT(true, "Unknown NodeType %d for Node %s", static_cast<int>(ntype), msgq_name_.c_str());
        break;
    }
}

MessageQueue::~MessageQueue()
{
    MessageQueue::Remove();
}

bool MessageQueue::Send(const void* data, size_t data_size)
{
    if (msgid_ == -1) {
        msgid_ = msgget(key_, 0666);
        XASSERT_RETURN(msgid_ == -1, false, "kReceiver of Node '%s' (key: 0x%x) does not exist", msgq_name_.c_str(), key_);
        XDEBG("kSender (MessageQueue) '%s' (key: 0x%x) created with ID %d", msgq_name_.c_str(), key_, msgid_);

        // Get the maximum message size for this queue
        struct msqid_ds queue_info;
        XASSERT_RETURN(msgctl(msgid_, IPC_STAT, &queue_info) == -1, false, "msgctl(IPC_STAT) fail");
        max_msg_size_ = queue_info.msg_qbytes;
    }

    XASSERT_RETURN(!data, false, "Data is null");

    size_t total_size = sizeof(Message) + data_size;
    XASSERT_RETURN(total_size > max_msg_size_, false, "Data size %zu exceeds maximum message size %zu", data_size, max_msg_size_);

    Message* message = static_cast<Message*>(malloc(total_size));
    XASSERT_RETURN(!message, false, "malloc fail");

    message->mtype = MESSAGE_TYPE;
    message->size = data_size;
    memcpy(message->data, data, data_size);

    if (msgsnd(msgid_, message, total_size, 0) == -1) {
        // Fail reasons:
        // 1. kReceiver restart makes the msgid_ invalid
        XASSERT(true, "msgsnd fail");
        free(message);
        return false;
    }

    free(message);
    return true;
}

std::shared_ptr<Buffer> MessageQueue::Receive()
{
    std::unique_ptr<char[]> buffer(new char[max_msg_size_]);
    XASSERT_RETURN(!buffer, nullptr, "malloc fail");

    ssize_t received = msgrcv(msgid_, buffer.get(), max_msg_size_, 0, 0);
    XASSERT_RETURN(received == -1, nullptr, "msgrcv fail");

    Message* message = reinterpret_cast<Message*>(buffer.get());
    XASSERT_RETURN(static_cast<size_t>(received) != sizeof(Message) + message->size, nullptr,
        "Received size %ld does not match expected size %zu", received, sizeof(Message) + message->size);

    auto result = std::make_shared<Buffer>(malloc(message->size), message->size);
    XASSERT_RETURN(!result, nullptr, "malloc fail");
    memcpy(result->Data(), message->data, message->size);
    return result;
}

bool MessageQueue::Remove()
{
    if (node_type_ == NodeType::kReceiver) {
        XDEBG("Removing message queue '%s' (key: 0x%x) with ID %d", msgq_name_.c_str(), key_, msgid_);
        // The destructors of Node and msgq will call Remove() multiple times
        // msgctl will return -1 and set errno to EINVAL if Remove repeatedly
        XASSERT_RETURN(msgctl(msgid_, IPC_RMID, nullptr) == -1 && errno != EINVAL, false, "msgctl(IPC_RMID) fail, msgid: %d", msgid_);
    }
    return true;
}

} // namespace msgq
#endif // _WIN32