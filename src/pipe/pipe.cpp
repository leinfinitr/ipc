#ifdef _WIN32
#include <atomic>
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

#include "ipc/pipe/pipe.h"
#include "utils/assert.h"
#include "utils/log.h"

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

named_pipe::named_pipe(const std::string& name, NodeType ntype)
    : pipe_name_(LOCAL_PIPI + name)
    , node_type_(ntype)
    , send_pipe_(INVALID_HANDLE_VALUE)
    , send_connected_(false)
    , recv_stop_flag_(false)
{
    stop_event_ = CreateEvent(NULL, TRUE, FALSE, NULL);
    ASSERT(stop_event_ == NULL, "CreateEvent failed");

    switch (ntype) {
    case NodeType::Sender:
        // Move connection establishment to send method
        // Prevent errors caused by not creating a receiver during initialization
        break;
    case NodeType::Receiver:
        // Start the receiver thread
        recv_thread_ = std::thread(&named_pipe::recv_main, this);
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
    ASSERT_RETURN(node_type_ == NodeType::Receiver, false, "Receiver can't send data");
    ASSERT_RETURN(!data, false, "Data is null");
    ASSERT_RETURN(!connect(), false, "Connect failed in send");

    DWORD bytesWritten;
    while (true) {
        BOOL success = WriteFile(
            send_pipe_,
            data,
            static_cast<DWORD>(data_size),
            &bytesWritten,
            NULL);

        if (!success && GetLastError() == ERROR_PIPE_NOT_CONNECTED) {
            // The pipe has been closed by the other end
            LOG_INFO("The pipe has been ended, try to reconnect...");
            send_connected_ = false;
            // Reset the current pipeline instance and reconnect
            DisconnectNamedPipe(send_pipe_);
            connect();
            // Try sending data again after reconnecting
            continue;
        }
        ASSERT_RETURN(!success, false, "WriteFile failed");
        break;
    }

    ASSERT_RETURN(bytesWritten != data_size, false, "WriteFile write wrong size data, expected: %zu, written: %lu", data_size, bytesWritten);
    LOG_DEBUG("WriteFile write %lu bytes data to pipe '%s'", bytesWritten, pipe_name_.c_str());
    return true;
}

std::shared_ptr<void> named_pipe::receive()
{
    // The constructor will automatically lock queue_mutex_
    std::unique_lock<std::mutex> lock(queue_mutex_);

    // First check if there is any readable data (to avoid loss notifications)
    // Prevent the function from not being called before receiving the recv_handle_connection notification
    if (recv_queue_.empty()) {
        // The wait method will release the mutex held by the lock
        // and block the current thread until other threads call notify_one() or notify_all() to wake it up.
        queue_cv_.wait(lock, [this] {
            // After waking up, the thread will reacquire the lock and check the predicate condition
            // If true, continue with the execution
            // If false, release the lock again and block
            LOG_DEBUG("Waiting for data in named_pipe '%s'", pipe_name_.c_str());
            return !recv_queue_.empty() || recv_stop_flag_.load();
        });
    }

    ASSERT_RETURN(recv_queue_.empty(), nullptr, "recv_queue_ is empty");

    auto result = recv_queue_.front();
    recv_queue_.pop();
    LOG_DEBUG("Received data from named pipe '%s'", pipe_name_.c_str());

    return result;
}

bool named_pipe::remove()
{
    LOG_DEBUG("Removing named pipe '%s'", pipe_name_.c_str());
    recv_stop_flag_.store(true);
    SetEvent(stop_event_);

    // Notify all waiting threads
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        queue_cv_.notify_all();
    }

    if (recv_thread_.joinable()) {
        recv_thread_.join();
    }

    if (send_connected_) {
        DisconnectNamedPipe(send_pipe_);
        send_connected_ = false;
    }

    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        ASSERT(!recv_queue_.empty(), "There is unread data in the queue");
        std::queue<std::shared_ptr<void>>().swap(recv_queue_);
    }

    CloseHandle(stop_event_);
    return true;
}

