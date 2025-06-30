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

Run `/output/bin/IPC-test-correctness` to perform correctness testing. To perform performance testing, run `/output/bin/ipc_server` and `/output/bin/ipc_client` sequentially on different terminals.

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

#### Code Writing

**Windows**: When creating IPC communication nodes, it is necessary to specify the node type as `Receiver` or `Sender`

```cpp
#include <ipc/ipc.h>

// Create two IPC nodes named 'Wow', using named pipes at the bottom (Windows default)
ipc::node receiver("Wow", ipc::LinkType::Receiver, ipc::ChannelType::NamedPipe);
ipc::node sender("Wow", ipc::LinkType::Sender);
auto rec = receiver.receive();   // Receive message (will block the process until the message is received)
sender.send(data, sizeof(data)); // Send a message
```

**Linux**: The communication node `IPC::node` is a half duplex communication, where each instance can send or receive messages to a specific IPC channel.

```cpp
#include <ipc/ipc.h>

// Create an IPC node named 'Wow' using message queues at the bottom layer
ipc::node ipc_node("Wow", ipc::ChannelType::MessageQueue);
ipc_node.send(data, sizeof(data)); // Send a message
auto rec = ipc_node.receive();     // Receive messages
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

- Windows testing platform: 13th Gen Intel (R) Core (TM) i7-13700 (2.10 GHz)
- Linux testing platform: Intel (R) Core (TM) Ultra 9 185H

<table>
<tr>
<th rowspan="3" align="center" class="vertical-center">Communication latency / µs</th>
<th colspan="2" align="center">Windows</th>
<th align="center">Linux</th>
</tr>
<tr>
<th colspan="2" align="center">Named pipe</th>
<th align="center">Message queue</th>
</tr>
<tr>
<th align="center">MinGW</th>
<th align="center">MSVC</th>
<th align="center">GCC</th>
</tr>
<tr>
<td align="center">Average</td>
<td align="center">153.4</td>
<td align="center">154.7</td>
<td align="center">63.0</td>
</tr>
<tr>
<td align="center">Median</td>
<td align="center">137.7</td>
<td align="center">136.7</td>
<td align="center">61.4</td>
</tr>
<tr>
<td align="center">P95</td>
<td align="center">290.9</td>
<td align="center">293.5</td>
<td align="center">89.1</td>
</tr>
<tr>
<td align="center">P99</td>
<td align="center">366.9</td>
<td align="center">350.3</td>
<td align="center">119.5</td>
</tr>
</table>
