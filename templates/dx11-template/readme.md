# DirectX 11 Seed

Modified from [jpvanoosten/LearningDirectX11](https://github.com/jpvanoosten/LearningDirectX11/tree/v1.0.0) from [this article](https://www.3dgep.com/introduction-to-directx-11/#DirectX_11_Pipeline)

- This version supports:

    - Draw Simple Cube using shader

    - Use glm as math library

## Setup

First install:

- [Git](https://git-scm.com/)

- [CMake](https://cmake.org)

- [Visual Studio](https://visualstudio.microsoft.com/downloads/)

Then type the following in your [terminal](https://hyper.is/).

```bash
# Clone the repo
git clone https://github.com/yanagiragi/ForwardPlus

# go inside the folder
cd "template"

# Make a build folder
mkdir build
cd build

# To build your Visual Studio solution on Windows x64
cmake .. -A x64

# 🔨 Build project
cmake --build .
```
## Project Layout

```bash
├─ 📂 src/                         # Source Files
│  ├─ 📄 Common.h                  # Utilities (Load Files, Check Shaders, etc.)
│  └─ 📄 Main.cpp                  # Application Main
├─ 📄 .gitignore                   # Ignore certain files in git repo
├─ 📄 CMakeLists.txt               # Build Script
├─ 📄 license.md                   # Your License (Unlicense)
└─ 📃readme.md                     # Readme
```