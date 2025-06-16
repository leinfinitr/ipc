# C++ IPC Library

## English | [简体中文](docs/README_zh-CN.md)

[![License](https://img.shields.io/badge/License-Apache_2.0-blue)](https://github.com/XpuOS/xsched/blob/main/LICENSE)

## IPC communication library with optional methods

This repository implements a C++ IPC library based on the System V IPC interface, encapsulating various IPC mechanisms such as message queues and shared memory. This library aims to simplify communication operations between processes.

### Characteristics

- The `ipc::node` instance is half duplex communication, and each instance can send or receive messages to a specific IPC channel.
- When creating an instance of `ipc::node`, the `IPC::ChannelType` parameter can be used to specify the underlying IPC channel type, such as `MessageQueue`, `SharedMemory`, etc.

### Compile

Ensure that relevant compilation tools such as GCC, Makefile, and CMake are installed, and then compile using the `make` command.

After compilation, generate a static library file named `libipc. a` in the `output` directory.

### Usage method

For projects built using CMake, the IPC library can be introduced in the following ways:

```cmake
set(IPC_LIB_PATH "/path/to/libipc.a")
set(IPC_INCLUDE_DIR "/path/to/ipc/include")
add_library(ipc STATIC IMPORTED)
set_target_properties(ipc PROPERTIES
    IMPORTED_LOCATION "${IPC_LIB_PATH}"
    INTERFACE_INCLUDE_DIRECTORIES "${IPC_INCLUDE_DIR}"
)

target_link_libraries(your_target ipc)
target_include_directories(your_target PRIVATE ${IPC_INCLUDE_DIR})
```

In C++ code, use:

```cpp
#include <ipc/ipc.h>

// Create an IPC node named 'Wow'
// using message queue at the bottom mechanism
ipc::node ipc_node("Wow", ipc::ChannelType::MessageQueue);
ipc_node.send(data, sizeof(data));  // Send a message
auto rec = ipc_node.receive();      // Receive messages
```

### Example

Two example programs are provided in the `examples` directory: `sender.cpp` and `receiver.cpp`.

The operation mode is as follows:

```bash
cd examples
# Compile
make
# Run receiver
make run_receiver
# Run sender on the new terminal
make run_sender
# At this time, the receiver can receive the message
Received message: Hello, IPC!
```
