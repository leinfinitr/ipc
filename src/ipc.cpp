#include <functional>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>

#include "ipc/assert.h"
#include "ipc/ipc.h"
#include "ipc/msgq/msgq.h"

namespace ipc {

node::node(std::string name, ChannelType type)
    : name_(std::move(name))
{
    std::hash<std::string> hasher;
    size_t hash = hasher(name_);
    key_t key = static_cast<key_t>(hash & 0xFFFFFFFF);
    ASSERT_EXIT(key == IPC_PRIVATE, "Generated key is IPC_PRIVATE, which is invalid");

    switch (type) {
    case ChannelType::MessageQueue:
        channel_ = std::make_shared<msgq::message_queue>(key);
        break;
    case ChannelType::SharedMemory:
        ASSERT_EXIT(true, "Shared memory channel not implemented yet.");
    default:
        ASSERT_EXIT(true, "Unknown channel type");
    }
}

node::~node()
{
    remove();
}

bool node::send(void const* data, size_t data_size)
{
    if (channel_) {
        return channel_->send(data, data_size);
    } else
        ASSERT_RETURN(true, false, "Channel not initialized");
}

std::shared_ptr<void> node::receive()
{
    if (channel_) {
        return channel_->receive();
    } else {
        ASSERT_RETURN(true, nullptr, "Channel not initialized");
    }
}

bool node::remove()
{
    if (channel_) {
        bool result = channel_->remove();
        channel_.reset(); // Reset the shared pointer to release the channel
        return result;
    }
    return true; // No channel to disconnect
}

} // namespace ipc