# C++ IPC Library

## [English](../README.md) | 简体中文

[![License](https://img.shields.io/badge/License-Apache_2.0-blue)](https://github.com/XpuOS/xsched/blob/main/LICENSE)

## 通信方式可选的 IPC 通信库

本仓库基于 System V IPC 接口实现了一个 C++ IPC 库，为消息队列、共享内存等多种 IPC 机制实现了封装。该库旨在简化进程间通信操作。

### 特点

- `ipc::node` 实例是半双工通信，每个实例均能向特定的 IPC 通道发送或接收消息。
- 在创建 `ipc::node` 实例时，可以通过 `ipc::ChannelType` 参数指定底层 IPC 通道类型，如 `MessageQueue`、`SharedMemory` 等。

### 编译

确保安装了 GCC、Makefile 和 CMake 等相关的编译工具，而后通过 `make` 命令编译。

编译完成后在 `output` 目录下生成 `libipc.a` 静态库文件。

### 使用方式

对于使用 CMake 构建的项目，可以通过以下方式引入 IPC 库：

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

在 c++ 代码中使用：

```cpp
#include <ipc/ipc.h>

// 创建一个名为 "Wow" 的 IPC 节点，底层使用消息队列
ipc::node ipc_node("Wow", ipc::ChannelType::MessageQueue);
ipc_node.send(data, sizeof(data));  // 发送消息
auto rec = ipc_node.receive();      // 接收消息
```

### 示例

在 `examples` 目录下提供了两个示例程序：`sender.cpp` 和 `receiver.cpp`。

运行方式如下：

```bash
cd examples
# 编译
make
# 运行 receiver
make run_receiver
# 在新终端运行 sender
make run_sender
# 此时 receiver 能接收到消息
Received message: Hello, IPC!
```
