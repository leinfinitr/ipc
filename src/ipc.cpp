#include <functional>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ipc/ipc.h"
#include "ipc/msgq/msgq.h"
#include "ipc/pipe/pipe.h"
#include "utils/assert.h"
#include "utils/log.h"

#ifndef _WIN32
#include <sys/ipc.h> // For IPC_PRIVATE
#endif

namespace ipc {

node::node(std::string name, NodeType ntype, ChannelType ctype)
    : name_(std::move(name))
    , node_type_(ntype)
{
#ifdef _WIN32
    switch (ctype) {
    case ChannelType::MessageQueue:
        ASSERT_EXIT(true, "Message queue channel not implemented yet.");
    case ChannelType::NamedPipe:
        channel_ = std::make_shared<pipe::named_pipe>(name, ntype);
        break;
    default:
        channel_ = std::make_shared<pipe::named_pipe>(name, ntype);
    }
#else
    std::hash<std::string> hasher;
    size_t hash = hasher(name);
    key_t key = static_cast<key_t>(hash & 0xFFFFFFFF);
    ASSERT_EXIT(key == IPC_PRIVATE, "Generated key is IPC_PRIVATE, which is invalid");

    switch (ctype) {
    case ChannelType::MessageQueue:
        channel_ = std::make_shared<msgq::message_queue>(name, ntype, key);
        break;
    case ChannelType::NamedPipe:
        ASSERT_EXIT(true, "Named pipe channel not implemented yet.");
    default:
        channel_ = std::make_shared<msgq::message_queue>(name, ntype, key);
    }
#endif
}

node::~node()
{
    remove();
}

bool node::send(void const* data, size_t data_size)
{
    ASSERT_RETURN(!channel_, false, "Channel not initialized");
    ASSERT_RETURN(node_type_ != NodeType::Sender, false, "Cannot send data from a Receiver node");

    return channel_->send(data, data_size);
}

std::shared_ptr<void> node::receive()
{
    ASSERT_RETURN(!channel_, nullptr, "Channel not initialized");
    ASSERT_RETURN(node_type_ != NodeType::Receiver, nullptr, "Cannot receive data from a Sender node");

    return channel_->receive();
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