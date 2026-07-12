add_executable(Sandbox
    main.cpp
)

target_link_libraries(Sandbox
PRIVATE
    TinyEngine
    "-framework Cocoa"
    "-framework QuartzCore"
    "-framework Metal"
)