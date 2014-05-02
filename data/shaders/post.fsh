#version 330

in vec2 UV;
flat in vec2 texelSize;

out vec4 outputColour;

layout (std140) uniform windowScale {
    vec2 winScale;
};

uniform sampler2D diffuseTex;

float fxaaLuma(vec3 rgb) {
	return rgb.g * (0.587f / 0.299f) + rgb.r; // Experiment with values 
}

vec4 textureOffset(sampler2D tex, vec2 offset) {
	return texture(tex, UV + (offset * texelSize));
}

const float FXAA_EDGE_THRESHOLD = 1.0f/8.0f;
const float FXAA_EDGE_THRESHOLD_MIN = 1.0f/24.0f;

const float FXAA_SUBPIX_TRIM = 1.0f/4.0f;
const float FXAA_SUBPIX_TRIM_SCALE = 1.0f/(1.0f - FXAA_SUBPIX_TRIM);
const float FXAA_SUBPIX_CAP = 0.75f;

const int FXAA_SEARCH_STEPS = 16;
const float FXAA_SEARCH_THRESHOLD = 0.25f;

#define FXAA_SUBPIX_FASTER 0

vec3 fxaa() {
	vec3 rgbN = textureOffset(diffuseTex, vec2( 0, -1)).xyz;
	vec3 rgbE = textureOffset(diffuseTex, vec2( 1,  0)).xyz;
	vec3 rgbS = textureOffset(diffuseTex, vec2( 0,  1)).xyz;
	vec3 rgbW = textureOffset(diffuseTex, vec2(-1,  0)).xyz;
	vec3 rgbM = textureOffset(diffuseTex, vec2( 0,  0)).xyz;

	float lumaN = fxaaLuma(rgbN);
	float lumaE = fxaaLuma(rgbE);
	float lumaS = fxaaLuma(rgbS);
	float lumaW = fxaaLuma(rgbW);
	float lumaM = fxaaLuma(rgbM);

	// Local contrast check
	float lumaMin = min(lumaM, min(min(lumaE, lumaW), min(lumaN, lumaS)));
	float lumaMax = max(lumaM, max(max(lumaE, lumaW), max(lumaN, lumaS)));

	float range = lumaMax - lumaMin;

	if (range < max(FXAA_EDGE_THRESHOLD_MIN, lumaMax * FXAA_EDGE_THRESHOLD)) {
		return rgbM;
	}

	// Sup-pixel aliasing test
	float lumaL = (lumaN + lumaE + lumaS + lumaW + lumaM) * 0.25f;
	float rangeL = abs(lumaL - lumaM);
	float blendL = max(0.0f, (rangeL / range) - FXAA_SUBPIX_TRIM) 
				 * FXAA_SUBPIX_TRIM_SCALE;
	blendL = min(FXAA_SUBPIX_CAP, blendL);
	#if FXAA_SUBPIX_FASTER
		vec3 rgbL = (rgbN + rgbE + rgbS + rgbW + rgbM) * vec3(0.2f);
	#else
		vec3 rgbL = rgbN + rgbE + rgbS + rgbW + rgbM;
	#endif

	vec3 rgbNE = textureOffset(diffuseTex, vec2(-1,  1)).xyz;
	vec3 rgbSE = textureOffset(diffuseTex, vec2( 1,  1)).xyz;
	vec3 rgbSW = textureOffset(diffuseTex, vec2( 1, -1)).xyz;
	vec3 rgbNW = textureOffset(diffuseTex, vec2(-1, -1)).xyz;

	#if (FXAA_SUBPIX_FASTER == 0)
		rgbL += (rgbNE + rgbSE + rgbSW + rgbNW);
		rgbL *= (1.0f / 9.0f);
	#endif

	float lumaNE = fxaaLuma(rgbNE);
	float lumaSE = fxaaLuma(rgbSE);
	float lumaSW = fxaaLuma(rgbSW);
	float lumaNW = fxaaLuma(rgbNW);

	//Vertical/Horizontal edge test
	float edgeVert = 
		abs((0.25f * lumaNW) + (-0.50f * lumaN) + (0.25f * lumaNE))
      + abs((0.50f * lumaW ) + (-1.00f * lumaM) + (0.50f * lumaE ))
      + abs((0.25f * lumaSW) + (-0.50f * lumaS) + (0.25f * lumaSE));
    float edgeHorz = 
    	abs((0.25f * lumaNW) + (-0.50f * lumaW) + (0.25f * lumaSW))
      + abs((0.50f * lumaN ) + (-1.00f * lumaM) + (0.50f * lumaS ))
      + abs((0.25f * lumaNE) + (-0.50f * lumaE) + (0.25f * lumaSE));

    bool horzSpan = edgeHorz >= edgeVert;

    float lengthSign = horzSpan ? -texelSize.y : -texelSize.x;

    if (!horzSpan) lumaN = lumaW;
    if (!horzSpan) lumaS = lumaE;

    float gradientN = abs(lumaN - lumaM);
    float gradientS = abs(lumaS - lumaM);

    lumaN = (lumaN + lumaM) * 0.5;
    lumaS = (lumaS + lumaM) * 0.5;


    bool pairN = gradientN > gradientS;
    if (!pairN) lumaN = lumaS;
    if (!pairN) gradientN = gradientS;
    if (!pairN) lengthSign *= -1.0f;

    vec2 posN;
    posN.x = UV.x + (horzSpan ? 0.0f : lengthSign * 0.5f);
    posN.y = UV.y + (horzSpan ? lengthSign * 0.5f : 0.0f);

    gradientN *= FXAA_SEARCH_THRESHOLD;

    vec2 posP = posN;
    vec2 offNP = horzSpan ? vec2(texelSize.x, 0.0f) : vec2(0.0f, texelSize.y);

    float lumaEndN = lumaN;
    float lumaEndP = lumaN;

    bool doneN = false;
    bool doneP = false;

    posN -= offNP;
    posP += offNP;

    // End-of-edge search
    for (int i = 0; i < FXAA_SEARCH_STEPS; i++) {
    	if (!doneN) lumaEndN = fxaaLuma(textureOffset(diffuseTex, posN).xyz);
    	if (!doneP) lumaEndP = fxaaLuma(textureOffset(diffuseTex, posP).xyz);

    	doneN = doneN || (abs(lumaEndN - lumaN) >= gradientN);
    	doneP = doneP || (abs(lumaEndP - lumaN) >= gradientN);
    	if (doneN && doneP) break;
    	if (!doneN) posN -= offNP;
    	if (!doneP) posP += offNP;
    }

    float dstN = horzSpan ? UV.x - posN.x : UV.y - posN.y;
    float dstP = horzSpan ? posP.x - UV.x : posP.y - UV.y;
    bool directionN = dstN < dstP;

    lumaEndN = directionN ? lumaEndN : lumaEndP;

    if (((lumaM - lumaN) < 0.0f) == ((lumaEndN - lumaN < 0.0f)))
    	lengthSign = 0.0f;

	float spanLength = (dstP + dstN);
	dstN = directionN ? dstN : dstP;
	float subPixelOffset = (0.5f + (dstN * (-1.0f/spanLength))) * lengthSign;

	vec3 rgbF = textureOffset(diffuseTex, 
							   vec2(horzSpan ? 0.0f : subPixelOffset, 
									horzSpan ? subPixelOffset : 0.0f)).xyz;

	return mix(rgbL, rgbF, blendL);
}

void main() {
    //vec4 diffuse = texture(diffuseTex, UV);
    vec3 texelAA = fxaa();
    outputColour = vec4(texelAA, 1.0f);
}