#version 460 core

in vec2 vTexCoord;
flat in float vTextureIndex;

uniform sampler2DArray uTextureArray;

out vec4 FragColor;

void main()
{
    FragColor =
        texture(
            uTextureArray,
            vec3(vTexCoord, vTextureIndex)
        );
}