cmake_minimum_required(VERSION 3.21)
project(Cubes)

set(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

add_executable(Cubes main.cpp XZMath.cpp Lambertian.cpp Lambertian.h stb_image.h stb_image_write.h Cube.cpp Cube.h GGX.cpp GGX.h)
