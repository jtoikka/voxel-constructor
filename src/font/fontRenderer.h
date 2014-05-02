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
#ifndef FONTRENDERER_H
#define FONTRENDERER_H

#include "../../lib/glm/gtc/type_ptr.hpp"
#include "../../lib/freetype-gl-master/vertex-buffer.h"
#include "../../lib/freetype-gl-master/texture-atlas.h"
#include "../../lib/freetype-gl-master/texture-font.h"

#include "../render/shaderManager.h"

#include <boost/unordered_map.hpp>

class FontRenderer {
public:
    FontRenderer(ShaderManager* shaderManager, const std::string& fileName, int fontSize);

    unsigned char* make_distance_map(unsigned char *img,
                                     unsigned int width, unsigned int height);

    glm::vec4 addText(const std::wstring& text);

    void drawText(std::wstring text, glm::vec2 location, glm::vec2 scale);

    glm::vec4 getTextBoundingBox(std::wstring text);

    void removeText(std::wstring text);

private:
    typedef struct {
        GLuint vertexArrayObject;
        GLuint vertexBufferObject;
        GLuint uvBufferObject;
        GLuint indexBufferObject;

        glm::vec4 boundingBox;
    } StringBuffer;

    boost::unordered_map<std::wstring, StringBuffer*> buffers;
    texture_atlas_t* atlas = 0;

    texture_font_t* font = 0;

    glm::mat3x2 translationScaleMatrix;
    GLuint translationScaleUnif;

    ShaderManager* shaderManager;
};

#endif