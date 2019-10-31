cmake_minimum_required(VERSION 3.9)
project(sphere-vis)

set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PORTAUDIO REQUIRED portaudio-2.0)
add_subdirectory(3rdparty/glfw)
add_subdirectory(3rdparty/glad)
add_subdirectory(3rdparty/glm)

add_library(glm INTERFACE)
target_include_directories(glm INTERFACE 3rdparty/glm)

add_executable(sphere-vis main.cpp)
target_include_directories(sphere-vis PRIVATE ${PORTAUDIO_INCLUDE_DIRS} include/)
target_link_libraries(sphere-vis PRIVATE ${PORTAUDIO_LIBRARIES} glfw glad glm)