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
#ifndef RENDER2D_H
#define RENDER2D_H

#include "shaderManager.h"

#include <boost/unordered_map.hpp>


#include <unordered_map>

#include "../font/fontRenderer.h"

class Render2D {
public: 
/**
  * Constructor, loads relevant textures and font.
  * 
  * @param shaderManager Pointer to shaderManager, used for accessing relevant 
  *                      shaders.
  */
    Render2D(ShaderManager* shaderManager, glm::vec2 dimensions);

    ~Render2D();

/**
  * Start rendering to 2D renderer.
  */
    void start();

/**
  * Finish rendering to 2D renderer.
  */
    void finish();

/**
  * Draw a rectangle.
  *
  * @param location The window location ((0, 0) is top left corner) of the 
  *                 rectangle (coordinates are rectangle's center). In pixels.
  * @param dimensions Width and height of the rectangle, in pixels.
  */
    void drawRectangle(glm::vec2 location, glm::vec2 dimensions, glm::vec4 colour);

    void drawText(std::wstring text, glm::vec2 location);

    GLuint getTexture(std::string textureName);

    glm::vec4 getTextBoundingBox(std::wstring text);

    void resize(int w, int h);

    void removeText(std::wstring text);

private:
    ShaderManager* shaderManager;  /**< Pointer to shader manager, for accessing relevant shaders */

    boost::unordered_map<std::string, GLuint> textures; /**< Unordered map of all loaded textures */

    GLuint quadVertexArray;
    GLuint quadVertexbuffer;

    glm::mat3x2 translationScaleMatrix;

    GLuint translationScaleUnif;
    GLuint colourUnif;

    glm::vec2 rendererDimensions;

    FontRenderer* fontReg16;

    typedef struct Quad {
        GLuint vertexArray;
        GLuint vertexBuffer;
        GLuint colourBuffer;
    } Quad;

    struct Vec2Hasher {
        size_t operator()(const glm::vec2 key) const {
            return (std::hash<float>()(key.x) ^ std::hash<float>()(key.y) << 1) >> 1;
        }
    };

    struct Vec4Hasher {
        size_t operator()(const glm::vec4 key) const {
            return (std::hash<float>()(key.x) 
                  ^ std::hash<float>()(key.y) 
                  ^ std::hash<float>()(key.z) 
                  ^ std::hash<float>()(key.w));
        }
    };

    typedef std::unordered_map<glm::vec2, 
                std::unordered_map<glm::vec2, 
                    std::unordered_map<glm::vec4, Quad, 
                    Vec4Hasher>, 
                Vec2Hasher>, 
            Vec2Hasher> QuadMap;

    QuadMap quadMap;

    bool checkQuad(glm::vec2 location, glm::vec2 dimensions, glm::vec4 colour);
/**
  * Load a texture, and store it in _textures_.
  * 
  * @param fileName Path name to texture, relative to program's directory.
  * @param textureName An identifier for the texture. (NOTE: make sure the name
  *                    is unique!)
  */
    void loadTexture(const std::string& fileName, std::string textureName);

/** 
  * Load font, currently does not accept any parameters, and loads the default
  */
    void loadFont();

    void setScale(glm::vec2 scale);

    void setTranslation(glm::vec2 translation);

    void genQuad(glm::vec2 dimensions);

    GLuint genQuad(glm::vec2 location, glm::vec2 dimensions, glm::vec4 colour);

    void clearQuadMap();
};

#endif