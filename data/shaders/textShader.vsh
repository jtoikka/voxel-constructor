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

    UV = uv / 0xFFFF;
}
