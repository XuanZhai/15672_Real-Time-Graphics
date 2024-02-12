cmake_minimum_required(VERSION 3.21)
project(TestProgram_Clion)

set(CMAKE_CXX_STANDARD 20)

add_library(glfw STATIC IMPORTED)
set_target_properties(glfw PROPERTIES
        IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/includes/glfw-3.3.9/lib-vc2019/glfw3.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/includes/glfw-3.3.9/include")

set(glm_DIR includes/glm/cmake/glm)

include_directories(
        C:/VulkanSDK/stb
        C:/VulkanSDK/tiny_obj_loader
)

find_package(glm REQUIRED)
find_package(Vulkan REQUIRED)

add_executable(TestProgram_Clion main.cpp XZMath.h XZMath.cpp)

target_link_libraries(TestProgram_Clion glm::glm glfw Vulkan::Vulkan)