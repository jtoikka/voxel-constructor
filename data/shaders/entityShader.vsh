#version 330

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 colourID;
layout (location = 3) in vec3 uvw;

smooth out vec4 normalToCam;
flat out vec3 colID;

smooth out vec3 viewPosition;
flat out vec3 texCoord;

const float MAXDISTANCE = 1.0f;

layout (std140) uniform globalMatrices {
    mat4 cameraToClipMatrix;
    mat4 modelToCameraMatrix;
    mat4 invCameraToClipMatrix;
};

void main() {
    vec4 positionCam = modelToCameraMatrix * vec4(position, 1.0f);
    gl_Position = cameraToClipMatrix * positionCam;

    normalToCam = modelToCameraMatrix * vec4(normal, 0.0f);
    colID = colourID;
    viewPosition = positionCam.xyz;
    texCoord = uvw - (normal / 1000.0f);
}