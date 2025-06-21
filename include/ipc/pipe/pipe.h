#pragma once

#include <windows.h>

#include "ipc/ipc.h"

using namespace ipc;

namespace pipe {

class named_pipe : public Channel {
public:
    named_pipe(const std::string& name);
    ~named_pipe();

    bool send(const void* data, size_t data_size);
    std::shared_ptr<void> receive();
    bool remove();

private:
    HANDLE pipe_;
    std::string pipe_name_;
    static const DWORD BUFFER_SIZE = 4096; // Default buffer size for named pipe communication
};
} // namespace pipe
