#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in float aTextureIndex;

uniform mat4 uProjection;
uniform mat4 uView;

out vec2 vTexCoord;
flat out float vTextureIndex;

void main()
{
    vTexCoord = aTexCoord;
    vTextureIndex = aTextureIndex;

    gl_Position =
        uProjection *
        uView *
        vec4(aPos, 1.0);
}