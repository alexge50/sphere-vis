//
// Created by alex on 10/31/19.
//

#ifndef SPHERE_VIS_SHADER_H
#define SPHERE_VIS_SHADER_H

#include <glad/glad.h>
#include <optional>

struct Shader
{
    unsigned int programId = 0;

    Shader(unsigned int programId = 0): programId{programId} {}

    Shader(const Shader&) = delete;
    Shader(Shader&& other)
    {
        programId = other.programId;
        other.programId = 0;
    }

    Shader& operator=(const Shader&) = delete;
    Shader& operator=(Shader&& other) noexcept
    {
        glDeleteProgram(programId);
        programId = other.programId;
        other.programId = 0;

        return *this;
    }

    ~Shader()
    {
        glDeleteProgram(programId);
    }

    void use()
    {
        glUseProgram(programId);
    }

    auto getUniformLocation(const char* name)
    {
        return glGetUniformLocation(programId, name);
    }
};

std::optional<Shader> createShader(
        const std::string& vertexShaderSource,
        const std::string& fragmentShaderSource
)
{
    const char* text;

    auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &(text = vertexShaderSource.c_str()), nullptr);
    glCompileShader(vertexShader);

    auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &(text = fragmentShaderSource.c_str()), nullptr);
    glCompileShader(fragmentShader);

    Shader shader{glCreateProgram()};
    glAttachShader(shader.programId, vertexShader);
    glAttachShader(shader.programId, fragmentShader);
    glLinkProgram(shader.programId);

    return shader;
}

#endif //SPHERE_VIS_SHADER_H
