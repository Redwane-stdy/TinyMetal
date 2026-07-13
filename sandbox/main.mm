#import <Cocoa/Cocoa.h>
#import <MetalKit/MetalKit.h>
#include "Renderer.hpp"
#include <iostream>

@interface AppDelegate : NSObject <NSApplicationDelegate>
@end

@implementation AppDelegate
- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    std::cout << "Application started!" << std::endl;
}
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    return YES;
}
@end

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

    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    if (!device) {
        std::cerr << "Failed to create Metal Device!" << std::endl;
        [pool release];
        return 1;
    }

    Renderer* renderer = new Renderer((__bridge void*)device);

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

    MTKView* mtkView = [[[MTKView alloc] initWithFrame:frame device:device] autorelease];
    mtkView.clearColor = MTLClearColorMake(0.11, 0.13, 0.19, 1.0);
    [window setContentView:mtkView];

    RenderDelegate* renderDelegate = [[[RenderDelegate alloc] initWithRenderer:renderer] autorelease];
    [mtkView setDelegate:renderDelegate];

    [NSApp run];

    delete renderer;
    [device release];
    [pool release];
    return 0;
}
