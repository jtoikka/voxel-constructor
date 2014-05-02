#version 330

smooth in vec4 normalToCam;
flat in vec3 colID;
smooth in vec3 viewPosition;
flat in vec3 texCoord;

layout (location = 1) out vec3 normalData;
layout (location = 2) out vec4 colourData;
layout (location = 3) out vec3 colourIDData;

uniform sampler3D diffuseTex;

void main() {
    vec4 diffuse = texture(diffuseTex, texCoord);
	colourData = vec4(diffuse.xyz, viewPosition.z);
	colourIDData = colID;
	normalData = normalize(normalToCam.xyz);
}