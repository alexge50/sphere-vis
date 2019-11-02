cmake_minimum_required(VERSION 3.9)
project(sphere-vis)

set(CMAKE_CXX_STANDARD 17)

function(add_resource target)
    set(generated_file ${CMAKE_CURRENT_BINARY_DIR}/${target}/${target}.h)
    file(WRITE ${generated_file})
    foreach(input ${ARGN})
        string(REGEX MATCH "([^/]+)$" filename ${input})
        string(REGEX REPLACE "\\.| |-" "_" filename ${filename})
        file(READ ${input} data HEX)
        string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," data ${data})
        file(
                APPEND ${generated_file}
                "const unsigned char ${filename}[]={${data}0x00};\nconst unsigned ${filename}_size = sizeof(${filename});\n"
        )
    endforeach()
    add_library(${target} INTERFACE)
    target_include_directories(${target} INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/${target}/)
endfunction()

add_subdirectory(3rdparty/glfw)
add_subdirectory(3rdparty/glad)
add_subdirectory(3rdparty/glm)

add_library(glm INTERFACE)
target_include_directories(glm INTERFACE 3rdparty/glm)

add_resource(shaders shaders/fragment.glsl shaders/vertex.glsl)

add_executable(sphere-vis main.cpp)
target_include_directories(sphere-vis PRIVATE ${PORTAUDIO_INCLUDE_DIRS} include/)
target_link_libraries(sphere-vis PRIVATE ${PORTAUDIO_LIBRARIES} glfw glad glm shaders)