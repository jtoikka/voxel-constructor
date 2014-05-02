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
#include "render2D.h"
#include "../lodepng.h"
#include "../utility.h"


Render2D::Render2D(ShaderManager* shaderManager, glm::vec2 dimensions) : 
    shaderManager(shaderManager), rendererDimensions(dimensions) {

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    genQuad(glm::vec2(1.0f, 1.0f));

    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error: " << err << std::endl;
        std::cerr << "Made a quad." << std::endl;
    }

    glUseProgram(shaderManager->get2DShader());

    colourUnif = glGetUniformLocation(shaderManager->get2DShader(), "colour");

    translationScaleUnif = glGetUniformLocation(shaderManager->get2DShader(), 
                                                "translationScaleMatrix");

    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error: " << err << std::endl;
        std::cerr << "Translation scale matrix..." << std::endl;
    }

    translationScaleMatrix[0][0] = 1.0f;
    translationScaleMatrix[1][1] = 1.0f;
    glUniformMatrix3x2fv(translationScaleUnif, 1, 
                         false, &translationScaleMatrix[0][0]);

    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error: " << err << std::endl;
        std::cerr << "Matrix uniform." << std::endl;
    }

    loadFont();

    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error: " << err << std::endl;
        std::cerr << "Loaded font" << std::endl;
    }
}

Render2D::~Render2D() {
    if (fontReg16 != nullptr) {
        delete fontReg16;
    }

    glDeleteVertexArrays(1, &quadVertexArray);
    glDeleteBuffers(1, &quadVertexbuffer);

    for (auto texturePair : textures) {
        glDeleteTextures(1, &texturePair.second);
    }
}

void Render2D::start() {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUseProgram(shaderManager->get2DShader());
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
}

void Render2D::finish() {

}

void Render2D::removeText(std::wstring text) {
    fontReg16->removeText(text);
}

glm::vec4 Render2D::getTextBoundingBox(std::wstring text) {
    return fontReg16->getTextBoundingBox(text);
}

void Render2D::drawRectangle(glm::vec2 location, 
                             glm::vec2 dimensions, glm::vec4 colour) {
    GLuint vao;
    if (checkQuad(location, dimensions, colour)) {
        vao = quadMap.find(location)->second.find(dimensions)
                   ->second.find(colour)->second.vertexArray;
    } else {
        vao = genQuad(location, dimensions, colour);
    }

    glUseProgram(shaderManager->get2DShader());

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glUseProgram(0);
}

bool Render2D::checkQuad(glm::vec2 location, 
                         glm::vec2 dimensions, glm::vec4 colour) {
    auto it = quadMap.find(location);
    if (it != quadMap.end()) {
        auto it2 = it->second.find(dimensions);
        if (it2 != it->second.end()) {
            if (it2->second.find(colour) != it2->second.end()) {
                return true;
            }
        }
    }
    return false;
}

GLuint Render2D::genQuad(glm::vec2 location, 
                         glm::vec2 dimensions, 
                         glm::vec4 colour) {
    Quad quad;
    glGenVertexArrays(1, &quad.vertexArray);
    glBindVertexArray(quad.vertexArray);

    if (colour.g == 1.0f) {
        std::cout << "!!!" << std::endl;
    }

    GLfloat quadVertexBufferData[] = {
        location.x,                location.y,
        location.x + dimensions.x, location.y,
        location.x,                location.y + dimensions.y,
        location.x,                location.y + dimensions.y,
        location.x + dimensions.x, location.y,
        location.x + dimensions.x, location.y + dimensions.y
    };

    GLfloat colourData[] = {
        colour.r, colour.g, colour.b, colour.a,
        colour.r, colour.g, colour.b, colour.a,
        colour.r, colour.g, colour.b, colour.a,
        colour.r, colour.g, colour.b, colour.a,
        colour.r, colour.g, colour.b, colour.a,
        colour.r, colour.g, colour.b, colour.a
    };

    glGenBuffers(1, &quad.vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, quad.vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertexBufferData),
                 quadVertexBufferData, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glGenBuffers(1, &quad.colourBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, quad.colourBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colourData), 
                 colourData, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);

    glBindVertexArray(0);

    quadMap[location][dimensions][colour] = quad;

    return quad.vertexArray;
}

void Render2D::genQuad(glm::vec2 dimensions) {
    glGenVertexArrays(1, &quadVertexArray);
    glBindVertexArray(quadVertexArray);

    static const GLfloat quadVertexBufferData[] = {
        -0.5f * dimensions.x, -0.5f * dimensions.y,
         0.5f * dimensions.x, -0.5f * dimensions.y,
        -0.5f * dimensions.x,  0.5f * dimensions.y,
        -0.5f * dimensions.x,  0.5f * dimensions.y,
         0.5f * dimensions.x, -0.5f * dimensions.y,
         0.5f * dimensions.x,  0.5f * dimensions.y
    };

    glGenBuffers(1, &quadVertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, quadVertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertexBufferData), 
                 quadVertexBufferData, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindVertexArray(0);
}

void Render2D::loadTexture(const std::string& fileName, 
                           std::string textureName) {
    std::vector<unsigned char> image;
    unsigned width, height;
    unsigned error = lodepng::decode(image, width, height, fileName);

    if (error) std::cerr << "PNG decoder error " << error << ": " 
                         << lodepng_error_text(error) << std::endl;

    GLuint texID;

    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, 
                 GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    textures.insert(std::pair<std::string, GLuint>(textureName, texID));
}

GLuint Render2D::getTexture(std::string textureID) {
    auto iterator = textures.find(textureID);
    if (iterator != textures.end()) {
        return iterator->second;
    } else {
        std::cerr << "ERROR: Texture " << textureID 
                  << " not found." << std::endl;
        return 0;
    }
}

void Render2D::loadFont() {
    fontReg16 = new FontRenderer(shaderManager, 
                                 Utility::programDirectory 
                               + "data/font/LatoOFL/TTF/Lato-Reg.ttf", 16);
}

void Render2D::drawText(std::wstring text, glm::vec2 location) {
    fontReg16->drawText(text, location, glm::vec2(1.0f, 1.0f));
}

void Render2D::setScale(glm::vec2 scale) {
    translationScaleMatrix[0][0] = scale.x;
    translationScaleMatrix[1][1] = scale.y;
}

void Render2D::setTranslation(glm::vec2 translation) {
    translationScaleMatrix[2] = translation;
}

void Render2D::resize(int w, int h) {
    rendererDimensions = glm::vec2(w, h);
    clearQuadMap();
}

void Render2D::clearQuadMap() {
    for (auto pair : quadMap) {
        for (auto pair2 : pair.second) {
            for (auto pair3 : pair2.second) {
                glDeleteBuffers(1, &pair3.second.vertexBuffer);
                glDeleteBuffers(1, &pair3.second.colourBuffer);
                glDeleteVertexArrays(1, &pair3.second.vertexArray);
            }
            pair2.second.clear();
        }
        pair.second.clear();
    }
    quadMap.clear();
}

