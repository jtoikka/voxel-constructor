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
#include "shaderManager.h"

#include "../utility.h"

#include <iostream>
#include <algorithm>

ShaderManager::ShaderManager() {
    GLenum err;
	initializePrograms();
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error: " << err << std::endl;
        std::cerr << "Initialized programs?" << std::endl;
    }

	bufferUniformBlocks();
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error: " << err << std::endl;
        std::cerr << "Hrmm..." << std::endl;
    }
}

ShaderManager::~ShaderManager() {
    glDeleteProgram(entityShader);
    glDeleteProgram(baseLightingShader);
    glDeleteProgram(shader2D);
    glDeleteProgram(textShader);
}

void ShaderManager::bufferUniformBlocks() {
    glGenBuffers(1, &globalMatricesUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, globalMatricesUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 3, NULL, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &windowScaleUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, windowScaleUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 2, NULL, GL_STREAM_DRAW);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferRange(GL_UNIFORM_BUFFER, 0, globalMatricesUBO, 0, sizeof(glm::mat4) * 3);
    glBindBufferRange(GL_UNIFORM_BUFFER, 1, windowScaleUBO, 0, sizeof(float) * 2);
}

GLuint ShaderManager::createShader(GLenum eShaderType, const std::string& strShaderFile) {
    GLuint shader = glCreateShader(eShaderType);
    const std::string& strFileData = Utility::textFileRead(strShaderFile);
    const char* cStrFileData = strFileData.c_str();
    glShaderSource(shader, 1, &cStrFileData, NULL);

    glCompileShader(shader);

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        GLint infoLogLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

        GLchar *strInfoLog = new GLchar[infoLogLength + 1];
        glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);

        const char *strShaderType = NULL;
        switch (eShaderType) {
            case GL_VERTEX_SHADER: strShaderType = "vertex"; break;
            case GL_GEOMETRY_SHADER: strShaderType = "geometry"; break;
            case GL_FRAGMENT_SHADER: strShaderType = "fragment"; break;
        }

        fprintf(stderr, "Compile failure in %s shader:\n%s\n", strShaderType, strInfoLog);
        delete[] strInfoLog;
    }

    return shader;
}

GLuint ShaderManager::createProgram(const std::vector<GLuint> &shaderList) {
    GLuint program = glCreateProgram();

    for (size_t i = 0; i < shaderList.size(); i++) 
        glAttachShader(program, shaderList[i]);

    glLinkProgram(program);

    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        GLint infoLogLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

        GLchar *strInfoLog = new GLchar[infoLogLength + 1];
        glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);
        fprintf(stderr, "Linker failure %s\n", strInfoLog);
        delete[] strInfoLog;
    }

    for(size_t i = 0; i < shaderList.size(); i++)
        glDetachShader(program, shaderList[i]);

    return program;
}

