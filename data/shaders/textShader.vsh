#version 330

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 uv;

layout (std140) uniform windowScale {
    vec2 winScale;
};

uniform mat3x2 translationScaleMatrix;

uniform vec4 Color;

out vec2 UV;

void main() {
	vec2 relativePosition = translationScaleMatrix * vec3(position, 1.0f);
	relativePosition = relativePosition / (winScale / 2) - vec2(1.0f, 1.0f);
	gl_Position = vec4(relativePosition.x, -relativePosition.y, 0.0f, 1.0f);

    UV = uv / 0xFFFF; // Convert from integer to floating point values
}
