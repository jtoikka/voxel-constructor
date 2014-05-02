#version 330

layout (location = 0) in vec4 position;
layout (location = 1) in vec2 uv;

smooth out vec2 UV;

flat out vec2 texelSize;

uniform sampler2D diffuseTex;

void main() {
    gl_Position = position;
    UV = uv;
    texelSize = 1.0f / vec2(textureSize(diffuseTex, 0)); // Ensures texels are sampled at center
}