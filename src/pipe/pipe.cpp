#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include "ipc/assert.h"
#include "ipc/pipe/pipe.h"

namespace pipe {

// The name of the named pipeline follows the following format:
// "\\<ServerName>\pipe\<PipeName>"
// - ServerName can be the name of a remote computer or a dot (.) representing the local computer.
// - PipeName is the name given to a pipeline, which can contain any character except for the back slash (\),
//   and the length of the entire name string should be limited to 256 characters.
//   Pipeline names are not sensitive to uppercase and lowercase letters.
// Due to the inability of the server to create pipelines on remote hosts, the <ServerName> section can only be a single point.
// The client can write <ServerName> as a dot or remote hostname.
#define LOCAL_PIPI R"(\\.\pipe\)"

named_pipe::named_pipe(const std::string& name, LinkType ltype)
    : pipe_(INVALID_HANDLE_VALUE)
    , pipe_name_(LOCAL_PIPI + name)
    , link_type_(ltype)
    , connected_(false)
{
    switch (ltype) {
    case LinkType::Unknown:
        ASSERT_RETURN(true, , "Link type must be specified in named_pipe constructor");
        break;
    case LinkType::Sender:
        // Move connection establishment to send method
        // Prevent errors caused by not creating a receiver during initialization
        break;
    case LinkType::Receiver:
        pipe_ = CreateNamedPipeA(
            pipe_name_.c_str(),
            PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, // OpenMode
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, // PipeMode
            PIPE_UNLIMITED_INSTANCES, // Maximum instances
            BUFFER_SIZE, // Output buffer size
            BUFFER_SIZE, // Input buffer size
            0, // Default timeout
            NULL);

        ASSERT_RETURN(pipe_ == INVALID_HANDLE_VALUE, , "CreateNamedPipeA failed, error = %lu", GetLastError());
        // Move connection establishment to receive method
        // Prevent blocking during initialization
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
    ASSERT_RETURN(link_type_ == LinkType::Receiver, false, "Receiver can't send data");
    ASSERT_RETURN(!data, false, "Data is null");
    ASSERT_RETURN(!connect(), false, "Connect failed in send");

    DWORD bytesWritten;
    while (true) {
        BOOL success = WriteFile(
            pipe_,
            data,
            static_cast<DWORD>(data_size),
            &bytesWritten,
            NULL);

        if (!success && GetLastError() == ERROR_PIPE_NOT_CONNECTED) {
            // The pipe has been closed by the other end
            printf("[ipc/pipe: sender] The pipe has been ended.\n");
            printf("[ipc/pipe: sender] Waiting for receiver connection...\n");
            connected_ = false;
            // Reset the current pipeline instance and reconnect
            DisconnectNamedPipe(pipe_);
            connect();
            // Try sending data again after reconnecting
            continue;
        }
        ASSERT_RETURN(!success, false, "WriteFile failed, error = %lu", GetLastError());
        break;
    }

    ASSERT_RETURN(bytesWritten != data_size, false, "WriteFile did not write all data, expected: %zu, written: %lu", data_size, bytesWritten);
    return true;
}

std::shared_ptr<void> named_pipe::receive()
{
    ASSERT_RETURN(link_type_ == LinkType::Sender, nullptr, "Sender can't receive data");
    ASSERT_RETURN(!connect(), false, "Connect failed in receive");

    char buffer[BUFFER_SIZE];
    DWORD bytesRead;

    while (true) {
        BOOL success = ReadFile(
            pipe_,
            buffer,
            BUFFER_SIZE,
            &bytesRead,
            NULL);
        if (!success && GetLastError() == ERROR_BROKEN_PIPE) {
            // The pipe has been closed by the other end
            printf("[ipc/pipe: receiver] The pipe has been ended.\n");
            printf("[ipc/pipe: receiver] Waiting for new connection...\n");
            connected_ = false;
            // Reset the current pipeline instance and reconnect
            DisconnectNamedPipe(pipe_);
            connect();
            // Try receiving data again after reconnecting
            continue;
        }

        ASSERT_RETURN(!success, nullptr, "ReadFile failed, error = %lu", GetLastError());
        break;
    }

    std::shared_ptr<void> result(malloc(bytesRead), free);
    ASSERT_RETURN(!result, nullptr, "malloc failed");

    memcpy(result.get(), buffer, bytesRead);
    return result;
}

bool named_pipe::remove()
{
    if (pipe_ != INVALID_HANDLE_VALUE) {
        DisconnectNamedPipe(pipe_);
        if (link_type_ == LinkType::Receiver)
            CloseHandle(pipe_);
        pipe_ = INVALID_HANDLE_VALUE;
        connected_ = false;
    }

    return true;
}

bool named_pipe::connect()
{
    if (connected_)
        return true;

    if (link_type_ == LinkType::Sender) {
        ASSERT_RETURN(!WaitNamedPipeA(pipe_name_.c_str(), NMPWAIT_USE_DEFAULT_WAIT), false, "No Named Pipe Accessible, erro = %lu", GetLastError());
        pipe_ = CreateFileA(
            pipe_name_.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            0,
            NULL);

        ASSERT_RETURN(pipe_ == INVALID_HANDLE_VALUE, false, "CreateFileA failed, erro = %lu", GetLastError());
        connected_ = true;
    }

    if (link_type_ == LinkType::Receiver) {
        if (!ConnectNamedPipe(pipe_, NULL)) {
            DWORD err = GetLastError();
            if (err == ERROR_PIPE_CONNECTED) {
                connected_ = true;
            } else {
                printf("ConnectNamedPipe failed, error = %lu\n", err);
                // Reset the pipeline for the next connection
                DisconnectNamedPipe(pipe_);
            }
        } else {
            connected_ = true;
        }
    }

    return connected_;
}

} // namespace pipe