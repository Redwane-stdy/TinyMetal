# Tutorial 1: Native macOS Window & Decoupled Render Loop

In this tutorial, we will take the next step: transitioning from a console application to a **native macOS window** with a live **render loop** that clears the screen to a custom color.

To do this, we will implement a professional **decoupled engine architecture** that separates the macOS windowing code from the core C++ rendering code.

---

## 1. The Header Conflict (Why we decouple)

If you attempt to `#include <Metal/Metal.hpp>` (`metal-cpp`) and `#import <Metal/Metal.h>` (the Objective-C SDK imported by `MetalKit.h`) in the same file, the compiler throws redeclaration errors. Both headers declare the exact same global symbols (like `MTLCommonCounterSetTimestamp`), but with conflicting types (Objective-C classes vs C++ classes).

### The Solution: Decoupled Architecture
*   **`main.mm` (Objective-C++):** Handles native Cocoa windowing, creates the `MTKView`, and implements the `MTKViewDelegate` render loop. It only includes native macOS headers and uses `(__bridge void*)` to pass Metal pointers to C++.
*   **`Renderer.hpp` (C++ Header):** A clean C++ header that declares the `Renderer` class. It uses C++ forward declarations to avoid including `metal-cpp` in files that other Objective-C++ files import.
*   **`Renderer.cpp` (C++ Source):** A pure C++ source file where we define our private implementations and include `<Metal/Metal.hpp>` to perform the actual GPU commands.

---

## 2. Component Files

### 1. [Renderer.hpp](file:///Users/ijklmmr/Projects/METAL/TinyEngine/sandbox/Renderer.hpp)
This header is visible to both C++ and Objective-C++ files. It uses forward declarations to avoid header leaks:

```cpp
#pragma once

// Forward declarations to avoid importing metal-cpp headers in Cocoa files
namespace MTL {
    class Device;
    class CommandQueue;
}

class Renderer {
public:
    Renderer(void* rawDevice);
    ~Renderer();

    // Receives raw pointers bridged from Objective-C
    void Draw(void* rawDrawable, void* rawRenderPass);

private:
    MTL::Device* _device;
    MTL::CommandQueue* _queue;
};
```

### 2. [Renderer.cpp](file:///Users/ijklmmr/Projects/METAL/TinyEngine/sandbox/Renderer.cpp)
Here we import `metal-cpp` and write standard C++ GPU code:

```cpp
#define NS_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include "Renderer.hpp"
#include <iostream>

Renderer::Renderer(void* rawDevice) {
    _device = (MTL::Device*)rawDevice;
    _device->retain(); // Increment reference count to own it
    _queue = _device->newCommandQueue();
    std::cout << "Renderer initialized with command queue: " << _queue << std::endl;
}

Renderer::~Renderer() {
    _queue->release();
    _device->release();
}

void Renderer::Draw(void* rawDrawable, void* rawRenderPass) {
    auto drawable = (CA::MetalDrawable*)rawDrawable;
    auto pass = (MTL::RenderPassDescriptor*)rawRenderPass;

    if (drawable && pass) {
        NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();

        MTL::CommandBuffer* buffer = _queue->commandBuffer();
        MTL::RenderCommandEncoder* encoder = buffer->renderCommandEncoder(pass);
        
        // (Any future GPU draw commands/pipeline binds go here)
        
        encoder->endEncoding();
        buffer->presentDrawable(drawable);
        buffer->commit();

        pool->release();
    }
}
```

### 3. [main.mm](file:///Users/ijklmmr/Projects/METAL/TinyEngine/sandbox/main.mm)
Here we handle native Cocoa application lifecycle events, windows, and views:

