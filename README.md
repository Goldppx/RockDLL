# RockDLL

Simple DLL injector for ReShade.

## Usage

1. Edit `config.ini`:
```ini
[INJECT]
target=NRC-Win64-Shipping.exe
dlls=C:\path\to\dxgi.dll
```

2. Run the game first

3. Run `RockDLL.exe`

## Build

```bash
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
cmake --build . --config Release
```

## Note

- Run as administrator (will prompt automatically)
- Game must be running before injection
