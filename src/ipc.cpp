#include <functional>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>

#include "ipc.h"
#include "msgq.h"

namespace ipc {

node::node(std::string name, ChannelType type)
    : name_(std::move(name))
{
    std::hash<std::string> hasher;
    size_t hash = hasher(name_);
    key_t key = static_cast<key_t>(hash & 0xFFFFFFFF);
    if (key == IPC_PRIVATE) {
        perror("Generated key is IPC_PRIVATE, which is invalid for message queues");
        exit(EXIT_FAILURE);
    }

    switch (type) {
    case ChannelType::MessageQueue:
        channel_ = new msgq::message_queue(key);
        break;
    case ChannelType::SharedMemory:
        fprintf(stderr, "Shared memory channel not implemented yet.\n");
        exit(EXIT_FAILURE);
    default:
        fprintf(stderr, "Unknown channel type.\n");
        exit(EXIT_FAILURE);
    }
}

node::~node()
{
    delete channel_;
}

void node::send(void const* data, std::size_t size)
{
    if (channel_) {
        channel_->send(data, size);
    } else {
        fprintf(stderr, "Channel not initialized.\n");
    }
}

void node::receive(void* data, std::size_t size)
{
    if (channel_) {
        channel_->receive(data, size);
    } else {
        fprintf(stderr, "Channel not initialized.\n");
    }
}

} // namespace msgq