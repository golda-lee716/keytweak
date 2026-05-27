# keytweak

A modern Windows keyboard remapping tool with a user-friendly GUI.

## Features

✨ **User-Friendly GUI**
- Clean, organized interface with grouped sections
- Dropdown menus for easy key selection
- Visual status indicators
- Real-time keyboard layout detection

🎹 **Easy Remapping**
- Simple drag-and-drop style workflow
- Add/delete individual key remaps
- Clear all remaps with one click
- See all active remaps in a list

🔒 **Safe & Reliable**
- Low-level keyboard hook implementation
- Admin-aware error messages
- Confirmation dialogs for destructive operations

## Build

**Requirements:**
- CMake 3.16+
- Windows C compiler (MSVC, MinGW, LLVM/Clang)
- Windows SDK

**Build Commands:**

```powershell
cd \path\to\C-keytweak
cmake -S . -B build
cmake --build build --config Release
```

The executable will be created under `build\Release\keytweak.exe`.

## Run

Launch the built `keytweak.exe` from the `build\Release` folder.

**Note:** Administrator privileges may be required for the keyboard hook to work properly.

## Usage

1. **View Detected Keys** - See all available keyboard keys in the "Detected Keyboard Layout" section
2. **Select Keys** - Use dropdown menus to select "From" and "To" keys
3. **Add Mapping** - Click "+ Add Mapping" to create a key remap
4. **Manage Remaps** - View, delete, or clear remaps from the "Active Remaps" section
5. **Activate** - Click "▶ Activate Remapping" to enable the remaps
6. **Deactivate** - Click "⊗ Deactivate Remapping" to disable or close the app

## Examples

- Map Caps Lock → Escape (common for vim users)
- Map Caps Lock → Ctrl (ergonomic improvement)
- Map unused keys to frequently used functions
- Remap number pad keys for gaming

## How It Works

The app uses a low-level Windows keyboard hook to intercept key presses and remap them according to your configuration. When activated, the remapping runs system-wide until you deactivate it or close the application.

## Technical Details

- Pure C implementation (C11 standard)
- Native Windows API (Win32)
- No external dependencies except Windows SDK
- ~500 lines of core remapping logic

## Notes

- The app uses a low-level keyboard hook for remapping
- Multiple remaps can be configured and activated together
- Remaps persist only during the session
- Close the app to uninstall the keyboard hook
- Administrator privileges may be required on some systems

## License

See LICENSE file for details.

---

**For detailed information on GUI improvements, see [GUI_IMPROVEMENTS.md](GUI_IMPROVEMENTS.md)**
