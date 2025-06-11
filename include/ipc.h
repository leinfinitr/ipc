#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace ipc {

class channel {
public:
    channel(std::string name);
    virtual ~channel() = default;

    // Virtual methods for sending and receiving messages
    virtual void send(void const* data, std::size_t size) = 0;
    virtual void receive(void* data, std::size_t size) = 0;
};

} // namespace ipc
