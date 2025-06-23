#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include "ipc/assert.h"
#include "ipc/pipe/pipe.h"

namespace pipe {

named_pipe::named_pipe(const std::string& name, LinkType ltype)
    : pipe_(INVALID_HANDLE_VALUE)
    , link_type_(ltype)
    , pipe_name_(R"(\\.\pipe\)" + name)
{
    switch (ltype) {
    case LinkType::Unknown:
        ASSERT_RETURN(true, , "Link type must be specified in named_pipe constructor");
        break;
    case LinkType::Sender:
        ASSERT_RETURN(!WaitNamedPipeA(pipe_name_.c_str(), NMPWAIT_USE_DEFAULT_WAIT), , "No Read Pipe Accessible");
        pipe_ = CreateFileA(
            pipe_name_.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            0,
            NULL);

        ASSERT_RETURN(pipe_ == INVALID_HANDLE_VALUE, , "CreateFileA failed, erro = %lu\n", GetLastError());
        break;
    case LinkType::Receiver:
        pipe_ = CreateNamedPipeA(
            pipe_name_.c_str(),
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            1, // Maximum instances
            BUFFER_SIZE, // Output buffer size
            BUFFER_SIZE, // Input buffer size
            0, // Default timeout
            NULL);

        ASSERT_RETURN(pipe_ == INVALID_HANDLE_VALUE, , "CreateNamedPipeA failed, error = %lu\n", GetLastError());
        if (!ConnectNamedPipe(pipe_, NULL)) {
            fprintf(stderr, "ConnectNamedPipe failed, error: %lu\n", GetLastError());
            CloseHandle(pipe_);
            pipe_ = INVALID_HANDLE_VALUE;
        }
        break;
    default:
        ASSERT_RETURN(true, , "Unknown link type in named_pipe constructor");
        break;
    }
}

named_pipe::~named_pipe()
{
    named_pipe::remove();
}

bool named_pipe::send(const void* data, size_t data_size)
{
    ASSERT_RETURN(!data, false, "Data is null");

    DWORD bytesWritten;
    BOOL success = WriteFile(
        pipe_,
        data,
        static_cast<DWORD>(data_size),
        &bytesWritten,
        NULL);

    ASSERT_RETURN(!success, false, "WriteFile failed, error = %lu", GetLastError());
    ASSERT_RETURN(bytesWritten != data_size, false, "WriteFile did not write all data, expected: %zu, written: %lu", data_size, bytesWritten);

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
    ASSERT_RETURN(!success, nullptr, "ReadFile failed, error = %lu", GetLastError());

    std::shared_ptr<void> result(malloc(bytesRead), free);
    ASSERT_RETURN(!result, nullptr, "malloc failed");

    memcpy(result.get(), buffer, bytesRead);
    return result;
}

bool named_pipe::remove()
{
    if (pipe_ != INVALID_HANDLE_VALUE) {
        DisconnectNamedPipe(pipe_);
        CloseHandle(pipe_);
        pipe_ = INVALID_HANDLE_VALUE;
    }

    return true;
}

} // namespace msgq