void ShaderManager::initializePrograms() {
    std::vector<GLuint> shaderList;

    GLuint globalMatricesIndex;

    GLuint windowScaleIndex;

    //GLuint kernelIndex;

    GLuint textureToDraw;

    /* Initialize entity shader ----------------------------------------------*/
    GLenum err;

    shaderList.push_back(createShader(GL_VERTEX_SHADER, Utility::programDirectory + "data/shaders/entityShader.vsh"));
    shaderList.push_back(createShader(GL_FRAGMENT_SHADER, Utility::programDirectory + "data/shaders/entityShader.fsh"));

    entityShader = createProgram(shaderList);

    std::for_each(shaderList.begin(), shaderList.end(), glDeleteShader);
    shaderList.clear();

    globalMatricesIndex = glGetUniformBlockIndex(entityShader, "globalMatrices");

    glUseProgram(entityShader);
    textureToDraw = glGetUniformLocation(entityShader, "diffuseTex");
    glUniform1i(textureToDraw, 0);

    glUseProgram(0);

    glUniformBlockBinding(entityShader, globalMatricesIndex, 0);

    /* Initialize base lighting shader ---------------------------------------*/

    shaderList.push_back(createShader(GL_VERTEX_SHADER, Utility::programDirectory + "data/shaders/baseLighting.vsh"));
    shaderList.push_back(createShader(GL_FRAGMENT_SHADER, Utility::programDirectory + "data/shaders/baseLighting.fsh"));

    baseLightingShader = createProgram(shaderList);
    std::for_each(shaderList.begin(), shaderList.end(), glDeleteShader);
    shaderList.clear();

    globalMatricesIndex = glGetUniformBlockIndex(baseLightingShader, "globalMatrices");
    glUniformBlockBinding(baseLightingShader, globalMatricesIndex, 0);

    windowScaleIndex = glGetUniformBlockIndex(baseLightingShader, "windowScale");
    glUniformBlockBinding(baseLightingShader, windowScaleIndex, 1);

    glUseProgram(baseLightingShader);
    textureToDraw = glGetUniformLocation(baseLightingShader, "normalTex");
    glUniform1i(textureToDraw, 0);

    textureToDraw = glGetUniformLocation(baseLightingShader, "diffuseTex");
    glUniform1i(textureToDraw, 1);

    textureToDraw = glGetUniformLocation(baseLightingShader, "colourIDTex");
    glUniform1i(textureToDraw, 2);

    textureToDraw = glGetUniformLocation(baseLightingShader, "noiseTex");
    glUniform1i(textureToDraw, 3);

    glUseProgram(0);

    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error: " << err << std::endl;
        std::cerr << "Tex to screen shader" << std::endl;
    }

    /* Initialize tex to screen shader ---------------------------------------*/

    shaderList.push_back(createShader(GL_VERTEX_SHADER, Utility::programDirectory + "data/shaders/occlusionBlur.vsh"));
    shaderList.push_back(createShader(GL_FRAGMENT_SHADER, Utility::programDirectory + "data/shaders/occlusionBlur.fsh"));

    occlusionBlurShader = createProgram(shaderList);
    std::for_each(shaderList.begin(), shaderList.end(), glDeleteShader);
    shaderList.clear();

    windowScaleIndex = glGetUniformBlockIndex(occlusionBlurShader, "windowScale");
    glUniformBlockBinding(occlusionBlurShader, windowScaleIndex, 1);

    glUseProgram(occlusionBlurShader);
    textureToDraw = glGetUniformLocation(occlusionBlurShader, "diffuseTex");
    glUniform1i(textureToDraw, 0);

    textureToDraw = glGetUniformLocation(occlusionBlurShader, "gaussianTex");
    glUniform1i(textureToDraw, 1);

    textureToDraw = glGetUniformLocation(occlusionBlurShader, "closenessTex");
    glUniform1i(textureToDraw, 2);

    glUseProgram(0);

    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error: " << err << std::endl;
        std::cerr << "Tex to screen shader" << std::endl;
    }

    /* Initialize post shader ------------------------------------------------*/

    shaderList.push_back(createShader(GL_VERTEX_SHADER, Utility::programDirectory + "data/shaders/post.vsh"));
    shaderList.push_back(createShader(GL_FRAGMENT_SHADER, Utility::programDirectory + "data/shaders/post.fsh"));

    postShader = createProgram(shaderList);
    std::for_each(shaderList.begin(), shaderList.end(), glDeleteShader);
    shaderList.clear();

    windowScaleIndex = glGetUniformBlockIndex(postShader, "windowScale");
    glUniformBlockBinding(postShader, windowScaleIndex, 1);

    glUseProgram(postShader);
    textureToDraw = glGetUniformLocation(postShader, "diffuseTex");
    glUniform1i(textureToDraw, 0);

    // textureToDraw = glGetUniformLocation(postShader, "gaussianTex");
    // glUniform1i(textureToDraw, 1);

    // textureToDraw = glGetUniformLocation(postShader, "closenessTex");
    // glUniform1i(textureToDraw, 2);

    glUseProgram(0);

    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error: " << err << std::endl;
        std::cerr << "Tex to screen shader" << std::endl;
    }

    /* Initialize 2D shader --------------------------------------------------*/

    shaderList.push_back(createShader(GL_VERTEX_SHADER, Utility::programDirectory + "data/shaders/2Dshader.vsh"));
    shaderList.push_back(createShader(GL_FRAGMENT_SHADER, Utility::programDirectory + "data/shaders/2Dshader.fsh"));

    shader2D = createProgram(shaderList);
    std::for_each(shaderList.begin(), shaderList.end(), glDeleteShader);
    shaderList.clear();

    windowScaleIndex = glGetUniformBlockIndex(shader2D, "windowScale");
    glUniformBlockBinding(shader2D, windowScaleIndex, 1);

    // textureToDraw = glGetUniformLocation(shader2D, "diffuseTexture");
    // glUniform1i(shader2D, 0);

    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error: " << err << std::endl;
        std::cerr << "2D shader" << std::endl;
    }

    /* Initialize text shader --------------------------------------------------*/

    shaderList.push_back(createShader(GL_VERTEX_SHADER, Utility::programDirectory + "data/shaders/textShader.vsh"));
    shaderList.push_back(createShader(GL_FRAGMENT_SHADER, Utility::programDirectory + "data/shaders/textShader.fsh"));

    textShader = createProgram(shaderList);
    std::for_each(shaderList.begin(), shaderList.end(), glDeleteShader);
    shaderList.clear();

    windowScaleIndex = glGetUniformBlockIndex(textShader, "windowScale");
    glUniformBlockBinding(textShader, windowScaleIndex, 1);

    // textureToDraw = glGetUniformLocation(textShader, "diffuseTexture");
    // glUniform1i(textShader, 0);

    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error: " << err << std::endl;
        std::cerr << "Text shader" << std::endl;
    }
}

const GLuint& ShaderManager::getEntityShader() {
	return entityShader;
}
const GLuint& ShaderManager::getBaseLightingShader() {
	return baseLightingShader;
}
const GLuint& ShaderManager::getOcclusionBlurShader() {
    return occlusionBlurShader;
}
const GLuint& ShaderManager::getPostShader() {
    return postShader;
}
const GLuint& ShaderManager::get2DShader() {
    return shader2D;
}
const GLuint& ShaderManager::getTextShader() {
    return textShader;
}
const GLuint& ShaderManager::getGlobalMatricesUBO() {
	return globalMatricesUBO;
}
const GLuint& ShaderManager::getWindowScaleUBO() {
    return windowScaleUBO;
}
const GLuint& ShaderManager::getKernelUBO() {
    return kernelUBO;
}