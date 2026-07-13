# Getting Started with Metal-cpp on Apple Silicon

This guide explains how to write a simple program in `main.cpp`, how to manage memory using Apple's C++ wrappers, and how to build/run the project.

---

## 1. Memory Management in Metal-cpp

Metal-cpp is a header-only wrapper around Apple's Objective-C Metal APIs. Because C++ does not have Objective-C's Automatic Reference Counting (ARC), you must manage memory manually using reference counting rules similar to Objective-C:

### Rule of Ownership
- **You own** any object created by methods that start with **`new`**, **`alloc`**, or **`copy`**.
  - *Example:* `device->newCommandQueue()` returns an owning pointer. You must call `queue->release()` when you are done with it.
- **You do NOT own** objects returned by other convenience methods.
  - *Example:* `MTL::RenderPassDescriptor::renderPassDescriptor()` returns an **autoreleased** object. You do not call `release()` on it.

### Autorelease Pools
To prevent memory leaks from autoreleased objects (like descriptors or drawables), you should wrap allocation code inside an autorelease pool.
```cpp
// Create a pool
NS::AutoreleasePool* pPool = NS::AutoreleasePool::alloc()->init();

// ... allocate autoreleased objects ...

// Drain/release the pool (this releases all autoreleased objects)
pPool->release();
```

---

## 2. A Simple, Compileable `main.cpp`

Since your current `main.cpp` references an undefined variable `pipelineState` and creates a `CA::MetalLayer` without a window, it will fail to compile or crash if run directly.

Here is a minimal, clean starter template for [main.cpp](file:///Users/ijklmmr/Projects/METAL/TinyEngine/sandbox/main.cpp) that initializes the device, creates a command queue, uses an autorelease pool, and correctly cleans up resources:

```cpp
#define NS_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include <iostream>

int main() {
    // 1. Create the Autorelease Pool
    NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();

    // 2. Initialize the Metal Device (represents the GPU)
    MTL::Device* device = MTL::CreateSystemDefaultDevice();
    if (!device) {
        std::cerr << "Failed to find a Metal device." << std::endl;
        pool->release();
        return 1;
    }
    std::cout << "Metal Device: " << device->name()->utf8String() << std::endl;

    // 3. Create the Command Queue (used to submit work to the GPU)
    MTL::CommandQueue* queue = device->newCommandQueue();
    if (!queue) {
        std::cerr << "Failed to create command queue." << std::endl;
        device->release();
        pool->release();
        return 1;
    }
    std::cout << "Command Queue created successfully." << std::endl;

    // 4. Release resources we own
    queue->release();
    device->release();

    // 5. Release/drain the pool
    pool->release();

    return 0;
}
```

---

## 3. How to Build and Run the App

From the terminal at the root of the project, follow these steps to build and run:

### Step A: Configure CMake
We run `cmake -B build` to generate the compilation build system. This also generates the `compile_commands.json` database which helps VS Code or other IDEs find the `<Foundation/Foundation.hpp>` headers.
```bash
cmake -B TinyEngine/build -S TinyEngine
```

### Step B: Compile/Build
Compile your sandbox binary using:
```bash
cmake --build TinyEngine/build
```

### Step C: Launch
Run the compiled executable:
```bash
./TinyEngine/build/sandbox/Sandbox
```
