# Tutorial 2: Rendering a Colored Triangle

Your goal in this tutorial is to render a simple colored triangle in the center of your dark slate blue window. 

To achieve this, you will write a Metal Shading Language (MSL) shader, load it into your C++ engine, configure a Render Pipeline State, allocate vertex data on the GPU, and encode draw commands.

---

## The Big Picture (Conceptual Pipeline)

To draw anything on the GPU, you must configure a **Pipeline**. For rendering, this involves:
1. **Vertex Shader:** A GPU program that takes your vertex coordinates and projects them onto the screen.
2. **Fragment Shader:** A GPU program that decides the final color of each pixel inside the triangle.
3. **Pipeline State Object (PSO):** A configuration state on the GPU that links your shaders, defines the pixel formats, and optimizes execution.
4. **Vertex Buffer:** An array of memory allocated directly on the GPU containing the triangle's vertices (coordinates and colors).

---

## Step 1: Write the Shaders (MSL)

Create a new file named `shaders.metal` in your `shaders/` directory (or inside your sandbox folder). You will need to write the shader code using the Metal Shading Language.

### Concept:
*   Define a `struct` representing the vertex input (e.g., 2D/3D position and RGBA color).
*   Define a `struct` representing the output of the vertex shader (which will be interpolated and sent to the fragment shader). Note: The output struct **must** contain a member with the `[[position]]` attribute.
*   Write a **vertex shader function** marked with the `vertex` keyword. It takes the array of vertex inputs, selects the current vertex using `[[vertex_id]]`, and returns the output struct.
*   Write a **fragment shader function** marked with the `fragment` keyword. It takes the interpolated output struct from the vertex shader and returns the final color (`half4` or `float4`).

---

## Step 2: Load the Shaders in C++

In your `Renderer` class initialization, you need to load these shaders from the source file.

### API Methods to Look Up & Use:
1.  Read the shader file content as a string in C++ (using `std::ifstream`).
2.  Convert the standard C++ string into an `NS::String` (using `NS::String::string(const char*, NS::StringEncoding)`).
3.  Compile the shader source code at runtime using `device->newLibrary()`. You will need to pass:
    *   The shader source string (`NS::String*`).
    *   Compile options (`MTL::CompileOptions*` - you can pass `nullptr` or construct a default one).
    *   An error pointer (`NS::Error**`) to inspect compile errors if compilation fails.
4.  Extract the vertex and fragment functions from the library using `library->newFunction(NS::String* name)`. Use the exact function names you wrote in your `.metal` shader file.

---

## Step 3: Create the Render Pipeline State

The GPU needs to know the layout of your rendering pipeline before executing drawing calls.

### API Methods to Look Up & Use:
1.  Create a Render Pipeline Descriptor using `MTL::RenderPipelineDescriptor::alloc()->init()`.
2.  Set the vertex function on the descriptor using `setVertexFunction(MTL::Function*)`.
3.  Set the fragment function on the descriptor using `setFragmentFunction(MTL::Function*)`.
4.  Set the color attachment format at index `0` to match your MTKView's pixel format (e.g., `descriptor->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormatBGRA8Unorm)`).
5.  Compile this descriptor into a pipeline state object using `device->newRenderPipelineState(MTL::RenderPipelineDescriptor*, NS::Error**)`.
6.  Remember to release the descriptor and functions once the pipeline state is created to prevent memory leaks!

---

## Step 4: Allocate Vertex Data on the GPU

You need to describe the 3 corners of the triangle and send them to the GPU.

### Concept:
*   Define a C++ structure that mirrors the vertex layout in your shader (e.g., position: `float` x 2 or x 3, color: `float` x 4).
*   Create a local array containing the 3 vertices:
    *   Top: `(0.0, 0.5)` (Green)
    *   Bottom-Left: `(-0.5, -0.5)` (Red)
    *   Bottom-Right: `(0.5, -0.5)` (Blue)
*   Allocate a GPU buffer to hold this data using `device->newBuffer(const void* pointer, NS::UInteger length, MTL::ResourceOptions options)`.
    *   *Tip:* Use `MTL::ResourceStorageModeShared` so both the CPU can write to it and the GPU can read from it.

---

## Step 5: Encode the Draw Commands

In your `Draw` loop (inside your `Renderer::Draw` function), configure the render command encoder to bind the pipeline and draw the triangle.

### Sequence of Encoder Calls:
1.  Set the pipeline state on the encoder: `encoder->setRenderPipelineState(MTL::RenderPipelineState*)`.
2.  Bind your vertex buffer to index `0`: `encoder->setVertexBuffer(MTL::Buffer*, NS::UInteger offset, NS::UInteger index)`.
3.  Issue the draw call: `encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger vertexStart, NS::UInteger vertexCount)`.
    *   *Parameters:* Type should be `MTL::PrimitiveTypeTriangle`, starting vertex is `0`, and vertex count is `3`.

---

## Step 6: Update CMake

Be sure to update [TinyEngine/sandbox/CMakeLists.txt](file:///Users/ijklmmr/Projects/METAL/TinyEngine/sandbox/CMakeLists.txt) if you need to add your new shader or source files, and run the configuration and build steps.

Good luck! Take it step-by-step and inspect compiler errors carefully.
