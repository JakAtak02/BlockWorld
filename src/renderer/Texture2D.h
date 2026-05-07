#pragma once

#include <string>
#include <vector>

class Texture2D
{
public:
    Texture2D();
    ~Texture2D();

    bool loadArrayFromFiles(const std::vector<std::string>& paths);
    void bind(unsigned int slot = 0) const;

private:
    unsigned int m_textureId = 0;
    int m_width = 0;
    int m_height = 0;
    int m_layers = 0;
};