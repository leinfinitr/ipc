#pragma once

#ifdef _WIN32
#include <atomic>
#include <condition_variable>
#include <queue>
#include <thread>
#include <windows.h>

#include "ipc/ipc.h"

using namespace ipc;

namespace pipe {

class named_pipe : public Channel {
public:
    named_pipe(const std::string& name, NodeType ntype);
    ~named_pipe();

    bool send(const void* data, size_t data_size);
    std::shared_ptr<void> receive();
    bool remove();

private:
    std::string pipe_name_;
    NodeType node_type_;
    HANDLE stop_event_;

    HANDLE send_pipe_;
    bool send_connected_;

    std::atomic<bool> recv_stop_flag_; // Determine whether the main thread has terminated
    std::thread recv_thread_; // The main thread of the server
    std::condition_variable queue_cv_; // Wake up the receive() upon receiving data
    std::mutex queue_mutex_;
    std::queue<std::shared_ptr<void>> recv_queue_;

    static const DWORD BUFFER_SIZE = 4096; // Default buffer size for named pipe communication

    bool connect();
    void recv_main();
    void recv_handle_connection(HANDLE pipe);
};
} // namespace pipe
#endif // _WIN32
