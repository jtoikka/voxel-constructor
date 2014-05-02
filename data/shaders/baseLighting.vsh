#version 330

layout (location = 0) in vec4 position;
layout (location = 1) in vec2 uv;

smooth out vec2 UV;

smooth out vec3 viewRay;

flat out vec4 lightDir;
flat out vec4 lightPos;

layout (std140) uniform globalMatrices {
    mat4 cameraToClipMatrix;
    mat4 worldToCameraMatrix;
    mat4 invCameraToClipMatrix;
};

void main() {
    gl_Position = position;
    UV = uv;

    viewRay = vec3(-(UV.x * 2.0f - 1.0f) / cameraToClipMatrix[0].x,
    			   -(UV.y * 2.0f - 1.0f) / cameraToClipMatrix[1].y,
    					 1.0f);

    lightPos = vec4(100.0f, 100.0f, 100.0f, 1.0f);
    lightPos = worldToCameraMatrix * lightPos;

    lightDir = normalize(vec4(0.5f, 0.7f, -1.0f, 0.0f));
    lightDir = worldToCameraMatrix * lightDir;

}