# DirectX 11 Seed

Modified from [alaingalvan/directx12-seed](https://github.com/alaingalvan/directx12-seed)

## Setup

First install:

- [Git](https://git-scm.com/)

- [CMake](https://cmake.org)

- [Visual Studio](https://visualstudio.microsoft.com/downloads/)

Then type the following in your [terminal](https://hyper.is/).

```bash
# Clone the repo
git clone https://github.com/yanagiragi/ForwardPlus --recurse-submodules

# go inside the folder
cd "DirectX11 - seed"

# If you forget to `recurse-submodules` you can always run:
# git submodule update --init

# submodule is not ready for now, manual download glm for now
mkdir -p external/
git clone https://github.com/g-truc/glm.git

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
├─ 📂 external/                    # Dependencies
│  └─ 📁 glm/                            # Linear Algebra
├─ 📂 src/                         # Source Files
│  ├─ 📄 Common.h                        # Utilities (Load Files, Check Shaders, etc.)
│  └─ 📄 Main.cpp                        # Application Main
├─ 📄 .gitignore                   # Ignore certain files in git repo
├─ 📄 CMakeLists.txt               # Build Script
├─ 📄 license.md                   # Your License (Unlicense)
└─ 📃readme.md                     # Readme
```
