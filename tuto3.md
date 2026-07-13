# Tutorial 3: Drawing the France Flag

In this tutorial, we will take what we've learned and build a more complex shape: the **French Flag** (three vertical stripes of Blue, White, and Red). 

Unlike Tutorial 2, which rendered a single triangle, this tutorial will guide you through:
1. Understanding screen space coordinates (Normalized Device Coordinates).
2. Decomposing rectangles (quads) into triangles.
3. Creating a larger vertex buffer with 18 vertices.
4. Rendering multiple shapes in a single draw call.

---

## 1. Normalized Device Coordinates (NDC)

In Metal, the screen coordinates are normalized between `-1.0` and `1.0`:
*   **X-axis:** ranges from `-1.0` (far left) to `1.0` (far right).
*   **Y-axis:** ranges from `-1.0` (bottom) to `1.0` (top).
*   **Center:** is `(0.0, 0.0)`.

To draw three equal-width vertical stripes, we split the X-axis into three zones:
1.  **Blue Stripe (Left):** from `X = -1.0` to `X = -0.333`
2.  **White Stripe (Middle):** from `X = -0.333` to `X = 0.333`
3.  **Red Stripe (Right):** from `X = 0.333` to `X = 1.0`

For all stripes, the vertical range covers the full height of the screen: from `Y = -1.0` (bottom) to `Y = 1.0` (top).

---

## 2. Decomposing Quads into Triangles

GPUs can only natively render points, lines, and triangles. To render a rectangle (a quad), we must break it into **two triangles**. 

For any rectangular stripe with corners **Top-Left (TL)**, **Bottom-Left (BL)**, **Bottom-Right (BR)**, and **Top-Right (TR)**, we draw:
*   **Triangle 1:** TL -> BL -> BR
*   **Triangle 2:** TL -> BR -> TR

```text
TL ------- TR
|  \        |
|    \      |
|      \    |
|        \  |
BL ------- BR
```

Since we have 3 stripes, and each stripe is a quad (2 triangles = 6 vertices), our vertex buffer will contain:
$$\text{3 stripes} \times \text{6 vertices} = \text{18 vertices total}$$

---

## 3. Step-by-Step Implementation

### Step A: Update the Vertex Buffer in `Renderer.cpp`

You need to replace your old 3-vertex triangle array in [Renderer.cpp](file:///Users/ijklmmr/Projects/METAL/TinyEngine/sandbox/Renderer.cpp) with the new 18-vertex array. 

Here is the code representing the geometries and official-like hex colors of the French flag:
*   **Blue:** `(0.0f, 0.14f, 0.46f, 1.0f)`
*   **White:** `(1.0f, 1.0f, 1.0f, 1.0f)`
*   **Red:** `(0.93f, 0.16f, 0.22f, 1.0f)`

Find the vertex buffer initialization inside `Renderer::Renderer()` and replace it with:

```cpp
    // Create vertex buffer for the France flag (3 stripes, 2 triangles each, 18 vertices total)
    Vertex vertices[] = {
        // --- 1. BLUE STRIPE (Left Third) ---
        // Triangle 1: Top-Left, Bottom-Left, Bottom-Right
        { { -1.0f,   1.0f }, { 0.00f, 0.14f, 0.46f, 1.0f } }, // TL
        { { -1.0f,  -1.0f }, { 0.00f, 0.14f, 0.46f, 1.0f } }, // BL
        { { -0.333f, -1.0f }, { 0.00f, 0.14f, 0.46f, 1.0f } }, // BR

        // Triangle 2: Top-Left, Bottom-Right, Top-Right
        { { -1.0f,   1.0f }, { 0.00f, 0.14f, 0.46f, 1.0f } }, // TL
        { { -0.333f, -1.0f }, { 0.00f, 0.14f, 0.46f, 1.0f } }, // BR
        { { -0.333f,  1.0f }, { 0.00f, 0.14f, 0.46f, 1.0f } }, // TR

        // --- 2. WHITE STRIPE (Middle Third) ---
        // Triangle 1: Top-Left, Bottom-Left, Bottom-Right
        { { -0.333f,  1.0f }, { 1.00f, 1.00f, 1.00f, 1.0f } }, // TL
        { { -0.333f, -1.0f }, { 1.00f, 1.00f, 1.00f, 1.0f } }, // BL
        { {  0.333f, -1.0f }, { 1.00f, 1.00f, 1.00f, 1.0f } }, // BR

        // Triangle 2: Top-Left, Bottom-Right, Top-Right
        { { -0.333f,  1.0f }, { 1.00f, 1.00f, 1.00f, 1.0f } }, // TL
        { {  0.333f, -1.0f }, { 1.00f, 1.00f, 1.00f, 1.0f } }, // BR
        { {  0.333f,  1.0f }, { 1.00f, 1.00f, 1.00f, 1.0f } }, // TR

        // --- 3. RED STRIPE (Right Third) ---
        // Triangle 1: Top-Left, Bottom-Left, Bottom-Right
        { {  0.333f,  1.0f }, { 0.93f, 0.16f, 0.22f, 1.0f } }, // TL
        { {  0.333f, -1.0f }, { 0.93f, 0.16f, 0.22f, 1.0f } }, // BL
        { {  1.0f,   -1.0f }, { 0.93f, 0.16f, 0.22f, 1.0f } }, // BR

        // Triangle 2: Top-Left, Bottom-Right, Top-Right
        { {  0.333f,  1.0f }, { 0.93f, 0.16f, 0.22f, 1.0f } }, // TL
        { {  1.0f,   -1.0f }, { 0.93f, 0.16f, 0.22f, 1.0f } }, // BR
        { {  1.0f,    1.0f }, { 0.93f, 0.16f, 0.22f, 1.0f } }  // TR
    };

    _vertexBuffer = _device->newBuffer(vertices, sizeof(vertices), MTL::ResourceStorageModeShared);
```

### Step B: Update the Draw Call in `Renderer.cpp`

Now we need to tell the GPU to draw all **18 vertices** instead of just 3. 

Find your draw call inside `Renderer::Draw()` and modify the vertex count parameter to `18`:

```cpp
        // Bind the pipeline state
        encoder->setRenderPipelineState(_pipelineState);

        // Bind the vertex buffer at buffer index 0
        encoder->setVertexBuffer(_vertexBuffer, 0, 0);

        // Draw 18 vertices (representing 6 triangles total)
        encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, (NS::UInteger)0, (NS::UInteger)18);
```

---

## 4. Shaders Code

Your shaders in [shaders/shaders.metal](file:///Users/ijklmmr/Projects/METAL/TinyEngine/shaders/shaders.metal) require **no changes** because they are already designed to take positions and colors, interpolate them, and paint the pixels. Since all vertices of each stripe have the exact same color, the color will interpolate flatly and produce three clean, solid-colored stripes!

---

## 5. Building and Launching

Run these commands from the root directory:

1.  **Build the project:**
    ```bash
    cmake --build TinyEngine/build
    ```
2.  **Launch the application:**
    ```bash
    ./TinyEngine/build/sandbox/Sandbox
    ```

You will see the window fully filled with a rendering of the French Flag!
