#version 460 core

in vec2 vTexCoord;
in vec3 vWorldPosition;
in vec3 vTintColor;
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

    float tintMask =
    max(
        max(textureColor.r, textureColor.g),
        textureColor.b
    );

tintMask =
    smoothstep(
        0.25,
        0.65,
        tintMask
    );

vec3 tintedTextureColor =
    mix(
        textureColor.rgb,
        textureColor.rgb * vTintColor,
        tintMask
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
            tintedTextureColor,
            uFogColor,
            fogAmount
        );

    FragColor =
        vec4(
            finalColor,
            textureColor.a
        );
}