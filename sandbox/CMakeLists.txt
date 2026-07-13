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