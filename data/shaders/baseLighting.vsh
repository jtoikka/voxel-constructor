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