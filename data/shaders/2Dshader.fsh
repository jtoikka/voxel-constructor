#version 330

layout (std140) uniform windowScale {
    vec2 winScale;
};

out vec4 outputColour;

flat in vec4 vertColour;

uniform vec4 colour;

void main() {
	outputColour = vertColour;
}