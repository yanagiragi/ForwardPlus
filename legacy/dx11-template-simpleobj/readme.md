# DirectX 11 Seed

- Modified from [jpvanoosten/LearningDirectX11](https://github.com/jpvanoosten/LearningDirectX11/tree/v1.0.0) from [this article](https://www.3dgep.com/texturing-lighting-directx-11/)

- This version supports:

    - Handmade Camera System (less functionality)

    - UI to control model & lights

    - Blinn-Phong lighting

    - Use DirectXTK::SimpleMath as math library

## Setup

First install:

- [Git](https://git-scm.com/)

- [CMake](https://cmake.org)

- [Visual Studio](https://visualstudio.microsoft.com/downloads/)

- [Windows SDK](https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/)
    
    - Use Windows SDK version 11 if you are win11, or use version 10 for win10.

Then type the following in your [terminal](https://hyper.is/).

```bash
# Clone the repo
git clone https://github.com/yanagiragi/ForwardPlus --recurse-submodules

# go inside the folder
cd "template"

# Make a build folder
mkdir build
cd build

# To build your Visual Studio solution on Windows x64
cmake .. -A x64

# π¨ Build project
cmake --build .
```
## Project Layout

```bash
ββ π src/                         # Source Files
β  ββ π Common.h                  # Utilities (Load Files, Check Shaders, etc.)
β  ββ π Main.cpp                  # Application Main
ββ π .gitignore                   # Ignore certain files in git repo
ββ π CMakeLists.txt               # Build Script
ββ π license.md                   # Your License (Unlicense)
ββ πreadme.md                     # Readme
```