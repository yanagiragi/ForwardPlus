# Forward plus

This project implements Foward+ from [Forward vs Deferred vs Forward+ Rendering with DirectX 11](https://www.3dgep.com/forward-plus/) using left hand coordinate and [DirectXMath](https://learn.microsoft.com/en-us/windows/win32/dxmath/directxmath-portal) as math library.

The source codes uses templates modified from [jpvanoosten/LearningDirectX11](https://github.com/jpvanoosten/LearningDirectX11/tree/v1.0.0) and [Introduction to DirectX 11](https://www.3dgep.com/introduction-to-directx-11/#DirectX_11_Pipeline). 

## Prerequisite

* [Git](https://git-scm.com/)
* [CMake](https://cmake.org)
* [Visual Studio](https://visualstudio.microsoft.com/downloads/)

## Setup

```bash
# Clone the repo
git clone  --recurse-submodules https://github.com/yanagiragi/ForwardPlus

# go inside the folder
cd "dx11-forwardplus"

# Make a build folder
mkdir build && cd build

# Generate Visual Studio solution on Windows x64
cmake .. -A x64

# Build project or manual open Visual Studio solution
cmake --build .
```