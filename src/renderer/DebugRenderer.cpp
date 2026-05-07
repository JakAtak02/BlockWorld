#include "renderer/DebugRenderer.h"

#include <glad/glad.h>

#include <iostream>

unsigned int DebugRenderer::compileShader(unsigned int type, const char* source)
{
    unsigned int shader = glCreateShader(type);

    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        char infoLog[512];

        glGetShaderInfoLog(shader, 512, nullptr, infoLog);

        std::cout
            << "Debug shader compile error:\n"
            << infoLog
            << std::endl;
    }

    return shader;
}

DebugRenderer::DebugRenderer()
{
    const char* vertexSource = R"(
        #version 460 core

        layout (location = 0) in vec3 aPos;

        uniform mat4 uProjection;
        uniform mat4 uView;

        void main()
        {
            gl_Position =
                uProjection *
                uView *
                vec4(aPos, 1.0);
        }
    )";

    const char* fragmentSource = R"(
        #version 460 core

        out vec4 FragColor;

        void main()
        {
            FragColor = vec4(1.0, 1.0, 0.0, 1.0);
        }
    )";

    unsigned int vertexShader =
        compileShader(GL_VERTEX_SHADER, vertexSource);

    unsigned int fragmentShader =
        compileShader(GL_FRAGMENT_SHADER, fragmentSource);

    m_shaderProgram = glCreateProgram();

    glAttachShader(m_shaderProgram, vertexShader);
    glAttachShader(m_shaderProgram, fragmentShader);

    glLinkProgram(m_shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(float) * 3 * 24,
        nullptr,
        GL_DYNAMIC_DRAW
    );

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        3 * sizeof(float),
        (void*)0
    );

    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

DebugRenderer::~DebugRenderer()
{
    if (m_vbo != 0)
    {
        glDeleteBuffers(1, &m_vbo);
    }

    if (m_vao != 0)
    {
        glDeleteVertexArrays(1, &m_vao);
    }

    if (m_shaderProgram != 0)
    {
        glDeleteProgram(m_shaderProgram);
    }
}

void DebugRenderer::drawChunkBorder(
    const glm::vec3& min,
    const glm::vec3& max,
    const glm::mat4& projection,
    const glm::mat4& view
)
{
    float vertices[] =
    {
        min.x, min.y, min.z,  max.x, min.y, min.z,
        max.x, min.y, min.z,  max.x, min.y, max.z,
        max.x, min.y, max.z,  min.x, min.y, max.z,
        min.x, min.y, max.z,  min.x, min.y, min.z,

        min.x, max.y, min.z,  max.x, max.y, min.z,
        max.x, max.y, min.z,  max.x, max.y, max.z,
        max.x, max.y, max.z,  min.x, max.y, max.z,
        min.x, max.y, max.z,  min.x, max.y, min.z,

        min.x, min.y, min.z,  min.x, max.y, min.z,
        max.x, min.y, min.z,  max.x, max.y, min.z,
        max.x, min.y, max.z,  max.x, max.y, max.z,
        min.x, min.y, max.z,  min.x, max.y, max.z
    };

    glUseProgram(m_shaderProgram);

    int projectionLoc =
        glGetUniformLocation(m_shaderProgram, "uProjection");

    int viewLoc =
        glGetUniformLocation(m_shaderProgram, "uView");

    glUniformMatrix4fv(
        projectionLoc,
        1,
        GL_FALSE,
        &projection[0][0]
    );

    glUniformMatrix4fv(
        viewLoc,
        1,
        GL_FALSE,
        &view[0][0]
    );

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    glBufferSubData(
        GL_ARRAY_BUFFER,
        0,
        sizeof(vertices),
        vertices
    );

    glDisable(GL_DEPTH_TEST);

    glDrawArrays(GL_LINES, 0, 24);

    glEnable(GL_DEPTH_TEST);
}