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
        perror("Generated key is IPC_PRIVATE, which is invalid");
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

bool node::send(void const* data)
{
    if (channel_) {
        return channel_->send(data);
    } else {
        fprintf(stderr, "Channel not initialized.\n");
        return false;
    }
}

std::shared_ptr<void> node::receive()
{
    if (channel_) {
        return channel_->receive();
    } else {
        fprintf(stderr, "Channel not initialized.\n");
        return nullptr;
    }
}

} // namespace msgq