#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/mat4x4.hpp>
#include <glm/common.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>

#include <Shader.h>
#include <Sphere.h>

#include <shaders.h>

std::string loadFile(const char* name)
{
    std::ifstream file{name};

    return std::string{
        std::istreambuf_iterator<char>{file},
        std::istreambuf_iterator<char>{}
    };
}

const float RADIUS = 10.f;
const int RINGS = 10;
const int SECTORS = 10;

int main()
{
    if(!glfwInit())
    {
        return -1;
    }

    glfwSetErrorCallback([](int error, const char* description){
        std::cout << "error (" << error << "): " << description << std::endl;
    });

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);

    GLFWwindow* window = glfwCreateWindow(
        512,
        512,
        "sphere-vis",
        nullptr,
        nullptr
    );

    if(!window)
    {
        return -1;
    }

    glfwMakeContextCurrent(window);
    gladLoadGL();

    Shader shader = createShader(
        std::string(reinterpret_cast<const char*>(vertex_glsl)),
        std::string(reinterpret_cast<const char*>(fragment_glsl))
    ).value();
    const auto mvpLocation = shader.getUniformLocation("mvp");

    Sphere sphere = generateSphere(RADIUS, RINGS, SECTORS);

    unsigned int vbo = 0;
    unsigned int ebo = 0;
    unsigned int vao = 0;

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        sphere.vertices.size() * sizeof(float),
        sphere.vertices.data(),
        GL_DYNAMIC_DRAW
    );

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            sphere.indices.size() * sizeof(int),
            sphere.indices.data(),
            GL_STATIC_DRAW
    );

    glGenVertexArrays(1, &vao);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        3 * sizeof(float),
        nullptr
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glEnable(GL_DEPTH_TEST);

    while(!glfwWindowShouldClose(window))
    {
        int width, height;
        float ratio;

        glfwGetFramebufferSize(window, &width, &height);
        ratio = float(width) / float(height);
        glViewport(0, 0, width, height);
        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto mvp = glm::perspective(
                0.78f,
                ratio,
                0.00001f,
                100.f
        ) * glm::lookAt(
                glm::vec3(0.f, 0.f, 40.f),
                glm::vec3(0.f, 0.f, 0.f),
                glm::vec3(0.f, 1.f, 0.f)
        );

        shader.use();
        glUniformMatrix4fv(mvpLocation, 1, 0, glm::value_ptr(mvp));

        glBindVertexArray(vao);
        glDrawElements(
            GL_LINE_STRIP,
            sphere.indices.size(),
            GL_UNSIGNED_INT,
            nullptr
        );

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}