#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

#include "msgq.h"

namespace msgq {

message_queue::message_queue(key_t key)
{
    msgid = msgget(key, IPC_CREAT);
    if (msgid == -1) {
        perror("msgget fail");
        exit(1);
    }
}

message_queue::~message_queue()
{
    if (msgctl(msgid, IPC_RMID, nullptr) == -1) {
        perror("msgctl fail");
    }
}

void message_queue::send(const void* data, std::size_t size)
{
    if (msgsnd(msgid, data, size, 0) == -1) {
        perror("msgsnd fail");
    }
}

void message_queue::receive(void* data, std::size_t size)
{
    if (msgrcv(msgid, data, size, 0, 0) == -1) {
        perror("msgrcv fail");
    }
}

} // namespace msgq