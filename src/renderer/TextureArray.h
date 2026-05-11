#pragma once

#include <string>
#include <vector>

class TextureArray
{
public:
    TextureArray();
    ~TextureArray();

    bool loadArrayFromFiles(
        const std::vector<std::string>& paths
    );

    void bind(unsigned int slot = 0) const;

private:
    unsigned int m_textureId = 0;

    int m_width = 0;
    int m_height = 0;
    int m_layers = 0;
};