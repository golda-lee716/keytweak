# keytweak

Simple Windows keyboard remapping tool.

## Build

Requirements:
- CMake 3.16+
- Windows C compiler (MSVC, MinGW, or LLVM/Clang)

Build commands:

```powershell
cd \path\to\C-keytweak
cmake -S . -B build
cmake --build build --config Release
```

The executable will be created under `build`.

## Run

Launch the built `keytweak.exe` from the `build` output folder.

## Notes

- The app uses a low-level keyboard hook for remapping.
- Add a source key and a target key, then activate the remap.
- Close the app to uninstall the hook.
