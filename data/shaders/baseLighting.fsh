#version 330

#ifndef OCCLUSION
    #define OCCLUSION 1
#endif

in vec2 UV;

in vec3 viewRay;

flat in vec4 lightDir;

layout (location = 0) out vec4 outputColour;

uniform sampler2D normalTex;
uniform sampler2D diffuseTex;
uniform sampler2D noiseTex;

layout (std140) uniform windowScale {
    vec2 winScale;
};

layout (std140) uniform globalMatrices {
    mat4 cameraToClipMatrix;
    mat4 worldToCameraMatrix;
    mat4 invCameraToClipMatrix;
};

const int kernelSize = 64; // Adjusts quality of SSAO

uniform vec3 kernel[kernelSize];

const float uRadius = 6.08f;

const float specIntensity = 0.4f;

float getOcclusion() {
    vec3 origin = viewRay * texture(diffuseTex, UV).a;
    vec3 normal = texture(normalTex, UV).xyz;

    vec3 rvec = texture(noiseTex, gl_FragCoord.xy * 0.25f).xyz * 2.0f - 1.0f;
    vec3 tangent = normalize(rvec - normal * dot(rvec, normal));
    vec3 bitangent = cross(normal, tangent);

    mat3 tbn = mat3(tangent, bitangent, normal);

    float occlusion = 0.0f;
    for (int i = 0; i < kernelSize; i++) {
        // Get sample location
        vec3 sample = tbn * kernel[i];

        sample = sample * uRadius + origin;

        // Project sample position
        vec4 offset = vec4(sample, 1.0f);
        offset = cameraToClipMatrix * offset;
        offset.xy /= offset.w;
        offset.xy = offset.xy * 0.5f + 0.5f;

        // Get sample depth
        float sampleDepth = texture(diffuseTex, offset.xy).a;
        // This will eventually be removed
        if (sampleDepth == 0.0f) {
            occlusion += 1.0f;
            continue;
        }

        float diff = sample.z - sampleDepth;
        occlusion += ((diff >= 0.0f) ? 1.0f : 0.0);
    }

    occlusion = (occlusion / kernelSize);
    return occlusion;
}

vec4 calculateSpecular(vec4 lightDirection, vec4 normal, float intensity) {
    vec3 halfAngle = normalize(lightDirection.xyz + viewRay);

    float blinnTerm = dot(normal.xyz, halfAngle);
    blinnTerm = clamp(blinnTerm, 0.0f, 1.0f);
    blinnTerm = pow(blinnTerm, 180.0f);

    float specIntensity = intensity * blinnTerm;

    vec4 specColour = specIntensity * vec4(1.0f, 1.0f, 1.0f, 1.0f);
    return specColour;
}

float roughness(vec4 lightDirection, vec4 normal) {
    float roughness = 0.6f;
    vec4 viewDir = normalize(vec4(viewRay, 0.0f));

    float nDotL = dot(lightDirection.xyz, normal.xyz);
    float nDotV = dot(viewDir.xyz, normal.xyz);

    float angleLN = acos(nDotL);
    float angleVN = acos(nDotV);

    float alpha = max(angleLN, angleVN);
    float beta = min(angleLN, angleVN);
    float gamma = dot(viewDir.xyz - normal.xyz * nDotV, 
                      lightDirection.xyz - normal.xyz * nDotL);

    float rSquared = roughness * roughness;

    float A = 1.0f - 0.5f * rSquared / (rSquared + 0.57f);
    float B = 0.45f * rSquared / (rSquared + 0.09f);
    float C = sin(alpha) / tan(beta);

    float L1 = max(0.0f, nDotL) * (A + B * max(0.0f, gamma) * C);

    return L1;

}

void main() {
    vec4 diffuse = texture(diffuseTex, UV);
    if (diffuse.a == 0.0f) {
        outputColour = vec4(0.2f, 0.2f, 0.2f, 1.0f);
        return;
    }

    diffuse.a = 1.0f;
    vec4 normal = texture(normalTex, UV);

    float roughness = roughness(lightDir, normal);

    vec4 finalColour = diffuse * roughness * 1.8f;

    vec4 specColour = calculateSpecular(lightDir, normal, specIntensity);

    float upness = (normal.y + 1.0f / 2.0f);
    vec4 ambientColour = upness 
                       * vec4(135.0f/255.0f, 206.0f/255.0f, 235.0f/255.0f, 0.0f)
                       + (1.0f - upness)
                       * vec4(122.0f/255.0f, 85.0f/255.0f, 43.0f/255.0f, 0.0f);

    ambientColour *= 0.3f * diffuse;

    float occlusion = 1.0f;
    #if OCCLUSION
        occlusion = getOcclusion();
    #endif

    finalColour += specColour + ambientColour;

    finalColour.a = occlusion;

    outputColour = finalColour;
}