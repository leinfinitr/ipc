#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <unistd.h>

#include "assert.h"
#include "msgq.h"

namespace msgq {

message_queue::message_queue(key_t key)
    : Channel(ChannelType::MessageQueue)
    , msgid(-1)
{
    msgid = msgget(key, IPC_EXCL | IPC_CREAT | 0666);
    if (msgid == -1) {
        msgid = msgget(key, IPC_CREAT | 0666);
        ASSERT_EXIT(msgid == -1, "msgget fail");
        create_flag = false;
    }

    struct msqid_ds queue_info;
    ASSERT_EXIT(msgctl(msgid, IPC_STAT, &queue_info) == -1, "msgctl(IPC_STAT) fail");
    max_msg_size = queue_info.msg_qbytes;
}

message_queue::~message_queue()
{
    if (create_flag) {
        if (msgctl(msgid, IPC_RMID, nullptr) == -1) {
            perror("msgctl fail");
        }
    } else {
        // If the queue was not created by this instance, we do not remove it.
        // This is to avoid removing a queue that might be used by other processes.
    }
}

bool message_queue::send(const void* data)
{
    if (!data) {
        perror("Data is null\n");
        return false;
    }

    size_t data_size = data ? strlen(static_cast<const char*>(data)) : 0;
    size_t total_size = sizeof(msg) + data_size; // +1 for null terminator
    ASSERT_RETURN(total_size > max_msg_size, false, "Data size %zu exceeds maximum message size %zu", data_size, max_msg_size);

    msg* message = static_cast<msg*>(malloc(total_size));
    ASSERT_RETURN(!message, false, "malloc fail");

    message->size = data_size;
    memcpy(message->data, data, data_size);

    if (msgsnd(msgid, message, total_size, 0) == -1)
        perror("msgsnd fail");

    free(message);
    return true;
}

std::shared_ptr<void> message_queue::receive()
{
    std::unique_ptr<char[]> buffer(new char[max_msg_size]);
    ASSERT_RETURN(!buffer, nullptr, "malloc fail");

    ssize_t received = msgrcv(msgid, buffer.get(), max_msg_size, 0, 0);
    ASSERT_RETURN(received == -1, nullptr, "msgrcv fail");

    msg* message = reinterpret_cast<msg*>(buffer.get());
    ASSERT_RETURN(static_cast<size_t>(received) != sizeof(msg) + message->size, nullptr,
        "Received size %ld does not match expected size %zu", received, sizeof(msg) + message->size);

    std::shared_ptr<void> result(malloc(message->size), free);
    ASSERT_RETURN(!result, nullptr, "malloc fail");
    memcpy(result.get(), message->data, message->size);
    return result;
}

} // namespace msgq