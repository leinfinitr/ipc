#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
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

    virtual bool send(const void* data, size_t data_size = 0) = 0;
    virtual std::shared_ptr<void> receive() = 0;

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

    bool send(const void* data, size_t data_size = 0);
    std::shared_ptr<void> receive();

    template <typename T>
    bool send_struct(const T& data)
    {
        return send(&data, sizeof(T));
    }

    template <typename T>
    std::shared_ptr<T> receive_struct()
    {
        auto rec = receive();
        if (!rec)
            return nullptr; // Return nullptr if no message was received
        return std::static_pointer_cast<T>(rec); // Cast the received data to the expected type
    }

private:
    std::string name_; // Name of the IPC node
    Channel* channel_; // Pointer to the underlying IPC channel
};

} // namespace ipc
