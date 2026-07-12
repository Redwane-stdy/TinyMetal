#define NS_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include <iostream>

int main() {
    MTL::Device* device = MTL::CreateSystemDefaultDevice();
    if (!device) {
        std::cerr << "Failed to find a Metal device." << std::endl;
        return 1;
    }

    std::cout << "Successfully initialized Metal-cpp!" << std::endl;
    std::cout << "Metal Device Name: " << device->name()->utf8String() << std::endl;

    device->release();
    return 0;
}