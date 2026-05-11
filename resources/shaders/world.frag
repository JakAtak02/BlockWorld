#version 460 core

in vec2 vTexCoord;
in vec3 vWorldPosition;
flat in float vTextureIndex;

uniform sampler2DArray uTextureArray;

uniform vec3 uCameraPosition;
uniform vec3 uFogColor;
uniform float uFogStart;
uniform float uFogEnd;

out vec4 FragColor;

void main()
{
    vec4 textureColor =
        texture(
            uTextureArray,
            vec3(vTexCoord, vTextureIndex)
        );

    float distanceFromCamera =
        distance(
            vWorldPosition,
            uCameraPosition
        );

    float fogAmount =
        clamp(
            (distanceFromCamera - uFogStart) /
            (uFogEnd - uFogStart),
            0.0,
            1.0
        );

    vec3 finalColor =
        mix(
            textureColor.rgb,
            uFogColor,
            fogAmount
        );

    FragColor =
        vec4(
            finalColor,
            textureColor.a
        );
}