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

#include "fontRenderer.h"

#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <string>

#include "../../lib/freetype-gl-master/freetype-gl.h"
#include "../../lib/freetype-gl-master/font-manager.h"
#include "../../lib/freetype-gl-master/text-buffer.h"
#include "../../lib/freetype-gl-master/markup.h"
#include "../../lib/freetype-gl-master/mat4.h"
#include "edtaa3func.h"
#include "../utility.h"

#if defined(__APPLE__)
    #include <Glut/glut.h>
#elif defined(_WIN32) || defined(_WIN64)
    #include <GLUT/glut.h>
#elif defined(__linux__)
    #include <GL/freeglut.h>
#else
    #include <GL/glut.h>
#endif

#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))

FontRenderer::FontRenderer(ShaderManager* shaderManager, const std::string& fileName, int fontSize) : shaderManager(shaderManager) {
    std::wstring cache = L" !\"#$%&'()*+,-./0123456789:;<=>?"
                         L"@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
                         L"`abcdefghijklmnopqrstuvwxyz{|}~";


    atlas = texture_atlas_new(512, 512, 1);
    font = texture_font_new_from_file(atlas, fontSize, fileName.c_str());
    texture_font_load_glyphs(font, cache.c_str());

    // std::cout << "Generating distance map" << std::endl;
    // unsigned char* map = make_distance_map(atlas->data, atlas->width, atlas->height);

    // memcpy(atlas->data, map, atlas->width * atlas->height * sizeof(unsigned char));
    // free(map);
    texture_atlas_upload( atlas );
    
    if (font == nullptr) {
        std::cerr << "ERROR: Font not loaded." << std::endl;
    }

    translationScaleMatrix = glm::mat3x2(1.0f);
    translationScaleUnif = glGetUniformLocation(shaderManager->getTextShader(), "translationScaleMatrix");
}

