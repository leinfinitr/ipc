#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include "ipc/assert.h"
#include "ipc/pipe/pipe.h"

namespace pipe {

named_pipe::named_pipe(const std::string& name)
    : Channel(ChannelType::NamedPipe)
    , pipe_(INVALID_HANDLE_VALUE)
    , pipe_name_(R"(\\.\pipe\)" + name)
{
    pipe_ = CreateNamedPipeA(
        pipe_name_.c_str(),
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        1,
        BUFFER_SIZE,
        BUFFER_SIZE,
        0,
        NULL);

    if (pipe_ == INVALID_HANDLE_VALUE) {
        pipe_ = CreateFileA(
            pipe_name_.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            0,
            NULL);

        ASSERT_EXIT(pipe_ == INVALID_HANDLE_VALUE, "Failed to open named pipe");
    }
}

named_pipe::~named_pipe()
{
    named_pipe::remove();
}

bool named_pipe::send(const void* data, size_t data_size)
{
    if (!data) {
        fprintf(stderr, "Data is null\n");
        return false;
    }

    DWORD bytesWritten;
    BOOL success = WriteFile(
        pipe_,
        data,
        static_cast<DWORD>(data_size),
        &bytesWritten,
        NULL);

    if (!success || bytesWritten != data_size) {
        fprintf(stderr, "WriteFile failed, error: %lu\n", GetLastError());
        return false;
    }

    return true;
}

std::shared_ptr<void> named_pipe::receive()
{
    char buffer[BUFFER_SIZE];
    DWORD bytesRead;

    BOOL success = ReadFile(
        pipe_,
        buffer,
        BUFFER_SIZE,
        &bytesRead,
        NULL);

    if (!success) {
        DWORD err = GetLastError();
        if (err == ERROR_BROKEN_PIPE) {
            printf("[ipc/msgq] Pipe disconnected\n");
        } else {
            fprintf(stderr, "ReadFile failed, error: %lu\n", err);
        }
        return nullptr;
    }

    std::shared_ptr<void> result(malloc(bytesRead), free);
    if (!result) {
        fprintf(stderr, "malloc fail\n");
        return nullptr;
    }

    memcpy(result.get(), buffer, bytesRead);
    return result;
}

bool named_pipe::remove()
{
    if (pipe_ != INVALID_HANDLE_VALUE) {
        CloseHandle(pipe_);
        pipe_ = INVALID_HANDLE_VALUE;
    }

    return true;
}

} // namespace msgq