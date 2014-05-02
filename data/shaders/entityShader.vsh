/*==============================================================================
The MIT License (MIT)

Copyright (c) 2014 Juuso Toikka

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
==============================================================================*/
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