void FontRenderer::drawText(std::wstring text, glm::vec2 location, glm::vec2 scale) {
    glUseProgram(shaderManager->getTextShader());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, atlas->id);

    auto bufferIt = buffers.find(text);

    if (bufferIt == buffers.end()) {
        addText(text);
        bufferIt = buffers.find(text);
    }
    auto stringBuffer = bufferIt->second;

    translationScaleMatrix[0][0] = scale.x;
    translationScaleMatrix[1][1] = scale.y;
    translationScaleMatrix[2] = location;
    glUniformMatrix3x2fv(translationScaleUnif, 1, false, &translationScaleMatrix[0][0]);

    glBindVertexArray(stringBuffer->vertexArrayObject);
    glDrawElements(GL_TRIANGLES, text.length() * 6 * 2, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void FontRenderer::removeText(std::wstring text) {
    auto it = buffers.find(text);
    if (it != buffers.end()) {
        delete it->second;
        buffers.erase(it);
    }
}

glm::vec4 FontRenderer::getTextBoundingBox(std::wstring text) {
    auto bufferIt = buffers.find(text);

    if (bufferIt == buffers.end()) {
        addText(text);
        bufferIt = buffers.find(text);
    }
    return bufferIt->second->boundingBox;
}

// Currently uses one VBO per attribute. More efficient with interleaved data?
glm::vec4 FontRenderer::addText(const std::wstring& text) {
    glm::vec4 boundingBox(0.0f);

    glm::vec2 pen(0.0f);

    StringBuffer* stringBuffer = new StringBuffer();

    std::vector<float> vertices;
    std::vector<unsigned short> texCoords;
    std::vector<GLuint> indices;

    GLuint indexShift = 0; // Increments the index values for each subsequent quad

    for (size_t i = 0; i < text.length(); i++) {
        const wchar_t currentChar = text.c_str()[i];
        texture_glyph_t* glyph = texture_font_get_glyph(font, currentChar);
        if (glyph != nullptr) {
            // Check if kerning need be applied
            int kerning = 0;
            if (i > 0) {
                const wchar_t previousChar = text.c_str()[i - 1];
                kerning = texture_glyph_get_kerning(glyph, previousChar);
            }
            pen.x += kerning;

            // (x0, y0) is the bottom left vertex
            float x0 = (int)(pen.x + glyph->offset_x);
            float y0 = (int)(pen.y - glyph->offset_y);
            float x1 = (int)(x0 + glyph->width);
            float y1 = (int)(y0 + glyph->height);
            // UVs
            unsigned short s0 = glyph->s0 * 0xFFFF;
            unsigned short t0 = glyph->t0 * 0xFFFF;
            unsigned short s1 = glyph->s1 * 0xFFFF;
            unsigned short t1 = glyph->t1 * 0xFFFF;

            pen.x += glyph->advance_x;

            // Discard empty characters (ie. space)
            if (x0 == x1 || y0 == y1) {
                continue;
            }

            vertices.insert(vertices.end(), {x0, y0,
                                             x0, y1,
                                             x1, y1,
                                             x1, y0});

            texCoords.insert(texCoords.end(), {s0, t0,
                                               s0, t1,
                                               s1, t1,
                                               s1, t0});

            indices.insert(indices.end(), {0 + indexShift, 1 + indexShift, 2 + indexShift,
                                           0 + indexShift, 2 + indexShift, 3 + indexShift});

            indexShift += 4;

            //std::cout << "x0: " << x0 << " y0: " << y0 << " x1: " << x1 << " y1: " << y1 << std::endl;

            if  (x0 < boundingBox.x)                      boundingBox.x = x0;
            if  (y1 < boundingBox.y)                      boundingBox.y = y1;
            if ((x1 - boundingBox.x) > boundingBox.z)     boundingBox.z = x1 - boundingBox.x;
            if (fabs(y0 - boundingBox.y) > boundingBox.w) boundingBox.w = fabs(y0 - boundingBox.y);
        }
    }

    // Generate VAO
    glGenVertexArrays(1, &stringBuffer->vertexArrayObject);
    glGenBuffers(1, &stringBuffer->vertexBufferObject);
    glGenBuffers(1, &stringBuffer->uvBufferObject);
    glGenBuffers(1, &stringBuffer->indexBufferObject);

    glBindVertexArray(stringBuffer->vertexArrayObject);

    // Buffer vertices
    glBindBuffer(GL_ARRAY_BUFFER, stringBuffer->vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    // UVs
    glBindBuffer(GL_ARRAY_BUFFER, stringBuffer->uvBufferObject);
    glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(GL_UNSIGNED_SHORT), &texCoords[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_UNSIGNED_SHORT, GL_FALSE, 0, 0);

    // Indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, stringBuffer->indexBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

    // Finish up
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    stringBuffer->boundingBox = boundingBox;

    buffers.insert(std::pair<std::wstring, StringBuffer*>(text, stringBuffer));

    return boundingBox;
}

// Taken from freetype-gl demo
unsigned char* FontRenderer::make_distance_map(unsigned char *img,
                                               unsigned int width, unsigned int height)
{
    short * xdist = (short *)  malloc( width * height * sizeof(short) );
    short * ydist = (short *)  malloc( width * height * sizeof(short) );
    double * gx   = (double *) calloc( width * height, sizeof(double) );
    double * gy      = (double *) calloc( width * height, sizeof(double) );
    double * data    = (double *) calloc( width * height, sizeof(double) );
    double * outside = (double *) calloc( width * height, sizeof(double) );
    double * inside  = (double *) calloc( width * height, sizeof(double) );
    int i;

    // Convert img into double (data)
    double img_min = 255, img_max = -255;
    for( i=0; i<width*height; ++i)
    {
        double v = img[i];
        data[i] = v;
        if (v > img_max) img_max = v;
        if (v < img_min) img_min = v;
    }
    // Rescale image levels between 0 and 1
    for( i=0; i<width*height; ++i)
    {
        data[i] = (img[i]-img_min)/img_max;
    }

    // Compute outside = edtaa3(bitmap); % Transform background (0's)
    computegradient( data, height, width, gx, gy);
    edtaa3(data, gx, gy, height, width, xdist, ydist, outside);
    for( i=0; i<width*height; ++i)
        if( outside[i] < 0 )
            outside[i] = 0.0;

    // Compute inside = edtaa3(1-bitmap); % Transform foreground (1's)
    memset(gx, 0, sizeof(double)*width*height );
    memset(gy, 0, sizeof(double)*width*height );
    for( i=0; i<width*height; ++i)
        data[i] = 1 - data[i];
    computegradient( data, height, width, gx, gy);
    edtaa3(data, gx, gy, height, width, xdist, ydist, inside);
    for( i=0; i<width*height; ++i)
        if( inside[i] < 0 )
            inside[i] = 0.0;

    // distmap = outside - inside; % Bipolar distance field
    unsigned char *out = (unsigned char *) malloc( width * height * sizeof(unsigned char) );
    std::cout << "Width: " << width << "height: " << height << std::endl;
    for( i=0; i<width*height; ++i)
    {
        outside[i] -= inside[i];
        outside[i] = 128+outside[i]*16;
        if( outside[i] < 0 ) outside[i] = 0;
        if( outside[i] > 255 ) outside[i] = 255;
        out[i] = 255 - (unsigned char) outside[i];
        //out[i] = (unsigned char) outside[i];
    }

    free( xdist );
    free( ydist );
    free( gx );
    free( gy );
    free( data );
    free( outside );
    free( inside );
    return out;
}