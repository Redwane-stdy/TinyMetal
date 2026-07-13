#pragma once

// Forward declarations to avoid importing metal-cpp headers in the Cocoa (.mm) file.
namespace MTL {
    class Device;
    class CommandQueue;
    class RenderPipelineState;
    class Buffer;
}

class Renderer {
public:
    Renderer(void* rawDevice);
    ~Renderer();

    void Draw(void* rawDrawable, void* rawRenderPass);

private:
    MTL::Device* _device;
    MTL::CommandQueue* _queue;
    MTL::RenderPipelineState* _pipelineState;
    MTL::Buffer* _vertexBuffer;
};
