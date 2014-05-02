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
#ifndef DEFERREDFRAMEBUFFER_H
#define DEFERREDFRAMEBUFFER_H

#include <GL/glew.h>
#include <vector>

#include "../../lib/glm/gtc/type_ptr.hpp"

class DeferredFramebuffer {
public:
    DeferredFramebuffer(int w, int h);

    void generateDeferredFBO(int w, int h);
    void generateOcclusionFBO(int w, int h);
    void generatePostFBO(int w, int h);

    void renderTo();

    void renderToOcclusion();
    void renderToPost();
    void renderToScreen(); 

    ~DeferredFramebuffer();

    void readDepthPixels();

    void resize(int w, int h);

    GLuint& getDepthTexture();

    float getDepth(glm::vec2 coordinates);

    unsigned int getID(glm::vec2 coordinates);

private:
    GLuint deferredFBO;
    GLuint occlusionFBO;
    GLuint postFBO;

    GLuint depthBuffer;

    GLuint fboNormal;
    GLuint fboDiffuse;
    GLuint fboDepth;
    GLuint fboColourID;
    GLuint fboOcclusion;
    GLuint fboPost;

    int width, height;
};

#endif