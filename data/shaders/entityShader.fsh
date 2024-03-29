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