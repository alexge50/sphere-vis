#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>

#include <Shader.h>

std::string loadFile(const char* name)
{
    std::ifstream file{name};

    return std::string{
        std::istreambuf_iterator<char>{file},
        std::istreambuf_iterator<char>{}
    };
}

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
        loadFile("shaders/vertex.glsl"),
        loadFile("shaders/fragment.glsl")
    ).value();

    while(!glfwWindowShouldClose(window))
    {
        int width, height;
        float ratio;

        glfwGetFramebufferSize(window, &width, &height);
        ratio = float(width) / float(height);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}