# SODB

SODB is a native x64 Linux debugger built to explore the low-level mechanics of process control. Inspired by "Building a Debugger" by Sy Brand, it implements core debugging functionality using the Linux ptrace API.

## Features

- **Process Control**: Launch new programs or attach to existing processes by PID.
- **Execution Management**: Resume process execution and handle stop signals.
- **Register Access**: Read and write GPRs, FPRs, and vector registers (SSE/AVX).
- **Breakpoints(WIP)**: Instruction-level software breakpoints.
- **Interactive Shell**: CLI with command history and basic help documentation.

## Technologies

- **C++23**: Using modern language features for a simpiler implementation when possible.
- **Linux ptrace**: Direct interaction with the kernel for process debugging.
- **libedit**: Terminal interaction and history.
- **Catch2**: Unit testing framework.
- **CMake**: Build system integration.

## Building and Running

### Prerequisites

This project uses vcpkg for dependency management. Make sure you have vcpkg installed and configured on your system.


### Build Instructions

1. Clone the repository:
   ```bash
   git clone https://github.com/yourusername/sodb.git
   cd sodb
   ```

2. Configure and build the project:
   ```bash
   cmake -B build -DCMAKE_TOOLCHAIN_FILE=[path-to-vcpkg]/scripts/buildsystems/vcpkg.cmake
   cmake --build build
   ```

### Usage

To start debugging a program:
```bash
./build/tools/sodb /path/to/binary
```

To attach to a running process:
```bash
./build/tools/sodb -p <pid>
```

Use the `help` command inside the debugger to see available actions.