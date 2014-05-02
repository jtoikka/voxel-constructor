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
#include "deferredFramebuffer.h"

#include <iostream>

DeferredFramebuffer::DeferredFramebuffer(int w, int h) {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error: " << err << std::endl;
        std::cerr << "Before deferred FBO" << std::endl;
    }
    width = w; height = h;
    generateDeferredFBO(w, h);
    generateOcclusionFBO(w, h);
    generatePostFBO(w, h);
}

DeferredFramebuffer::~DeferredFramebuffer() {
    std::cout << "Deleting deferred FBO" << std::endl;
    glDeleteTextures(1, &fboNormal);
    glDeleteTextures(1, &fboDiffuse);
    glDeleteTextures(1, &fboDepth);
    glDeleteTextures(1, &fboColourID);
    glDeleteTextures(1, &fboOcclusion);
    glDeleteTextures(1, &fboPost);  
    glDeleteFramebuffers(1, &deferredFBO);
    glDeleteFramebuffers(1, &occlusionFBO);
    glDeleteFramebuffers(1, &postFBO);
    glDeleteRenderbuffers(1, &depthBuffer);
}

// Note: depth buffer to render buffer?
void DeferredFramebuffer::generateDeferredFBO(int w, int h) {
    // Create and bind the FBO
    glGenFramebuffers(1, &deferredFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, deferredFBO);

    // Depth buffer
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);

    // Normal buffer
    glGenTextures(1, &fboNormal);
    glBindTexture(GL_TEXTURE_2D, fboNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Diffuse color
    glGenTextures(1, &fboDiffuse);
    glBindTexture(GL_TEXTURE_2D, fboDiffuse);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Colour id
    glGenTextures(1, &fboColourID);
    glBindTexture(GL_TEXTURE_2D, fboColourID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8UI, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Attach the images to the framebuffer
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboNormal, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, fboDiffuse, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, fboColourID, 0);

    GLenum FBOStatus = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
    if (FBOStatus != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, ("ERROR: Framebuffer incomplete"));
    }

    GLenum drawBuffers[] = {GL_NONE, GL_COLOR_ATTACHMENT0, 
                            GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};

    glDrawBuffers(4, drawBuffers);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DeferredFramebuffer::generateOcclusionFBO(int w, int h) {
    // Create and bind the FBO
    glGenFramebuffers(1, &occlusionFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, occlusionFBO);

    // Diffuse color
    glGenTextures(1, &fboOcclusion);
    glBindTexture(GL_TEXTURE_2D, fboOcclusion);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Attach the images to the framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboOcclusion, 0);

    GLenum FBOStatus = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
    if (FBOStatus != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, ("ERROR: Framebuffer incomplete"));
    }

    GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0};

    glDrawBuffers(1, drawBuffers);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DeferredFramebuffer::generatePostFBO(int w, int h) {
    // Create and bind the FBO
    glGenFramebuffers(1, &postFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, postFBO);

    // Diffuse color
    glGenTextures(1, &fboPost);
    glBindTexture(GL_TEXTURE_2D, fboPost);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Attach the images to the framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboPost, 0);

    GLenum FBOStatus = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
    if (FBOStatus != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, ("ERROR: Framebuffer incomplete"));
    }

    GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0};

    glDrawBuffers(1, drawBuffers);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DeferredFramebuffer::resize(int w, int h) {
    width = w; height = h;

    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);

    glBindTexture(GL_TEXTURE_2D, fboNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glBindTexture(GL_TEXTURE_2D, fboDiffuse);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);

    glBindTexture(GL_TEXTURE_2D, fboColourID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glBindTexture(GL_TEXTURE_2D, fboOcclusion);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_FLOAT, NULL);

    glBindTexture(GL_TEXTURE_2D, fboPost);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
}

void DeferredFramebuffer::renderTo() {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, deferredFBO);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
}

void DeferredFramebuffer::renderToOcclusion() {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, occlusionFBO);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fboNormal);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, fboDiffuse);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, fboColourID);
}

void DeferredFramebuffer::renderToPost() {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, postFBO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fboOcclusion);
}

void DeferredFramebuffer::renderToScreen() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fboPost);
}

GLuint& DeferredFramebuffer::getDepthTexture() {
    return fboDepth;
}

unsigned int DeferredFramebuffer::getID(glm::vec2 coordinates) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, deferredFBO);
    GLint xCoordinate = coordinates.x;
    GLint yCoordinate = height - coordinates.y;

    float depthValue[4];

    glReadBuffer(GL_COLOR_ATTACHMENT2);
    glReadPixels(xCoordinate, yCoordinate, 1, 1, GL_RGBA, GL_FLOAT, depthValue);

    unsigned int rNorm = depthValue[0] * 255;
    unsigned int gNorm = depthValue[1] * 255;
    unsigned int bNorm = depthValue[2] * 255;

    unsigned int id = (rNorm << 16) ^ (gNorm << 8) ^ bNorm;
    return id;
}