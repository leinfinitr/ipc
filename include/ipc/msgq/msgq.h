#pragma once

#ifndef _WIN32
#include <cstddef>
#include <cstdint>
#include <sys/msg.h>

#include "ipc/ipc.h"

using namespace ipc;

namespace msgq {

class MessageQueue : public Channel {
public:
    MessageQueue(std::string name, NodeType ntype, key_t key);
    ~MessageQueue();

    bool Send(const void* data, size_t data_size = 0) override;
    std::shared_ptr<Buffer> Receive() override;
    bool Remove() override;

private:
    const std::string msgq_name_;
    const NodeType node_type_;
    const key_t key_;

    int msgid_ = -1;
    msglen_t max_msg_size_ = 0;

    static constexpr long MESSAGE_TYPE = 1;
    struct Message {
        long mtype; // Message type, required by System V communication standards
        size_t size;
        char data[];
    };
};
} // namespace msgq
#endif // _WIN32
