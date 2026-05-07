#include "renderer/Texture2D.h"

#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>

Texture2D::Texture2D()
{
    glGenTextures(1, &m_textureId);
}

Texture2D::~Texture2D()
{
    if (m_textureId != 0)
    {
        glDeleteTextures(1, &m_textureId);
    }
}

bool Texture2D::loadArrayFromFiles(const std::vector<std::string>& paths)
{
    if (paths.empty())
    {
        std::cout << "Texture array path list is empty." << std::endl;
        return false;
    }

    stbi_set_flip_vertically_on_load(true);

    glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureId);

    for (size_t layer = 0; layer < paths.size(); layer++)
    {
        int width = 0;
        int height = 0;
        int channels = 0;

        unsigned char* data = stbi_load(
            paths[layer].c_str(),
            &width,
            &height,
            &channels,
            4
        );

        if (!data)
        {
            std::cout << "Failed to load texture: " << paths[layer] << std::endl;
            return false;
        }

        if (layer == 0)
        {
            m_width = width;
            m_height = height;
            m_layers = static_cast<int>(paths.size());

            glTexImage3D(
                GL_TEXTURE_2D_ARRAY,
                0,
                GL_RGBA8,
                m_width,
                m_height,
                m_layers,
                0,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                nullptr
            );
        }
        else
        {
            if (width != m_width || height != m_height)
            {
                std::cout << "Texture array size mismatch: " << paths[layer] << std::endl;
                std::cout << "All block textures must be the same size." << std::endl;
                stbi_image_free(data);
                return false;
            }
        }

        glTexSubImage3D(
            GL_TEXTURE_2D_ARRAY,
            0,
            0,
            0,
            static_cast<GLint>(layer),
            m_width,
            m_height,
            1,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            data
        );

        stbi_image_free(data);

        std::cout << "Loaded texture array layer " << layer << ": " << paths[layer] << std::endl;
    }

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    return true;
}

void Texture2D::bind(unsigned int slot) const
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureId);
}