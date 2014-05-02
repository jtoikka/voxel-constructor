#version 330

in vec2 UV;
in vec2 position;
//in vec4 fontColour;

out vec4 outputColour;

uniform sampler2D diffuseTexture;

uniform vec4 colour;

void main() {
	vec4 diffuse = texture(diffuseTexture, UV);

	outputColour = vec4(0.9f, 0.9f, 0.9f, diffuse.r);
}