# Windows Setup Guide for Multibind

## ⚠️ Important: Visual Studio Code vs Visual Studio

**You probably have this issue:** You installed Visual Studio Code thinking it would let you compile C++ code, but it won't work for building this plugin.

### The Confusion
- **Visual Studio Code** = Text editor with extensions (what many people install)
- **Visual Studio** = Full IDE with integrated C++ compiler (what you need to build C++ projects)

They are completely different products from Microsoft!

## Solutions

You need a **C++ compiler**. Choose ONE of these options:

### Option 1: Visual Studio Community (Easiest for beginners)
**Best for:** People new to C++ development

1. Go to [visualstudio.microsoft.com](https://visualstudio.microsoft.com/downloads/)
2. Download "Visual Studio Community 2022" (free)
3. During installation, select **"Desktop development with C++"** workload
4. Install (takes ~7GB disk space)

✅ **You can keep using Visual Studio Code as your editor** - just use Visual Studio for compiling.

### Option 2: Build Tools for Visual Studio (Smaller download)
**Best for:** People who want to keep using Visual Studio Code as their editor

1. Go to [visualstudio.microsoft.com/downloads](https://visualstudio.microsoft.com/downloads/)
2. Scroll down to "Tools for Visual Studio"
3. Download "Build Tools for Visual Studio 2022" 
4. During installation, select **"C++ build tools"** workload
5. Install (takes ~3GB disk space)

### Option 3: MinGW-w64 (Open source)
**Best for:** People who prefer open source tools

#### Via winget (easiest):
```cmd
winget install mingw-w64
```

#### Via MSYS2 (manual setup):
1. Download [MSYS2](https://www.msys2.org/)
2. Install it
3. Open MSYS2 terminal and run:
   ```bash
   pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-make
   ```
4. Add `C:\msys64\mingw64\bin` to your Windows PATH environment variable

## After Installing a Compiler

1. **Run the setup script:**
   ```cmd
   setup-sdk.bat
   ```

2. **Build the plugin:**
   ```cmd
   build.bat
   ```

The build script will automatically detect which compiler you installed and use it.

## Troubleshooting

### "Could not create named file generator"
This means you don't have a C++ compiler installed. Follow one of the options above.

### "No suitable C++ compiler found"
The build script couldn't find Visual Studio, Build Tools, or MinGW-w64. Make sure you:
- Installed one of the options above
- Restarted your command prompt after installation
- For MinGW-w64: made sure it's in your PATH environment variable

### Still having issues?
1. **Check what you have installed:**
   - Run `where cl` to check for Visual Studio compiler
   - Run `where gcc` to check for MinGW-w64
   - If neither command finds anything, you need to install a compiler

2. **For Visual Studio users:** Try running the build from "Developer Command Prompt for VS" instead of regular Command Prompt

3. **For MinGW-w64 users:** Make sure `C:\msys64\mingw64\bin` (or wherever you installed it) is in your PATH

## FAQ

**Q: Can I use Visual Studio Code to edit the code?**  
A: Yes! Visual Studio Code is great for editing. You just need Visual Studio or another compiler for building.

**Q: Do I need the full Visual Studio IDE if I prefer Visual Studio Code?**  
A: No, you can install just "Build Tools for Visual Studio" (smaller download) and keep using VS Code.

**Q: Which option should I choose?**  
A: If you're new to C++: Visual Studio Community. If you want to stick with VS Code: Build Tools. If you prefer open source: MinGW-w64.

**Q: How much disk space do these take?**
- Visual Studio Community: ~7GB  
- Build Tools for Visual Studio: ~3GB
- MinGW-w64: ~500MB