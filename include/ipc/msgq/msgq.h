#pragma once

#ifndef _WIN32
#include <cstddef>
#include <cstdint>
#include <sys/msg.h>

#include "ipc/ipc.h"

using namespace ipc;

namespace msgq {

class message_queue : public Channel {
public:
    message_queue(key_t key);
    ~message_queue();

    bool send(const void* data, size_t data_size = 0) override;
    std::shared_ptr<void> receive() override;
    bool remove() override;

private:
    static constexpr long MESSAGE_TYPE = 1;
    int msgid;
    msglen_t max_msg_size = 0;
    bool create_flag = true;

    struct msg {
        long mtype; // Message type, required by System V communication standards
        size_t size;
        char data[];
    };
};
} // namespace msgq
#endif // _WIN32
