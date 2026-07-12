add_library(TinyEngine INTERFACE)

target_include_directories(TinyEngine
INTERFACE
    ${CMAKE_SOURCE_DIR}/external/metal-cpp
)