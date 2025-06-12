#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace ipc {

enum class ChannelType {
    MessageQueue,
    SharedMemory
};

class Channel {
public:
    explicit Channel(ChannelType type)
        : type_(type)
    {
    }

    virtual ~Channel() = default;

    // Disable copy constructor and assignment operator
    Channel(const Channel&) = delete;
    Channel& operator=(const Channel&) = delete;

    ChannelType getType() const { return type_; }

    virtual void send(const void* data, size_t size) = 0;
    virtual void receive(void* buffer, size_t size) = 0;

protected:
    ChannelType type_;
};

class node {
public:
    node(std::string name, ChannelType type = ChannelType::MessageQueue);
    ~node();

    // Disable copy constructor and assignment operator
    node(const node&) = delete;
    node& operator=(const node&) = delete;

    const std::string& getName() const { return name_; }

    void send(void const* data, std::size_t size);
    void receive(void* data, std::size_t size);

private:
    std::string name_; // Name of the IPC node
    Channel* channel_; // Pointer to the underlying IPC channel
};

} // namespace ipc