void named_pipe::recv_main()
{
    while (!recv_stop_flag_.load()) {
        HANDLE pipe = CreateNamedPipeA(
            pipe_name_.c_str(),
            // CreateNamedPipeA     | CreateFileA                 | Description
            // ---------------------|-----------------------------|------------------------------------------------------------
            // PIPE_ACCESS_INBOUND  | GENERIC_READ                | Read-only for server and write-only for client.
            // PIPE_ACCESS_OUTBOUND | GENERIC_WRITE               | Write-only for server and read-only for client.
            // PIPE_ACCESS_DUPLEX   | GENERIC_READ | GENEIC_WRITE | Pipeline can be read/written by both the server and client.
            PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, // OpenMode
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, // PipeMode
            PIPE_UNLIMITED_INSTANCES, // Maximum instances of pipes with the same name
            BUFFER_SIZE, // Output buffer size
            BUFFER_SIZE, // Input buffer size
            0, // Default timeout
            NULL);

        if (pipe == INVALID_HANDLE_VALUE) {
            if (GetLastError() == ERROR_PIPE_BUSY) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
            ASSERT(true, "CreateNamedPipeA failed");
        }
        LOG_DEBUG("Named pipe '%s' created successfully", pipe_name_.c_str());

        // Set up overlapping structures and associate event
        OVERLAPPED overlapped = { 0 };
        overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        ASSERT(overlapped.hEvent == NULL, "CreateEvent failed");

        // Non blocking connection
        if (!ConnectNamedPipe(pipe, &overlapped)) {
            if (GetLastError() != ERROR_IO_PENDING) {
                ASSERT(true, "ConnectNamedPipe failed");
                CloseHandle(pipe);
                CloseHandle(overlapped.hEvent);
                continue;
            }
        }

        // Waiting for connection or stop event
        HANDLE waitHandles[2] = { overlapped.hEvent, stop_event_ };
        DWORD waitResult = WaitForMultipleObjects(2, waitHandles, FALSE, INFINITE);

        // Processing stop signal
        if (waitResult == WAIT_OBJECT_0 + 1) {
            CancelIo(pipe);
            CloseHandle(pipe);
            CloseHandle(overlapped.hEvent);
            break;
        }

        // Check the connection results
        DWORD bytesTransferred;
        if (!GetOverlappedResult(pipe, &overlapped, &bytesTransferred, FALSE)) {
            ASSERT(true, "GetOverlappedResult failed");
            CloseHandle(pipe);
            CloseHandle(overlapped.hEvent);
            continue;
        }

        // Connection successful, start working thread
        LOG_DEBUG("Named pipe '%s' connected successfully", pipe_name_.c_str());
        std::thread(&named_pipe::recv_handle_connection, this, pipe).detach();
        CloseHandle(overlapped.hEvent);
    }
}

void named_pipe::recv_handle_connection(HANDLE pipe)
{
    char buffer[BUFFER_SIZE];
    OVERLAPPED overlapped = { 0 };
    overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    while (!recv_stop_flag_.load()) {
        DWORD bytesRead;
        BOOL success = ReadFile(
            pipe,
            buffer,
            BUFFER_SIZE,
            &bytesRead,
            // Using overlapping I/O to avoid blockage
            &overlapped);

        // Handle asynchronous operations
        if (!success && GetLastError() != ERROR_IO_PENDING) {
            // Error or disconnection
            ASSERT(true, "ReadFile failed");
            break;
        }

        // Waiting for data or stop signal
        HANDLE waitHandles[2] = { overlapped.hEvent, stop_event_ };
        DWORD waitResult = WaitForMultipleObjects(2, waitHandles, FALSE, INFINITE);

        // Stop Event
        if (waitResult == WAIT_OBJECT_0 + 1) {
            CancelIo(pipe);
            break;
        }

        // Obtain reading results
        if (!GetOverlappedResult(pipe, &overlapped, &bytesRead, FALSE)) {
            if (GetLastError() == ERROR_BROKEN_PIPE) {
                // The pipe has been closed by the other end
                LOG_INFO("The pipe has been ended, close pipe instance.");
            } else
                ASSERT(true, "GetOverlappedResult failed");
            break;
        }

        // Processing valid data
        if (bytesRead > 0) {
            auto data = std::shared_ptr<void>(malloc(bytesRead), free);
            memcpy(data.get(), buffer, bytesRead);
            {
                std::lock_guard<std::mutex> lock(queue_mutex_);
                recv_queue_.push(data);
                queue_cv_.notify_one();
            }
        }
        LOG_DEBUG("ReadFile read %lu bytes data from pipe '%s'", bytesRead, pipe_name_.c_str());
    }

    // Cleanup
    CloseHandle(overlapped.hEvent);
    FlushFileBuffers(pipe);
    DisconnectNamedPipe(pipe);
    CloseHandle(pipe);
}

// Sender connects to Receiver
bool named_pipe::connect()
{
    if (send_connected_) {
        return true;
    }

    const int max_retries = 30;
    const int retry_interval_ms = 100;

    for (int i = 0; i < max_retries; ++i) {
        if (WaitNamedPipeA(pipe_name_.c_str(), NMPWAIT_USE_DEFAULT_WAIT)) {
            send_pipe_ = CreateFileA(
                pipe_name_.c_str(),
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                0,
                NULL);

            if (send_pipe_ != INVALID_HANDLE_VALUE) {
                send_connected_ = true;
                LOG_DEBUG("Connected to named pipe '%s' successfully", pipe_name_.c_str());
                return true;
            }
            ASSERT(true, "CreateFile failed, retry %d times ...", i);
        }

        if (GetLastError() != ERROR_FILE_NOT_FOUND) {
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(retry_interval_ms));
    }

    ASSERT_RETURN(true, false, "Connect failed after retrying %d times", max_retries);
}

} // namespace pipe
#endif // _WIN32