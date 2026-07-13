#define NS_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include "Renderer.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

struct Vertex {
    float position[2];
    float color[4];
};

std::string ReadBuffer(const std::string& fileName){
    std::ifstream file(fileName);
    if(!file){
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

Renderer::Renderer(void *rawDevice) {
    _device = (MTL::Device *)rawDevice;
    _device->retain();
    _queue = _device->newCommandQueue();

    std::cout << "Renderer initialized with command queue: " << _queue << std::endl;

    // Load shaders
    std::string shaderSource = ReadBuffer("TinyEngine/shaders/shaders.metal");
    if (shaderSource.empty()) {
        shaderSource = ReadBuffer("shaders/shaders.metal");
    }
    if (shaderSource.empty()) {
        shaderSource = ReadBuffer("/Users/ijklmmr/Projects/METAL/TinyEngine/shaders/shaders.metal");
    }
    if (shaderSource.empty()) {
        std::cerr << "Error: Failed to find shaders.metal" << std::endl;
    }

    NS::Error* error = nullptr;
    auto sourceStr = NS::String::string(shaderSource.c_str(), NS::UTF8StringEncoding);
    MTL::Library* library = _device->newLibrary(sourceStr, nullptr, &error);
    if (!library) {
        std::cerr << "Failed to compile shader library: " 
                  << (error ? error->localizedDescription()->utf8String() : "Unknown error") 
                  << std::endl;
    }

    auto vertName = NS::String::string("vertexMain", NS::UTF8StringEncoding);
    auto fragName = NS::String::string("fragmentMain", NS::UTF8StringEncoding);
    MTL::Function* vertFunc = library->newFunction(vertName);
    MTL::Function* fragFunc = library->newFunction(fragName);

    MTL::RenderPipelineDescriptor* desc = MTL::RenderPipelineDescriptor::alloc()->init();
    desc->setVertexFunction(vertFunc);
    desc->setFragmentFunction(fragFunc);
    desc->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormatBGRA8Unorm);

    _pipelineState = _device->newRenderPipelineState(desc, &error);
    if (!_pipelineState) {
        std::cerr << "Failed to create pipeline state: " 
                  << (error ? error->localizedDescription()->utf8String() : "Unknown error") 
                  << std::endl;
    }

    desc->release();
    vertFunc->release();
    fragFunc->release();
    library->release();

    // Create vertex buffer (Top: Green, Bottom-Left: Red, Bottom-Right: Blue)
    Vertex vertices[] = {
        { {  0.0f,  0.5f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
        { { -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
        { {  0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
    };

    _vertexBuffer = _device->newBuffer(vertices, sizeof(vertices), MTL::ResourceStorageModeShared);
}

Renderer::~Renderer() {
    _vertexBuffer->release();
    _pipelineState->release();
    _queue->release();
    _device->release();
}

void Renderer::Draw(void *rawDrawable, void *rawRenderPass) {
    auto drawable = (CA::MetalDrawable *)rawDrawable;
    auto pass = (MTL::RenderPassDescriptor *)rawRenderPass;

    if (drawable && pass) {
        NS::AutoreleasePool *pool = NS::AutoreleasePool::alloc()->init();

        MTL::CommandBuffer *buffer = _queue->commandBuffer();
        MTL::RenderCommandEncoder *encoder = buffer->renderCommandEncoder(pass);

        encoder->setRenderPipelineState(_pipelineState);
        encoder->setVertexBuffer(_vertexBuffer, 0, 0);
        encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, (NS::UInteger)0, (NS::UInteger)3);

        encoder->endEncoding();
        buffer->presentDrawable(drawable);
        buffer->commit();

        pool->release();
    }
}
