#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <portaudio.h>

#include <iostream>
#include <fstream>
#include <numeric>
#include <algorithm>

#include <fft.h>
#include <RingBuffer.h>
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

const int SAMPLE_RATE = 44100;
const int SAMPLES = 32;
const int SLIDING_WINDOW_SIZE = 16384;

using BufferType = RingBuffer<float, SAMPLE_RATE>;

int audioCallback(
    const void* inputBuffer,
    void*,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo*,
    PaStreamCallbackFlags,
    void* data
)
{
    auto* buffer = static_cast<BufferType*>(data);
    auto in = static_cast<const float*>(inputBuffer);

    for(int i = 0; i < framesPerBuffer; i++)
    {
        buffer->push(*in++);
    }

    return 0;
}

template<std::size_t N>
void remap_to_sphere(Sphere& sphere, std::array<float, N> points)
{
    int verticesCount = sphere.vertices.size() / 3;
    struct
    {
        int sphere_start, sphere_end;
        int points_start, points_end;
    } sections[4] = {
        {verticesCount / 4, verticesCount / 2, 1, N / 8},
        {verticesCount / 2, 3 * verticesCount / 4, N / 8, N / 4},
        {0, verticesCount / 4, N / 4, N / 2},
        {3 * verticesCount / 4, verticesCount - 1, N / 2, N - 1}
    };

    for(const auto& section: sections)
    {
        const int length =
            (section.points_end - section.points_start) /
            (section.sphere_end - section.sphere_start + 1) + 1;
        int p = section.points_start;
        for(int i = section.sphere_start; i <= section.sphere_end; i++)
        {
            float value = *std::max_element(
                points.begin() + p,
                points.begin() + std::min(section.points_end, p + length) + 1
            );
            p += length;

            glm::vec3 v{
                sphere.vertices[i * 3 + 0],
                sphere.vertices[i * 3 + 1],
                sphere.vertices[i * 3 + 2],
            };

            auto displaced = v + value * RADIUS * glm::normalize(v);

            sphere.vertices[i * 3 + 0] = displaced.x;
            sphere.vertices[i * 3 + 1] = displaced.y;
            sphere.vertices[i * 3 + 2] = displaced.z;
        }
    }
}

int run(GLFWwindow* window)
{
    Shader shader = createShader(
            std::string(reinterpret_cast<const char*>(vertex_glsl)),
            std::string(reinterpret_cast<const char*>(fragment_glsl))
    ).value();
    const auto mvpLocation = shader.getUniformLocation("mvp");

    const Sphere identitySphere = generateSphere(RADIUS, RINGS, SECTORS);

    unsigned int vbo = 0;
    unsigned int ebo = 0;
    unsigned int vao = 0;

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
            GL_ARRAY_BUFFER,
            identitySphere.vertices.size() * sizeof(float),
            identitySphere.vertices.data(),
            GL_DYNAMIC_DRAW
    );

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            identitySphere.indices.size() * sizeof(int),
            identitySphere.indices.data(),
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

    int numDevices;
    numDevices = Pa_GetDeviceCount();
    if(numDevices == 0)
    {
        std::cout << "ERROR: no devices available" << std::endl;
        return -1;
    }
    else if (numDevices < 0)
    {
        std::cout << "PortAudio error: " << Pa_GetErrorText(numDevices) << std::endl;
        return -1;
    }


    BufferType buffer;
    PaStream *stream;
    if(PaError error = Pa_OpenDefaultStream(
        &stream,
        1,
        0,
        paFloat32,
        SAMPLE_RATE,
        SAMPLES,
        audioCallback,
        &buffer
    ); error != paNoError)
    {
        std::cout << "PortAudio error: " << Pa_GetErrorText(error) << std::endl;
        return -1;
    }

    if(PaError error = Pa_StartStream(stream); error != paNoError)
    {
        std::cout << "PortAudio error: " << Pa_GetErrorText(error) << std::endl;
        return -1;
    }

    Sphere sphere = identitySphere;

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

        if(buffer.size() >= SLIDING_WINDOW_SIZE)
        {
            std::array<std::complex<float>, SLIDING_WINDOW_SIZE> x;

            for(int i = 0; i < SLIDING_WINDOW_SIZE; i++)
                x[i] = buffer[i];

            while(buffer.size() > SLIDING_WINDOW_SIZE)
                buffer.raw_pop();

            x = fft(x);

            std::array<float, SLIDING_WINDOW_SIZE / 2> frequencies{};
            for(int i = 0; i < frequencies.size(); i++)
                frequencies[i] = sinf(std::abs(x[i]) / 100.f);

            sphere.vertices = identitySphere.vertices;
            remap_to_sphere(sphere, frequencies);

            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferSubData(
                GL_ARRAY_BUFFER,
                0,
                sphere.vertices.size() * sizeof(float),
                sphere.vertices.data()
            );
        }


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

    return 0;
}

int main()
{
    if(!glfwInit())
    {
        return -1;
    }

    if(PaError error = Pa_Initialize(); error != paNoError)
    {
        printf("PortAudio error: %s\n", Pa_GetErrorText(error));
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

    int error = run(window);

    if(PaError error = Pa_Terminate(); error != paNoError)
        std::cout << "PortAudio error: " << Pa_GetErrorText(error) << std::endl;

    glfwDestroyWindow(window);
    glfwTerminate();

    return error;
}