```objc
#import <Cocoa/Cocoa.h>
#import <MetalKit/MetalKit.h>
#include "Renderer.hpp"
#include <iostream>

// Application delegate to handle program lifecycle
@interface AppDelegate : NSObject <NSApplicationDelegate>
@end

@implementation AppDelegate
- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    std::cout << "Application started!" << std::endl;
}
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    return YES; // Closes the app when the window is closed
}
@end

// Rendering delegate to bridge Cocoa's MTKView to our C++ Renderer
@interface RenderDelegate : NSObject <MTKViewDelegate> {
    Renderer* _renderer;
}
- (instancetype)initWithRenderer:(Renderer*)renderer;
@end

@implementation RenderDelegate
- (instancetype)initWithRenderer:(Renderer*)renderer {
    self = [super init];
    if (self) {
        _renderer = renderer;
    }
    return self;
}

- (void)mtkView:(MTKView *)view drawableSizeWillChange:(CGSize)size {
}

- (void)drawInMTKView:(MTKView *)view {
    id<CAMetalDrawable> drawable = [view currentDrawable];
    MTLRenderPassDescriptor* pass = [view currentRenderPassDescriptor];
    
    // Cast and pass native Objective-C pointers to our C++ Renderer
    if (drawable && pass) {
        _renderer->Draw((__bridge void*)drawable, (__bridge void*)pass);
    }
}
@end

int main(int argc, char* argv[]) {
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

    [NSApplication sharedApplication];
    
    AppDelegate* appDelegate = [[[AppDelegate alloc] init] autorelease];
    [NSApp setDelegate:appDelegate];

    // Create the system default Metal device
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    if (!device) {
        std::cerr << "Failed to create Metal Device!" << std::endl;
        [pool release];
        return 1;
    }

    // Initialize our C++ Renderer
    Renderer* renderer = new Renderer((__bridge void*)device);

    // Create the window
    NSRect frame = NSMakeRect(0, 0, 800, 600);
    NSWindow* window = [[[NSWindow alloc] initWithContentRect:frame
                                                    styleMask:(NSWindowStyleMaskTitled | 
                                                               NSWindowStyleMaskClosable | 
                                                               NSWindowStyleMaskResizable | 
                                                               NSWindowStyleMaskMiniaturizable)
                                                      backing:NSBackingStoreBuffered
                                                        defer:NO] autorelease];
    [window setTitle:@"Tiny Metal Engine - Tutorial 1"];
    [window center];
    [window makeKeyAndOrderFront:nil];

    // Create MTKView and set clear color (dark slate blue)
    MTKView* mtkView = [[[MTKView alloc] initWithFrame:frame device:device] autorelease];
    mtkView.clearColor = MTLClearColorMake(0.11, 0.13, 0.19, 1.0);
    [window setContentView:mtkView];

    // Assign render delegate
    RenderDelegate* renderDelegate = [[[RenderDelegate alloc] initWithRenderer:renderer] autorelease];
    [mtkView setDelegate:renderDelegate];

    // Start event loop
    [NSApp run];

    // Cleanup
    delete renderer;
    [device release];
    [pool release];
    return 0;
}
```

---

## 3. CMake Configuration

To build this structure, we update [TinyEngine/sandbox/CMakeLists.txt](file:///Users/ijklmmr/Projects/METAL/TinyEngine/sandbox/CMakeLists.txt):

```cmake
add_executable(Sandbox
    main.mm
    Renderer.cpp
)

target_link_libraries(Sandbox
PRIVATE
    TinyEngine
    "-framework Cocoa"
    "-framework QuartzCore"
    "-framework Metal"
    "-framework MetalKit"
)
```

---

## 4. Building and Launching

Run these commands from the root directory:

1. **Configure CMake:**
   ```bash
   cmake -B TinyEngine/build -S TinyEngine
   ```
2. **Build the project:**
   ```bash
   cmake --build TinyEngine/build
   ```
3. **Launch the window:**
   ```bash
   ./TinyEngine/build/sandbox/Sandbox
   ```
You will see a beautiful dark slate blue window appear on screen, rendering live at your monitor's native refresh rate!
