#include <functional>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ipc/assert.h"
#include "ipc/ipc.h"

#ifdef _WIN32
#include "ipc/pipe/pipe.h"
#else
#include "ipc/msgq/msgq.h"
#include <sys/ipc.h>
#endif

namespace ipc {

node::node(std::string name, LinkType ltype, ChannelType ctype)
    : name_(std::move(name))
{
#ifdef _WIN32
    switch (ctype) {
    case ChannelType::MessageQueue:
        ASSERT_EXIT(true, "Message queue channel not implemented yet.");
    case ChannelType::NamedPipe:
        channel_ = std::make_shared<pipe::named_pipe>(name_, ltype);
        break;
    default:
        ASSERT_EXIT(true, "Unknown channel type");
    }
#else
    std::hash<std::string> hasher;
    size_t hash = hasher(name_);
    key_t key = static_cast<key_t>(hash & 0xFFFFFFFF);
    ASSERT_EXIT(key == IPC_PRIVATE, "Generated key is IPC_PRIVATE, which is invalid");

    switch (ctype) {
    case ChannelType::MessageQueue:
        channel_ = std::make_shared<msgq::message_queue>(key);
        break;
    case ChannelType::SharedMemory:
        ASSERT_EXIT(true, "Shared memory channel not implemented yet.");
    default:
        ASSERT_EXIT(true, "Unknown channel type");
    }
#endif
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