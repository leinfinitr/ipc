#pragma once

#include <cstddef>
#include <cstdint>

#include "ipc.h"

using namespace ipc;

namespace msgq {

class message_queue : public Channel {
public:
    message_queue(key_t key);
    ~message_queue();

    void send(const void* data, std::size_t size) override;
    void receive(void* data, std::size_t size) override;

private:
    int msgid;
    bool create_flag = true;
};
} // namespace msgq
