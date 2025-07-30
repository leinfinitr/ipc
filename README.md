# C++ IPC Library

## English | [简体中文](docs/README_zh-CN.md)

[![License](https://img.shields.io/badge/License-Apache_2.0-blue)](https://github.com/XpuOS/xsched/blob/main/LICENSE)

## IPC communication library with optional communication methods

This repository implements a C++ IPC library based on the **Linux System V IPC interface** and **Windows communication interface**, encapsulating various IPC mechanisms such as message queues and named pipes. This library aims to simplify communication operations between processes.

### Characteristics

- Supports both Linux and Windows operating systems, as well as multiple compilers such as GCC, MinGW, MSVC, etc.
- When creating a communication node abstraction `IPC::node`, the `IPC::ChannelType` parameter can be used to specify the underlying IPC channel type, such as `messageQueue`, `NamedPipe`, etc.

### Compile and Test

Ensure that relevant compilation tools such as Makefile and CMake are installed, and then compile using the `make` command. After compilation, a static library file named `libipc.a`(Linux) or `ipc.lib`(Windows) will be generated in the `output` directory.

- Correctness Test: `/output/bin/ipc-test-correctness`
- Performance Test: run `/output/bin/ ipc-test-performance-server` and `/output/bin/ipc-test-performance-client` sequentially on different terminals.

### Communication method support

- ✅ Realized
- 🔘 Unrealized
- 🚧 Implementing

<table>
<tr>
<th rowspan="2" align="center" class="vertical-center">Communication method</th>
<th colspan="2" align="center">OS</th>
</tr>
<tr>
<th align="center">Windows</th>
<th align="center">Linux</th>
</tr>
<tr>
<td align="center">Named pipe</td>
<td align="center">✅</td>
<td align="center">🔘</td>
</tr>
<tr>
<td align="center">Message queue</td>
<td align="center">🔘</td>
<td align="center">✅</td>
</tr>
<tr>
<td align="center">Shared memory</td>
<td align="center">🚧</td>
<td align="center">🚧</td>
</tr>
</table>

### Usage method

#### Introduce IPC library

For projects built using CMake, the IPC library can be introduced through the following methods:

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

Alternatively, it can be used directly as a third-party library in CMakeLists.txt:

```cmake
add_subdirectory(ipc EXCLUDE_FROM_ALL)
set(IPC_INCLUDE_DIR "/path/to/ipc/include")
target_link_libraries(your_target ipc)
target_include_directories(your_target PRIVATE ${IPC_INCLUDE_DIR})
```

#### Code Writing

```cpp
#include <ipc/ipc.h>

// Create two IPC nodes named 'Wow', using named pipe (Windows default) or message queue (Linux default) at the bottom 
// ipc::node receiver("Wow", ipc::NodeType::Receiver, ipc::ChannelType::NamedPipe);
// ipc::node receiver("Wow", ipc::NodeType::Receiver, ipc::ChannelType::MessageQueue);
ipc::node receiver("Wow", ipc::NodeType::Receiver);
ipc::node sender("Wow", ipc::NodeType::Sender);
auto rec = receiver.receive();   // Receive message (will block the process until the message is received)
sender.send(data, sizeof(data)); // Send a message
```

### Example (Linux)

Two example programs are provided in the `examples` directory: `sender.cpp` and `receiver.cpp`.

The usage method is as follows:

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

### Performance

- **Windows**: 13th Gen Intel (R) Core (TM) i7-13700 (2.10 GHz)
- **Linux**: Intel (R) Core (TM) Ultra 9 185H

<table>
<tr>
<th rowspan="2" align="center" class="vertical-center">Communication latency / µs</th>
<th colspan="3" align="center">Windows(Named pipe)</th>
<th colspan="2" align="center">Linux(Message queue)</th>
</tr>
<tr>
<th align="center">ipc(1-1)</th>
<th align="center">ipc(1-N)</th>
<th align="center"><a href="https://github.com/mutouyun/cpp-ipc">cpp-ipc</a></th>
<th align="center">ipc</th>
<th align="center"><a href="https://github.com/mutouyun/cpp-ipc">cpp-ipc</a></th>
</tr>
<tr>
<td align="center">Average</td>
<td align="center">153.4</td>
<td align="center">239.1</td>
<td align="center">198.3</td>
<td align="center">61.5</td>
<td align="center">47.0</td>
</tr>
<tr>
<td align="center">Median</td>
<td align="center">137.7</td>
<td align="center">229.9</td>
<td align="center">179.5</td>
<td align="center">54.0</td>
<td align="center">45.9</td>
</tr>
<tr>
<td align="center">P95</td>
<td align="center">290.9</td>
<td align="center">416.7</td>
<td align="center">356.0</td>
<td align="center">89.1</td>
<td align="center">60.1</td>
</tr>
<tr>
<td align="center">P99</td>
<td align="center">366.9</td>
<td align="center">523.4</td>
<td align="center">450.7</td>
<td align="center">119.5</td>
<td align="center">79.1</td>
</tr>
</table>

Due to Windows NamedPipe not supporting multiple clients connecting to a server instance simultaneously:

- **IPC (1-1)**: Use default one-to-one connection
- **IPC (1-N)**: Adopting a server-side multi instance and overlapped I/O approach, allowing a single Receiver to connect to multiple Senders simultaneously
