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

in vec2 UV;
flat in vec2 texelSize;

layout (location = 0) out vec4 outputColour;

layout (std140) uniform windowScale {
    vec2 winScale;
};

uniform sampler2D diffuseTex;
uniform sampler2D gaussianTex;
uniform sampler1D closenessTex;

const int blurSize = 5;

float boxBlur() {
	vec2 limit = (float(-blurSize) / 2 + 0.5) * vec2(1.0f);
	float sum = 0.0f;
	float baseIntensity = texture(diffuseTex, UV).a;
	if (baseIntensity > 0.99f || baseIntensity < 0.01f) { // Early exit if white or black
		return baseIntensity;
	}
	for (int x = 0; x < blurSize; x++) {
		for (int y = 0; y < blurSize; y++) {
			vec2 offset = (limit + vec2(x, y)) * texelSize;
			float offsetIntensity = texture(diffuseTex, UV + offset).a;
			sum += offsetIntensity;
		}
	}
	return sum / 25;
}


// To do: optimize blurring (multi-step cross-blur)
float bilateralBlur() {
	vec2 limit = (float(-blurSize) / 2 + 0.5) * vec2(1.0f);
	float sum = 0.0f;
	float baseIntensity = texture(diffuseTex, UV).a;
	if (baseIntensity > 0.99f || baseIntensity < 0.01f) { // Early exit if white or black
		return baseIntensity;
	}
	for (int x = 0; x < blurSize; x++) {
		for (int y = 0; y < blurSize; y++) {
			vec2 offset = (limit + vec2(x, y)) * texelSize;
			float gaussianValue = texture(gaussianTex, vec2(x, y) / float(blurSize)).r;
			float offsetIntensity = texture(diffuseTex, UV + offset).a;
			float intensityDiff = offsetIntensity - baseIntensity;
			float closeness = texture(closenessTex, abs(intensityDiff)).r;
			sum += (intensityDiff * closeness + baseIntensity) * gaussianValue;
		}
	}
	return sum;
}

float luma(vec3 rgb) {
	return rgb.g * 0.7152f + rgb.r * 0.2198f;
}

float exposure(vec4 diffuse, float exposure) {
	float luminance = luma(diffuse.rgb);

	float brightness = 1.0f - (exp((exposure) * -luma(diffuse.xyz)));

	return brightness;
}

void main() {
    vec4 diffuse = texture(diffuseTex, UV);
    if (diffuse.a == 0.0f) {
    	outputColour = vec4(57.0f / 255.0f, 57.0f / 255.0f, 57.0f / 255.0f, 0.0f);
    	return;
    }
    //float blurredOcclusion = bilateralBlur();
    float blurredOcclusion = boxBlur();
    blurredOcclusion = blurredOcclusion > 1.0f ? 1.0f : blurredOcclusion;
    blurredOcclusion = pow(blurredOcclusion, 5.0f);

    vec4 finalColour = diffuse * blurredOcclusion;

    const float expo = 4.0f;
    float luminance = exposure(finalColour, expo);

    finalColour = luminance * finalColour;

    finalColour = pow(finalColour, vec4(1.0f/2.2f)); // Gamma correction

    outputColour = finalColour;  
}