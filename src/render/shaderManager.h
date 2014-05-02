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
#ifndef SHADERMANAGER_H
#define SHADERMANAGER_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <string>
// #include <algorithm>

#include "../../lib/glm/gtc/type_ptr.hpp"

class ShaderManager {
public:
/**
  * Constructor for the shader manager. The constructor takes no parameters, and
  * loads a predetermined set of shaders.
  */
    ShaderManager();

    ~ShaderManager();

// The following are simple getters
    const GLuint& getEntityShader();
    const GLuint& getBaseLightingShader();
    const GLuint& getOcclusionBlurShader();
    const GLuint& getPostShader();
    const GLuint& get2DShader();
    const GLuint& getTextShader();
    const GLuint& getGlobalMatricesUBO();
    const GLuint& getWindowScaleUBO();
    const GLuint& getKernelUBO();

private:
    GLuint entityShader;       /**< Shader program for rendering entities */
    GLuint baseLightingShader;  
    GLuint occlusionBlurShader; 
    GLuint postShader;
    GLuint shader2D;           /**< Shader program for rendering 2D assets (GUI, HUD) */
    GLuint textShader;

    GLuint globalMatricesUBO;
    GLuint windowScaleUBO;
    GLuint kernelUBO;

/**
  * Given a list of shaders (vertex, geometry, fragment), creates a shader
  * program, and frees the memory allocated to the used shaders.
  * 
  * @param shaderList The program is built using the shaders provided in this list
  * @return Returns the unique identifier for the created shader program
  */
    GLuint createProgram(const std::vector<GLuint> &shaderList);

/**
  * Creates a shader from a text file.
  * 
  * @param eShaderType The type of shader to create (GL_VERTEX_SHADER, 
  *                    GL_GEOMETRY_SHADER, or GL_FRAGMENT_SHADER).
  * @param strShaderFile The location of the text-based shader file to load, 
  *                      relative to the programs location.
  * @return Returns the unique identifier for the created shader
  */
    GLuint createShader(GLenum eShaderType, const std::string& strShaderFile);
/**
  * Helper function, this function initializes a predetermined set of shader
  * programs. Initialized shader programs are: entityShader, occlusionBlurShader,
  * and shader2D.
  */
    void initializePrograms();

/**
  * Helper function, initializes uniform blocks (matrices and window scale).
  */
    void bufferUniformBlocks();
};

#endif