#pragma once

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

    bool send(const void* data) override;
    std::shared_ptr<void> receive() override;

private:
    int msgid;
    msglen_t max_msg_size = 0;
    bool create_flag = true;

    struct msg {
        long mtype = 0;
        size_t size;
        char data[];
    };
};
} // namespace msgq
