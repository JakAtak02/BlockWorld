#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in float aTextureIndex;
layout (location = 3) in vec3 aTintColor;

uniform mat4 uProjection;
uniform mat4 uView;

out vec2 vTexCoord;
out vec3 vWorldPosition;
out vec3 vTintColor;
flat out float vTextureIndex;

void main()
{
    vTexCoord = aTexCoord;
    vWorldPosition = aPos;
    vTintColor = aTintColor;
    vTextureIndex = aTextureIndex;

    gl_Position =
        uProjection *
        uView *
        vec4(aPos, 1.0);
}