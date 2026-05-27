# KeyTweak Quick Start Guide

## 🚀 Getting Started in 5 Minutes

### Prerequisites
- Windows 7 or later
- Administrator account (for keyboard hook to work)

### Step 1: Download or Build
You have two options:

**Option A: Download Pre-built Executable** (if available)
- Download `keytweak.exe` from releases
- Run the executable

**Option B: Build from Source**
```powershell
# Open PowerShell and navigate to project folder
cd "g:\Logs\github\C-keytweak"

# Run the build script (requires CMake)
.\build.ps1

# Run the application
.\build\Release\keytweak.exe
```

### Step 2: Launch the Application
Double-click `keytweak.exe` or run from PowerShell:
```powershell
keytweak.exe
```

### Step 3: Add Your First Remap
1. Look at **"Detected Keyboard Layout"** section
2. Select a key from **"Map From"** dropdown
3. Select target key from **"Map To"** dropdown
4. Click **"+ Add Mapping"** button
5. Your remap appears in **"Active Remaps"** section

### Step 4: Activate
Click **"▶ Activate Remapping"** button

### Step 5: Test
- The button changes to **"⊗ Deactivate Remapping"**
- Status shows **"● ACTIVE"** in green
- Your keyboard remapping is now active!

---

## 📋 Common Remaps

### For Vim Users
```
Caps Lock → Escape
```

### For Ergonomics
```
Caps Lock → Left Control
Right Alt → Right Control
```

### For Gaming
```
ScrollLock → F13
PrintScreen → F12
```

### For Productivity
```
Menu Key → Super (Windows key)
Right Alt → F1 (Custom shortcut)
```

---

## 🎮 Using the Interface

### Sections Explained

**Detected Keyboard Layout**
- Shows all available keys on your keyboard
- Format: `Key Name (VK: 0xXX) | Row: X, Col: X`
- Use this to find key names

**Configure Remapping**
- "Map From": Select the key you want to replace
- "Map To": Select what it should become
- "+ Add Mapping": Create the remap
- "- Delete Selected": Remove a remap from Active Remaps list
- "✕ Clear All": Remove all remaps at once

**Active Remaps**
- Shows all configured remaps
- Format: `Source Key → Target Key`
- Click to select, then "- Delete Selected" to remove

**Status & Controls**
- **● ACTIVE** (Green): Remapping is running
- **● INACTIVE** (Gray): Remapping is stopped
- Button text changes based on state

---

## ⚠️ Troubleshooting

### Issue: "Could not install keyboard remap hook"
**Solution:** 
- Run application as Administrator
- Right-click `keytweak.exe` → "Run as administrator"
- Some antivirus software may block low-level hooks

### Issue: Keys not remapping
**Verify:**
1. Status shows "● ACTIVE" (not "● INACTIVE")
2. Your remap appears in "Active Remaps" list
3. Try a simple remap first (e.g., Caps Lock → Escape)
4. Some keys (like Fn on laptops) may not be remappable

### Issue: Application crashes
**Try:**
1. Run as Administrator
2. Close other applications that may hook keyboard
3. Update keyboard drivers
4. Check Windows for updates

### Issue: Remaps not working after screen lock
**Expected behavior:** 
- Remaps stop working when Windows is locked
- Reactivate after unlocking

### Issue: Want to undo remaps
**Solution:**
1. Close KeyTweak application
2. Keyboard returns to normal
3. No permanent changes are made

---

## 🔧 Advanced Usage

### Save Your Favorite Remaps
Since KeyTweak doesn't save between sessions, create a batch file to automate:

1. Create `start-keytweak.bat`:
```batch
@echo off
cd /d g:\Logs\github\C-keytweak\build\Release
keytweak.exe
```

2. Save your remap configuration in a text file for reference

### Schedule on Startup
1. Press `Win+R`
2. Type: `shell:startup`
3. Create shortcut to `keytweak.exe` in Startup folder
4. Right-click shortcut → Properties → Advanced → Check "Run as administrator"

### Disable Remaps Temporarily
- Click "⊗ Deactivate Remapping" button
- Keyboard immediately returns to normal
- Remaps stay configured for quick re-activation

---

## 📊 Comparison with Other Tools

| Feature | KeyTweak | AutoHotkey | SharpKeys |
|---------|----------|-----------|-----------|
| **Interface** | Modern GUI | Script files | Simple UI |
| **Setup** | Easy | Complex | Very easy |
| **Remaps** | Multiple | Unlimited | Limited |
| **Complexity** | Simple | Advanced | Very simple |
| **Learning curve** | Minimal | Steep | Minimal |
| **Temporary use** | Yes | Yes | No (permanent) |

---

## 💡 Tips & Tricks

1. **Fast Configuration**: Use Tab key to cycle through dropdown menus
2. **Quick Delete**: Select a remap from the list, click "- Delete Selected"
3. **Clear Mistakes**: Click "✕ Clear All" with confirmation to reset
4. **Check Availability**: Always check "Detected Keyboard Layout" for key names
5. **Test First**: Activate with one remap, then add more
6. **Error Prevention**: Dropdowns prevent typos unlike text input

---

## ❓ FAQ

**Q: Will this work with my gaming mouse/keyboard?**
A: Yes, as long as the key appears in "Detected Keyboard Layout"

**Q: Can I remap to the same key?**
A: No, the app prevents mapping a key to itself

**Q: Do remaps persist after restart?**
A: No, you need to reconfigure each time or use Startup folder trick

**Q: Can I have multiple keys map to the same key?**
A: Yes, multiple source keys can map to one target key

**Q: Is this safe?**
A: Yes, close the app to restore normal keyboard function immediately

**Q: Does it work with non-English keyboards?**
A: Should work with any Windows keyboard layout

**Q: Can I remap modifier keys?**
A: Yes (Ctrl, Shift, Alt, Windows key)

**Q: Is there a portable version?**
A: Yes, just copy keytweak.exe - no installation needed

---

## 📚 Additional Resources

- **Documentation**: See `GUI_IMPROVEMENTS.md` for detailed feature list
- **UI Layout**: See `UI_LAYOUT.md` for visual reference
- **Build Help**: See `build.ps1 -Help` for build options
- **Original README**: See `README.md` for technical details

---

## 🎯 Common Tasks

### Task: Remap Caps Lock to Escape
1. Select "Caps Lock" from "Map From"
2. Select "Escape" from "Map To"
3. Click "+ Add Mapping"
4. Click "▶ Activate Remapping"
✓ Done!

### Task: Remove a Remap
1. Click on the remap in "Active Remaps" list
2. Click "- Delete Selected"
3. Confirm the change
✓ Done!

### Task: Start Fresh
1. Click "✕ Clear All" button
2. Click "Yes" in confirmation
3. All remaps removed
✓ Done!

### Task: Switch Between Two Configurations
1. Activate your first configuration
2. Note down what it looks like
3. To switch:
   - Click "✕ Clear All"
   - Add your second configuration
   - Click "▶ Activate Remapping"
✓ Done!

---

## 📞 Support

If you encounter issues:
1. Check Troubleshooting section above
2. Try running as Administrator
3. Look at console output for error messages
4. Check GitHub issues for similar problems

---

**Happy Remapping! 🎹**
