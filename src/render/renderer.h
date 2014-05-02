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
#ifndef RENDERER_H
#define RENDERER_H

#include <stack>
#include <unordered_map>
#include <random>

#include "render2D.h"
#include "deferredFramebuffer.h"
#include "shaderManager.h"
#include "../scene.h"
#include "mesh.h"
#include "halfEdge.h"
#include "../eventManager.h"

class Renderer {
public:
	Renderer(int w, int h, glm::vec4 deferredArea, std::string id, EventManager* eventManager);

    ~Renderer();

	void start();

	void finish();

    void start2D();

    void finish2D();

    void resize(int width, int height, glm::vec4 deferredArea);

	void renderScene(Scene* scene);

	void renderEntity(Entity* entity);

    void loadMesh(const std::string& modelDirectory, const std::string& meshName);

	glm::mat4 lookAtTarget(glm::vec3 eyePt, glm::vec3 targetPt);

	glm::mat4 lookCamera(Entity* camera);

    glm::vec4 getTextBoundingBox(std::wstring text);

    GLuint& getDepthTexture();

    Render2D* get2DRenderer();

    bool getMouseLocation(Scene* scene, glm::vec2 coordinates, 
                          glm::ivec3& returnLocation, 
                          glm::vec3& returnNormal);

    void rebuildTile(Scene* scene, glm::ivec3 tileLocation);

    void update(double delta);

    void removeText(std::wstring text);

    void exportScene(Scene* scene);

    //void castRay(glm::vec2 coordinates);

private:
    typedef struct ModelInfo {
        GLuint vertexArrayObject;
        GLuint vertexBufferObject;
        GLuint normalBufferObject;
        GLuint indexBufferObject;
        GLuint colourIDBufferObject;
        GLuint uvwBufferObject;
        
        unsigned int numIndices;

        ModelInfo();
        ~ModelInfo();
    } ModelInfo;

    glm::vec4 deferredArea;
    glm::vec2 screenDimensions;

    std::string id;

    EventManager* eventManager;

    Listener* listener;

    bool wireframe = false;

    Render2D* render2D; /**< Renderer for topmost level 2D rendering. */
    ShaderManager* shaderManager; /**< Loads and manages all of the application's shaders */
    DeferredFramebuffer* deferredFBO;  /**< Frame buffer for deferred rendering */

    std::unordered_map<std::string, HalfEdge*> halfEdgeMeshes;
    std::unordered_map<std::string, std::vector<Mesh*>> meshes;

    std::unordered_map<std::string, ModelInfo*> models;

    // The below values would optimally all be in a struct
    GLuint screenQuadVertexArray;
    GLuint screenQuadVertexbuffer;
    GLuint screenQuadUvBuffer;
    GLuint screenQuadIndexBuffer;

    GLuint quadVertexArray;
    GLuint quadVertexbuffer;
    GLuint quadUvBuffer;
    GLuint quadIndexBuffer;

    //GLuint cubeVertexArrayID;

    GLuint noiseTex;
    GLuint gaussianTex;
    GLuint closenessTex;

    GLuint tileTex;

    float fFrustumScale;  /*< Scale of the frustum, set with calculateFrustumScale */

    std::stack<glm::mat4> matrixStack; /*< Model to camera matrix stack */

    glm::mat4 cameraToClipMatrix;
    glm::mat4 worldToCameraMatrix;

    std::mt19937 rndEngine;
    std::uniform_int_distribution<uint32_t> uintDist; 

    float fzNear, fzFar;

    void renderTexToScreen();

    void setModelToCameraMatrix();

    void bufferWindowScale(int w, int h);

    void renderTile(Scene* scene, glm::ivec3 tileLocation);

    void buildTileVBO(Scene* scene, glm::ivec3 tileLocation);

    void setRenderArea(glm::vec4 area, glm::vec2 screenDimensions);

    Mesh simplifyMesh(const std::vector<glm::vec3>& vertices, 
                      const std::vector<glm::vec3>& normals,
                      const std::vector<glm::ivec3>& indices, 
                      glm::vec3 normal);


/**
  * Generates a 2 x 2 quad, centered at (0, 0).
  * 
  * @return Address of generated quad's VAO.
  */
    void genQuad();

    void genScreenQuad();

    void gen3DTex();

    float randomFloat(float min, float max);

    void genKernel();

    void genNoiseTexture();

    void genGaussianTexture(int dimension, float sigma);

    GLuint genGaussianTexture1D(int dimension, float sigma);

    Mesh* getBlockType(Scene::Block::BlockType blockType, int blockRotation);

    void buildModel(const std::vector<float>& vertices,
                    const std::vector<float>& normals,
                    const std::vector<GLuint>& indices,
                    const std::vector<float>& colourIDs,
                    const std::vector<float>& uvw,
                    Scene::Tile* tile);

    glm::vec3 getTileColourID(glm::vec3 normal, int index,
                              Scene* scene, Scene::Tile* tile);
};

#endif