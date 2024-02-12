cmake_minimum_required(VERSION 3.21)
project(XuanJamesZhai_A1)

set(CMAKE_CXX_STANDARD 17)

include_directories(
        C:/VulkanSDK/glfw-3.3.9.bin.WIN64/include
        C:/VulkanSDK/stb
)

find_package(Vulkan REQUIRED)
link_directories(C:/VulkanSDK/glfw-3.3.9.bin.WIN64/lib-vc2015)

add_executable(XuanJamesZhai_A1 main.cpp XZJParser.cpp XZJParser.h VulkanHelper.cpp VulkanHelper.h S72Helper.cpp S72Helper.h XZMath.cpp XZMath.h FrustumCulling.cpp FrustumCulling.h EventHelper.cpp EventHelper.h)

target_link_libraries(XuanJamesZhai_A1 glfw3 Vulkan::Vulkan)