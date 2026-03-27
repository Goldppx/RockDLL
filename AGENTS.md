# AGENTS.md - RockDLL Project

## Project Overview
This project develops a command-line tool to inject ReShade DLLs into **Roco Kingdom** (洛克王国) game.
- **Game Path**: `C:\Games\WeGameApps\rail_apps\RocoKingdom(2002304)`
- **Target EXE**: `Win64\NRC\Binaries\Win64\NRC-Win64-Shipping.exe`
- **Graphics**: DX11/DX12
- **Anti-Cheat**: ACE (lenient version)
- **Constraint**: Modified DLLs CANNOT be placed in game directory

## Build Commands

### CMake with Ninja (Recommended)
```bash
mkdir build && cd build
cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=Release
ninja
```

### CMake with MinGW Makefiles
```bash
mkdir build && cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

### Single Test
```bash
cd build
ctest -R <test_name> --output-on-failure
# Or run directly:
./injector_tests.exe
```

### Lint (if available)
```bash
cmake --build build --target lint
```

## Code Style Guidelines

### General
- C++17 standard minimum
- Windows-specific code should use `#ifdef _WIN32` guards
- 4-space indentation, no tabs
- LF line endings (`.gitattributes` enforces this)

### Naming Conventions
| Element | Style | Example |
|---------|-------|---------|
| Classes | PascalCase | `DllInjector` |
| Structs | PascalCase | `ProcessInfo` |
| Functions | PascalCase | `InjectDll` |
| Variables | camelCase | `targetPid` |
| Constants | kebab-case | `MAX_PATH_LENGTH` |
| Private members | m_ prefix | `m_hProcess` |
| Enums | PascalCase | `InjectionMethod` |
| Enum values | PascalCase | `InjectionMethod::Manual` |

### Headers & Imports
```cpp
// Order: associated header, C standard, C++ standard, third-party, project
#include "dll_injector.h"
#include <windows.h>
#include <string>
#include <vector>
#include <tlhelp32.h>
```

### Error Handling
- Use `std::optional<T>` for nullable returns
- Throw `std::runtime_error` for critical failures
- Log errors with severity levels: `ERROR`, `WARN`, `INFO`, `DEBUG`
- Always check Win32 API return values

### Types
- Use `DWORD`, `HANDLE`, `LPVOID` for Windows types
- Use `uint32_t`, `int64_t` for portable integers
- Prefer `std::string` over `char*`
- Use `std::vector<uint8_t>` for binary data

### Memory & Resources
- Use RAII for handles (`.close()`, `CloseHandle()` in destructor)
- No raw `new`/`delete` - use `std::unique_ptr` or `std::vector`
- Virtual memory: `VirtualAllocEx`/`VirtualFreeEx` paired

### Injection Strategy
Since modified DLLs cannot be in game directory, use:
1. **Manual mapping** - Map DLL into target process without file on disk
2. **Reflective DLL** - DLL exports `ReflectiveLoader()`
3. **APC injection** - Queue user-mode APC before anti-cheat initializes

## Project Structure
```
RockDLL/
├── src/
│   ├── main.cpp           # CLI entry point
│   ├── injector/          # Injection engine
│   │   ├── DllInjector.h
│   │   └── DllInjector.cpp
│   ├── process/           # Process handling
│   │   ├── ProcessManager.h
│   │   └── ProcessManager.cpp
│   └── loader/            # Reflective loader
│       ├── ReflectiveLoader.h
│       └── ReflectiveLoader.cpp
├── include/               # Public headers
├── tests/
│   └── injector_tests.cpp
├── CMakeLists.txt
└── AGENTS.md
```

## Dependencies
- Windows SDK (for `tlhelp32.h`, `psapi.h`)
- No external dependencies for core injection

## Anti-Cheat Notes
- ACE (lenient) does NOT block modding
- Avoid direct syscall hooking
- Do NOT tamper with anti-cheat components
- Manual mapping is preferred over LoadLibrary

## CLI Interface
```bash
# Basic injection
RockDLL.exe --target "NRC-Win64-Shipping.exe" --dll "path/to/reshade.dll"

# Options
--help           Show help
--target <name>   Process name (partial match)
--pid <id>       Target by PID
--dll <path>     Path to DLL to inject
--method <m>     Injection method: apc|manual|thread (default: apc)
--wait <sec>     Seconds to wait for process (default: 10)
```
