#version 330

layout (location = 0) in vec2 position;
layout (location = 1) in vec4 colour;

layout (std140) uniform windowScale {
    vec2 winScale;
};

uniform mat3x2 translationScaleMatrix;

smooth out vec2 UV;

flat out vec4 vertColour;

void main() {
	vertColour = colour;

	vec2 relativePosition = position / (winScale * 0.5f) - vec2(1.0f, 1.0f);
	gl_Position = vec4(relativePosition.x, -relativePosition.y, 0.0f, 1.0f);
}