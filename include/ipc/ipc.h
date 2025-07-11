#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

namespace ipc {

enum class LinkType {
    Unknown,
    Sender,
    Receiver
};

enum class ChannelType {
    MessageQueue,
    NamedPipe
};

class Channel {
public:
    Channel() = default;
    virtual ~Channel() = default;

    // Disable copy constructor and assignment operator
    Channel(const Channel&) = delete;
    Channel& operator=(const Channel&) = delete;

    virtual bool send(const void* data, size_t data_size) = 0;
    virtual std::shared_ptr<void> receive() = 0;
    virtual bool remove() = 0;
};

class node {
public:
    #ifdef _WIN32
    node(std::string name, LinkType ltype = LinkType::Unknown, ChannelType ctype = ChannelType::NamedPipe);
    #else
    node(std::string name, LinkType ltype = LinkType::Unknown, ChannelType ctype = ChannelType::MessageQueue);
    #endif
    node(std::string name, ChannelType ctype)
        : node(name, LinkType::Unknown, ctype)
    {
    }
    ~node();

    // Disable copy constructor and assignment operator
    node(const node&) = delete;
    node& operator=(const node&) = delete;

    const std::string& getName() const { return name_; }

    bool send(const void* data, size_t data_size);
    std::shared_ptr<void> receive();
    bool remove();

private:
    std::string name_; // Name of the IPC node
    std::shared_ptr<Channel> channel_; // Pointer to the underlying IPC channel
};

} // namespace ipc
