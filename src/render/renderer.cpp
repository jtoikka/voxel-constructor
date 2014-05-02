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
#include "renderer.h"
#include "../utility.h"
#include "trimesh.h"

#include <fstream>
#include <unordered_map>

// Uses degrees as opposed to radians for ease of use...
float calcFrustumScale(float fFovDeg) {
    const float degToRad = 3.141592654f * 2.0f / 360.0f;
    float fFovRad = fFovDeg * degToRad;
    return 1.0f / tan(fFovRad / 2.0f);
}

/* Initialize Renderer -------------------------------------------------------*/
Renderer::Renderer(int w, int h, glm::vec4 deferredArea,
                   std::string id, EventManager* eventManager) 
                        : deferredArea(deferredArea), id(id), 
                          eventManager(eventManager) {

    listener = new Listener();
    eventManager->addListener(id, listener);                                                                

    GLenum err;
    glewExperimental = GL_TRUE;
    
    err = glewInit();
    if(err != GLEW_OK) {
        glfwTerminate();
        fprintf(stderr, "Failed to initialize GLEW");
        fprintf(stderr, "%s", (char*)glewGetErrorString(err));
    }

    deferredFBO = new DeferredFramebuffer(w, h);
    shaderManager = new ShaderManager();

    render2D = new Render2D(shaderManager, glm::vec2(w, h));

    glm::mat4 modelToCameraMatrix(1.0f);
    matrixStack.push(modelToCameraMatrix);

    cameraToClipMatrix = glm::mat4(0.0f);

    fFrustumScale = calcFrustumScale(45.0f); // 45.0 degree field of view

    fzNear = 1.0f; fzFar = 100.0f;

    cameraToClipMatrix[0].x = fFrustumScale / (w  / (float) h);
    cameraToClipMatrix[1].y = fFrustumScale;
    cameraToClipMatrix[2].z = (fzFar + fzNear) / (fzNear - fzFar);
    cameraToClipMatrix[2].w = -1.0f;
    cameraToClipMatrix[3].z = (2 * fzFar * fzNear) / (fzNear - fzFar);

    genScreenQuad();
    genQuad();
    resize(w, h, deferredArea);

    rndEngine.seed(2309234852); // Button mashed random seed

    genNoiseTexture();
    genGaussianTexture(5, 16.0f);
    closenessTex = genGaussianTexture1D(255, 128.0f);
    genKernel();

    gen3DTex();

    glClampColor(GL_CLAMP_FRAGMENT_COLOR, GL_FALSE);
}
/*----------------------------------------------------------------------------*/

GLuint& Renderer::getDepthTexture() {
    return deferredFBO->getDepthTexture();
}

Renderer::ModelInfo::ModelInfo() { }

Renderer::ModelInfo::~ModelInfo() {
    glDeleteBuffers(1, &vertexBufferObject);
    glDeleteBuffers(1, &normalBufferObject);
    glDeleteBuffers(1, &indexBufferObject);
    glDeleteBuffers(1, &colourIDBufferObject);
    glDeleteVertexArrays(1, &vertexArrayObject);
}

Renderer::~Renderer() {
    if (render2D != nullptr)
        delete render2D;
    if (shaderManager != nullptr)
        delete shaderManager;
    if (deferredFBO != nullptr)
        delete deferredFBO;

    for (auto meshPair : halfEdgeMeshes) {
        delete meshPair.second;
    }

    for (auto meshPair : meshes) {
        for (auto mesh : meshPair.second) {
            delete mesh;
        }
    }

    for (auto modelPair : models) {
        delete modelPair.second;
    }

    eventManager->removeListener(id);
    delete listener;

    glDeleteVertexArrays(1, &quadVertexArray);
    glDeleteBuffers(1, &quadVertexbuffer);
    glDeleteBuffers(1, &quadUvBuffer);
    glDeleteBuffers(1, &quadIndexBuffer);

    glDeleteVertexArrays(1, &screenQuadVertexArray);
    glDeleteBuffers(1, &screenQuadVertexbuffer);
    glDeleteBuffers(1, &screenQuadUvBuffer);
    glDeleteBuffers(1, &screenQuadIndexBuffer);
}

void Renderer::update(double delta) {
    for (auto event : listener->events) {
        switch (event->action) {
        case Action::REBUILD_TILE: {
            auto eventScene = std::dynamic_pointer_cast<Event<Scene*>>(event);
            Scene* scene = eventScene->args[0];
            std::vector<glm::ivec3>& modifiedTiles = scene->getModifiedTiles();
            for (glm::ivec3 tileLocation : modifiedTiles) {
                rebuildTile(scene, tileLocation);
            }
            modifiedTiles.clear();
            break;
        }
        case Action::TOGGLE_WIREFRAME: {
            if (wireframe) {
                wireframe = false;
            } else {
                wireframe = true;
            }
            break;
        }
        case Action::EXPORT_TILE: {
            auto eventScene = std::dynamic_pointer_cast<Event<Scene*>>(event);
            exportScene(eventScene->args[0]);
            break;
        }
        default:
            break;
        }
    }
    listener->events.clear();
}

void Renderer::renderTexToScreen() {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);

    glBindVertexArray(screenQuadVertexArray);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Renderer::start() {
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glDisable(GL_BLEND);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
    glDepthRange(0.0f, 1.0f);

    glEnable(GL_DEPTH_CLAMP);

    glClearDepth(1.0f);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    deferredFBO->renderTo(); // Start rendering to FBO

    float viewPortWidth = deferredArea.z - deferredArea.x;
    float viewPortHeight = deferredArea.w - deferredArea.y;

    bufferWindowScale(viewPortWidth, viewPortHeight);

    glViewport(0, 0, (GLsizei) viewPortWidth, (GLsizei) viewPortHeight);

    if (wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDisable(GL_CULL_FACE);
    }
}

void Renderer::finish() {
    deferredFBO->renderToOcclusion();

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glUseProgram(shaderManager->getBaseLightingShader());
    glClearColor(57.0f / 255.0f, 57.0f / 255.0f, 57.0f / 255.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, noiseTex);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);

    glBindVertexArray(quadVertexArray);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    deferredFBO->renderToPost();

    glUseProgram(shaderManager->getOcclusionBlurShader());

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gaussianTex);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_1D, closenessTex);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(quadVertexArray);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    deferredFBO->renderToScreen();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderManager->getPostShader());

    glViewport(0, 0, (GLsizei) screenDimensions.x, 
                     (GLsizei) screenDimensions.y);
    bufferWindowScale((GLsizei) screenDimensions.x, 
                      (GLsizei) screenDimensions.y);

    renderTexToScreen();
}

void Renderer::start2D() {
    render2D->start();
}

void Renderer::finish2D() {
    render2D->finish();
}

bool Renderer::getMouseLocation(Scene* scene, glm::vec2 coordinates, 
                                glm::ivec3& returnLocation, 
                                glm::vec3& returnNormal) {
    unsigned int id = deferredFBO->getID(coordinates);
    if (id != 0) {

        unsigned int mask = (1 << scene->getMaxBytes()) - 1;

        unsigned int blockId = mask & id;

        unsigned int tileId = ((~mask) & id) >> scene->getMaxBytes();

        blockId--;

        glm::vec3 normal(0.0f);

        switch (blockId % 6) {
        case 0:
            normal.x =  1.0f; break;
        case 1:
            normal.x = -1.0f; break;
        case 2: 
            normal.y = -1.0f; break;
        case 3:
            normal.z = -1.0f; break;
        case 4:
            normal.y =  1.0f; break;
        case 5:
            normal.z =  1.0f; break;
        }

        blockId /= 6;

        glm::ivec3 idLocation = scene->getBlockLocation(blockId);

        glm::vec3 tileLocation = scene->getTileLocation(tileId) * glm::vec3(scene->getTileDimensions());

        returnLocation = glm::vec3(idLocation) + tileLocation;

        returnNormal = normal;

        return true;
    }
    return false;
}

void Renderer::gen3DTex() {
    std::vector<float> texels;

    for (int i = 0; i < 16 * 16 * 16; i++) {
        if (i % 2) {
            texels.push_back(randomFloat(0.0f, 1.0f));
            texels.push_back(randomFloat(0.0f, 1.0f));
            texels.push_back(randomFloat(0.0f, 1.0f));
        } else {
            texels.push_back(randomFloat(0.0f, 1.0f));
            texels.push_back(randomFloat(0.0f, 1.0f));
            texels.push_back(randomFloat(0.0f, 1.0f));
        }
    }

    glGenTextures(1, &tileTex);
    glBindTexture(GL_TEXTURE_3D, tileTex);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB16F, 8, 8, 
                 8, 0, GL_RGB, GL_FLOAT, &texels[0]);
}

void Renderer::renderScene(Scene* scene) {
    Entity* camera = scene->getCamera();
    worldToCameraMatrix = lookCamera(camera);
    matrixStack.push(matrixStack.top() * worldToCameraMatrix);

    glUseProgram(shaderManager->getEntityShader());

    setModelToCameraMatrix();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, tileTex);

    for (auto tile : scene->getTiles()) {
        renderTile(scene, tile->location);
    }

    matrixStack.pop();
}

Mesh* Renderer::getBlockType(Scene::Block::BlockType blockType,
                             int blockRotation) {
    Mesh* mesh = nullptr;
    switch (blockType) {
    case Scene::Block::BlockType::CUBE: {
        auto meshIt = meshes.find("cube");
        if (meshIt == meshes.end()) break;
        mesh = meshIt->second[blockRotation];
        break;
    }
    case Scene::Block::BlockType::SLOPE: {
        auto meshIt = meshes.find("slope");
        if (meshIt == meshes.end()) break;
        mesh = meshIt->second[blockRotation];
        break; 
    }
    case Scene::Block::BlockType::RSLOPE: {
        auto meshIt = meshes.find("rslope");
        if (meshIt == meshes.end()) break;
        mesh = meshIt->second[blockRotation];
        break;
    }
    case Scene::Block::BlockType::DIAGONAL: {
        auto meshIt = meshes.find("diagonal");
        if (meshIt == meshes.end()) break;
        mesh = meshIt->second[blockRotation];
        break;
    }
    case Scene::Block::BlockType::CORNERSLOPE: {
        auto meshIt = meshes.find("cornerSlope");
        if (meshIt == meshes.end()) break;
        mesh = meshIt->second[blockRotation];
        break;
    }
    case Scene::Block::BlockType::RCORNERSLOPE: {
        auto meshIt = meshes.find("rcornerSlope");
        if (meshIt == meshes.end()) break;
        mesh = meshIt->second[blockRotation];
        break;
    }
    case Scene::Block::BlockType::INVCORNER: {
        auto meshIt = meshes.find("invCorner");
        if (meshIt == meshes.end()) break;
        mesh = meshIt->second[blockRotation];
        break;
    }
    case Scene::Block::BlockType::RINVCORNER: {
        auto meshIt = meshes.find("rInvCorner");
        if (meshIt == meshes.end()) break;
        mesh = meshIt->second[blockRotation];
        break;
    }
    default:
        break;
    }
    return mesh;
}

void Renderer::buildModel(const std::vector<float>& vertices,
                          const std::vector<float>& normals,
                          const std::vector<GLuint>& indices,
                          const std::vector<float>& colourIDs,
                          const std::vector<float>& uvw,
                          Scene::Tile* tile) {
    GLuint vertexArrayID;
    GLuint vertexBufferID;
    GLuint normalBufferID;
    GLuint colourIDBuffer;
    GLuint indexBufferID;
    GLuint uvwBufferID;

    // Create VAO
    glGenVertexArrays(1, &vertexArrayID);
    glBindVertexArray(vertexArrayID);

    // Create vertex buffer
    glGenBuffers(1, &vertexBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(),
                 &vertices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // Create normal buffer
    glGenBuffers(1, &normalBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normals[0]) * normals.size(), 
                 &normals[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // Create colour ID buffer
    glGenBuffers(1, &colourIDBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, colourIDBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colourIDs[0]) * colourIDs.size(),
                 &colourIDs[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // Create uvw buffer
    glGenBuffers(1, &uvwBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, uvwBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uvw[0]) * uvw.size(),
                 &uvw[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // Create index buffer
    glGenBuffers(1, &indexBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(),
                 &indices[0], GL_STATIC_DRAW);

    glBindVertexArray(0);

    ModelInfo* modelInfo = new ModelInfo();

    modelInfo->vertexArrayObject = vertexArrayID;
    modelInfo->vertexBufferObject = vertexBufferID;
    modelInfo->normalBufferObject = normalBufferID;
    modelInfo->indexBufferObject = indexBufferID;
    modelInfo->colourIDBufferObject = colourIDBuffer;
    modelInfo->uvwBufferObject = uvwBufferID;
    modelInfo->numIndices = indices.size();

    std::stringstream ss;

    ss << "tile " << tile->location.x 
       << " " << tile->location.y
       << " " << tile->location.z;

    models[ss.str()] = modelInfo;
}

glm::vec3 Renderer::getTileColourID(glm::vec3 normal, int index,
                                    Scene* scene, Scene::Tile* tile) {
    unsigned int faceID = 0;

    if (normal.x < 0.0f) {
        faceID = 1;
    } else if (normal.y < 0.0f) {
        faceID = 2;
    } else if (normal.z < 0.0f) {
        faceID = 3;
    } else if (normal.y > 0.0f) {
        faceID = 4;
    } else if (normal.z > 0.0f) {
        faceID = 5;
    }

    unsigned int colourID = index * 6 + 1 + faceID;

    unsigned int blockR = ((colourID >> 16) & 0xFF);
    unsigned int blockG = ((colourID >> 8 ) & 0xFF);
    unsigned int blockB = ((colourID >> 0 ) & 0xFF);

    unsigned int tileColourID = scene->getTileID(tile);

    tileColourID = tileColourID << scene->getMaxBytes();

    unsigned int tileR = ((tileColourID >> 16) & 0xFF);
    unsigned int tileG = ((tileColourID >> 8 ) & 0xFF);
    unsigned int tileB = ((tileColourID >> 0 ) & 0xFF);

    float colourR = (blockR | tileR);
    float colourG = (blockG | tileG);
    float colourB = (blockB | tileB);

    colourR = (blockR | tileR) / 255.0f;
    colourG = (blockG | tileG) / 255.0f;
    colourB = (blockB | tileB) / 255.0f;

    return glm::vec3(colourR, colourG, colourB);
}

void Renderer::buildTileVBO(Scene* scene, glm::ivec3 tileLocation) {
    auto tile = scene->getTile(tileLocation);
    if (tile == nullptr) return;

    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<GLuint> indices;
    std::vector<float> colourIDs;
    std::vector<float> uvw;

    size_t indexCount = 0;
    for (size_t i = 0; i < tile->blocks.size(); i++) {
        glm::ivec3 blockLocation = scene->getBlockLocation(i);

        glm::ivec3 location = tileLocation * scene->getTileDimensions()
                            + blockLocation;

        Scene::Block& block = scene->getBlock(location);

        Mesh* mesh = getBlockType(block.blockType, block.rotation);
        if (mesh == nullptr) {
            continue;
        }

        std::vector<glm::vec3> faceNormals;

        auto pushVertex = [&](glm::vec3 vertex) {
            vertices.push_back(vertex.x + location.x);
            vertices.push_back(vertex.y + location.y);
            vertices.push_back(vertex.z + location.z);
        };
        auto pushNormal = [&](glm::vec3 normal) {
            normals.push_back(normal.x);
            normals.push_back(normal.y);
            normals.push_back(normal.z);
        };
        auto pushColourID = [&](glm::vec3 colourID) {
            colourIDs.push_back(colourID.x);
            colourIDs.push_back(colourID.y);
            colourIDs.push_back(colourID.z);
        };
        auto pushUvw = [&](glm::vec3 location) {
            uvw.push_back((location.x + blockLocation.x + 0.5f)
                / scene->getTileDimensions().x);
            uvw.push_back((location.y + blockLocation.y + 0.5f)
                / scene->getTileDimensions().y);
            uvw.push_back((location.z + blockLocation.z + 0.5f)
                / scene->getTileDimensions().z);
        };
        auto addTriangle = [&](size_t v1, size_t v2, size_t v3) {
            pushVertex(mesh->vertices[v1]);
            pushVertex(mesh->vertices[v2]);
            pushVertex(mesh->vertices[v3]);

            pushNormal(mesh->normals[v1]);
            pushNormal(mesh->normals[v2]);
            pushNormal(mesh->normals[v3]);

            indices.push_back(indexCount + 0);
            indices.push_back(indexCount + 1);
            indices.push_back(indexCount + 2);

            indexCount += 3;

            glm::vec3 colourID = 
                        getTileColourID(mesh->normals[v1], 
                                        i, scene, tile);

            pushColourID(colourID);
            pushColourID(colourID);
            pushColourID(colourID);

            glm::vec3 averageVert = (mesh->vertices[v1]
                                   + mesh->vertices[v2]
                                   + mesh->vertices[v3]) / 3.0f;

            pushUvw(averageVert);
            pushUvw(averageVert);
            pushUvw(averageVert);
        };

        for (size_t j = 0; j <= mesh->normals.size(); j++) {
            if (!(faceNormals.size() == 0 
               || faceNormals[0] == mesh->normals[j])) {

                int visibility = 
                    scene->checkVisibility(location, mesh->normals[j - 1]);
                size_t faceSize = faceNormals.size();
                faceNormals.clear();
                if (faceSize == 3) {
                    int ownVisibility = 
                        scene->checkVisibilityDirection(location, 
                                                        mesh->normals[j - 1]);
                    int criteria = 1;
                    if (mesh->normals[j - 1].y != 0) criteria = 0;  
                    if (abs(visibility + ownVisibility) != criteria 
                     && ownVisibility != -1)
                        addTriangle(j-3, j-2, j-1);
                } else {
                    switch (visibility) {
                    case 1: {
                        addTriangle(j-4, j-3, j-2);
                        addTriangle(j-4, j-2, j-1);
                        break;
                    }
                    case Scene::SE: {
                        addTriangle(j-4, j-2, j-1);
                        break;
                    } 
                    case Scene::SW: {
                        addTriangle(j-1, j-4, j-3);
                        break;
                    }
                    case Scene::NE: {
                        addTriangle(j-3, j-2, j-1);
                        break;
                    }
                    case Scene::NW: {
                        addTriangle(j-4, j-3, j-2);
                        break;
                    }
                    default:
                        break;
                    }
                }
            }
            if (j < mesh->normals.size())
                faceNormals.push_back(mesh->normals[j]);
        }
    }

    if (vertices.size() == 0) {
        std::cout << "No vertices" << std::endl;
        return;
    }

    buildModel(vertices, normals, indices, colourIDs, uvw, tile);
}

void Renderer::removeText(std::wstring text) {
    render2D->removeText(text);
}

void Renderer::rebuildTile(Scene* scene, glm::ivec3 tileLocation) {
    auto tile = scene->getTile(tileLocation);
    if (tile != nullptr) {
        std::stringstream ss;
    
        auto modelIt = models.find(ss.str());
        if (modelIt != models.end()) {
            delete modelIt->second;
            models.erase(modelIt);
        }
        buildTileVBO(scene, tileLocation);
    } else {
        std::cout << "Tile is null" << std::endl;
    }
}

void Renderer::loadMesh(const std::string& modelDirectory, 
                        const std::string& meshName) {
    HalfEdge* halfEdge = new HalfEdge(modelDirectory, meshName);
    halfEdgeMeshes.emplace(meshName, halfEdge);
    Mesh* mesh = halfEdge->toMesh();
    std::vector<Mesh*> rotatedMeshes;
    for (int i = 0; i < 4; i++) {
        glm::mat4 rotationMatrix = Utility::calcYRotation(Utility::PI / 2 * i);
        Mesh* rotatedMesh = new Mesh();
        for (glm::vec3 vertex : mesh->vertices) {
            glm::vec3 rotatedVertex = glm::vec3(rotationMatrix 
                                              * glm::vec4(vertex, 1.0f));

            rotatedVertex.x = round(rotatedVertex.x * 10.0f) / 10.0f;
            rotatedVertex.y = round(rotatedVertex.y * 10.0f) / 10.0f;
            rotatedVertex.z = round(rotatedVertex.z * 10.0f) / 10.0f;
            rotatedMesh->vertices.push_back(rotatedVertex);
        }
        for (glm::vec3 normal : mesh->normals) {
            glm::vec3 rotatedNormal = glm::vec3(rotationMatrix
                                              * glm::vec4(normal, 1.0f));
            rotatedNormal.x = round(rotatedNormal.x * 1000000.0f) / 1000000.0f;
            rotatedNormal.y = round(rotatedNormal.y * 1000000.0f) / 1000000.0f;
            rotatedNormal.z = round(rotatedNormal.z * 1000000.0f) / 1000000.0f;
            rotatedMesh->normals.push_back(rotatedNormal);
        }
        for (unsigned int index : mesh->indices) {
            rotatedMesh->indices.push_back(index);
        }
        rotatedMeshes.push_back(rotatedMesh);
    }
    meshes.insert(make_pair(meshName, rotatedMeshes));
}

struct Point {
    std::vector<Point*> points; // Going out from the point
};

void Renderer::exportScene(Scene* scene) {
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::ivec3> indices;

    size_t indexCount = 0;
    for (auto tile : scene->tiles) {
        glm::ivec3 tileLocation = tile->location;
        for (size_t i = 0; i < tile->blocks.size(); i++) {
            glm::ivec3 location = tileLocation * scene->getTileDimensions()
                                + scene->getBlockLocation(i);

            Scene::Block& block = scene->getBlock(location);

            Mesh* mesh = getBlockType(block.blockType, block.rotation);
            if (mesh == nullptr) {
                continue;
            }

            std::vector<glm::vec3> faceNormals;

            auto pushVertex = [&](glm::vec3 vertex) {
                vertices.push_back(vertex + glm::vec3(location));
            };
            auto pushNormal = [&](glm::vec3 normal) {
                normals.push_back(normal);
            };
            auto addTriangle = [&](size_t v1, size_t v2, size_t v3) {
                pushVertex(mesh->vertices[v1]);
                pushVertex(mesh->vertices[v2]);
                pushVertex(mesh->vertices[v3]);

                pushNormal(mesh->normals[v1]);
                pushNormal(mesh->normals[v2]);
                pushNormal(mesh->normals[v3]);

                glm::ivec3 index;
                index.x = indexCount + 0;
                index.y = indexCount + 1;
                index.z = indexCount + 2;

                indices.push_back(index);

                indexCount += 3;
            };

            for (size_t j = 0; j <= mesh->normals.size(); j++) {
                if (!(faceNormals.size() == 0 
                   || faceNormals[0] == mesh->normals[j])) {

                    int visibility = 
                        scene->checkVisibility(location, mesh->normals[j - 1]);
                    size_t faceSize = faceNormals.size();
                    faceNormals.clear();
                    if (faceSize == 3) {
                        int ownVisibility = 
                            scene->checkVisibilityDirection(location, 
                                                            mesh->normals[j - 1]);
                        int criteria = 1;
                        if (mesh->normals[j - 1].y != 0) criteria = 0;  
                        if (abs(visibility + ownVisibility) != criteria 
                         && ownVisibility != -1)
                            addTriangle(j-3, j-2, j-1);
                    } else {
                        switch (visibility) {
                        case 1: {
                            addTriangle(j-4, j-3, j-2);
                            addTriangle(j-4, j-2, j-1);
                            break;
                        }
                        case Scene::SE: {
                            addTriangle(j-4, j-2, j-1);
                            break;
                        } 
                        case Scene::SW: {
                            addTriangle(j-1, j-4, j-3);
                            break;
                        }
                        case Scene::NE: {
                            addTriangle(j-3, j-2, j-1);
                            break;
                        }
                        case Scene::NW: {
                            addTriangle(j-4, j-3, j-2);
                            break;
                        }
                        default:
                            break;
                        }
                    }
                }
                if (j < mesh->normals.size())
                    faceNormals.push_back(mesh->normals[j]);
            }
        }
    }

    auto myHash = [](const glm::vec3& vert) {
        return std::hash<float>()(vert.x) 
             ^ std::hash<float>()(vert.y) 
             ^ std::hash<float>()(vert.z);
    };
    std::unordered_map<glm::vec3, int, decltype(myHash)> vertexMap(10, myHash);

    int i = 0;
    for (glm::vec3 vertex : vertices) {
        if (vertexMap.find(vertex) == vertexMap.end()) {
            vertexMap[vertex] = i;
            i++;
        }
    }

    BasicMesh baseMesh;

    for (glm::ivec3 triangle : indices) {
        glm::vec3 vertex1 = vertices[triangle.x];
        glm::vec3 vertex2 = vertices[triangle.y];
        glm::vec3 vertex3 = vertices[triangle.z];
        glm::vec3 normal = normals[triangle.x];

        glm::ivec4 baseIndex;
        baseIndex.x = vertexMap[vertex1];
        baseIndex.y = vertexMap[vertex2];
        baseIndex.z = vertexMap[vertex3];
        baseIndex.w = -1;

        baseMesh.indices.push_back(baseIndex);
        baseMesh.normals.push_back(normal);
    }

    std::map<int, glm::vec3> orderedVerts;

    for (auto vertexPair : vertexMap) {
        orderedVerts[vertexPair.second ] = vertexPair.first;
    }

    for (auto vert : orderedVerts) {
        baseMesh.vertices.push_back(vert.second);
    }

    scene->save("tile.sav");

    TriMesh* trimesh = new TriMesh(baseMesh);
    trimesh->decimate();
    trimesh->writeObj();
    delete trimesh;
}

void Renderer::renderTile(Scene* scene, glm::ivec3 tileLocation) {
    auto tile = scene->getTile(tileLocation);
    if (tile != nullptr) {
        std::stringstream ss;

        ss << "tile " << tile->location.x 
           << " " << tile->location.y
           << " " << tile->location.z;

        auto modelIt = models.find(ss.str());
        if (modelIt == models.end()) {
            buildTileVBO(scene, tileLocation);
            modelIt = models.find(ss.str());
        }

        ModelInfo* model = modelIt->second;
        glBindVertexArray(model->vertexArrayObject);
        glDrawElements(GL_TRIANGLES, model->numIndices, GL_UNSIGNED_INT, 0);
    }
}

void Renderer::genScreenQuad() {
    glGenVertexArrays(1, &screenQuadVertexArray);
    glBindVertexArray(screenQuadVertexArray);

    static const GLfloat screenQuadVertexBufferData[] = {
        -1.0f, -1.0f, // 0 1 2 - 2 1 3
         1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f,  1.0f
    };

    static const GLfloat uvBufferData[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f
    };

    static const GLuint indexBufferData[] = {
        0, 1, 2,
        2, 1, 3
    };

    glGenBuffers(1, &screenQuadVertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, screenQuadVertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(screenQuadVertexBufferData), screenQuadVertexBufferData, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glGenBuffers(1, &screenQuadUvBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, screenQuadUvBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uvBufferData), uvBufferData, GL_STATIC_DRAW);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glGenBuffers(1, &screenQuadIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, screenQuadIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexBufferData), indexBufferData, GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void Renderer::genQuad() {
    glGenVertexArrays(1, &quadVertexArray);
    glBindVertexArray(quadVertexArray);

    static const GLfloat quadVertexBufferData[] = {
        -1.0f, -1.0f, // 0 1 2 - 2 1 3
         1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f,  1.0f
    };

    static const GLfloat uvBufferData[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f
    };

    static const GLuint indexBufferData[] = {
        0, 1, 2,
        2, 1, 3
    };

    glGenBuffers(1, &quadVertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, quadVertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertexBufferData), quadVertexBufferData, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glGenBuffers(1, &quadUvBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, quadUvBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uvBufferData), uvBufferData, GL_STATIC_DRAW);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glGenBuffers(1, &quadIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexBufferData), indexBufferData, GL_STATIC_DRAW);

    glBindVertexArray(0);

    //return quadVertexArray;
}

void Renderer::setRenderArea(glm::vec4 area, glm::vec2 screenDimensions) {
    GLfloat screenQuadVertexBufferData[] = {
        2 * area.x / screenDimensions.x - 1.0f, -2 * area.w / screenDimensions.y + 1.0f,
        2 * area.z / screenDimensions.x - 1.0f, -2 * area.w / screenDimensions.y + 1.0f,
        2 * area.x / screenDimensions.x - 1.0f, -2 * area.y / screenDimensions.y + 1.0f,
        2 * area.z / screenDimensions.x - 1.0f, -2 * area.y / screenDimensions.y + 1.0f
    };

    glBindBuffer(GL_ARRAY_BUFFER, screenQuadVertexbuffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(screenQuadVertexBufferData), screenQuadVertexBufferData);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Renderer::setModelToCameraMatrix() {
    glBindBuffer(GL_UNIFORM_BUFFER, shaderManager->getGlobalMatricesUBO());
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), 
                    sizeof(glm::mat4), glm::value_ptr(matrixStack.top()));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

glm::mat4 Renderer::lookCamera(Entity* camera) {
    if (camera != nullptr) {
        SpatialComponent* spatialComponent = 
            (SpatialComponent*)camera->getComponent(SpatialComponent::key);
        CameraComponent* cameraComponent = 
            (CameraComponent*)camera->getComponent(CameraComponent::key);
        
        worldToCameraMatrix = lookAtTarget(spatialComponent->location, 
                                           cameraComponent->targetLocation);
        return worldToCameraMatrix;
    } else {
        std::cerr << "ERROR: Camera not found." << std::endl;
    }
    return glm::mat4(1.0f);
}

glm::mat4 Renderer::lookAtTarget(glm::vec3 eyePt, glm::vec3 targetPt) {
    glm::vec3 forward, side, up;

    up.y = 1.0f; 
    forward = glm::normalize(glm::vec3(targetPt.x, targetPt.y, -targetPt.z) 
                           - glm::vec3(eyePt.x, eyePt.y, -eyePt.z));
    forward.x = -forward.x;
    if (forward.y == 1.0f || forward.y == -1.0f) {
        up = glm::vec3(1.0f, 0.0f, 0.0f);
    }
    side = glm::normalize(glm::cross(forward, up));
    up = glm::normalize(glm::cross(side, forward));

    glm::mat4 lookAtMatrix(1.0f);
    lookAtMatrix[0] = glm::vec4(side, 0.0f);
    lookAtMatrix[1] = glm::vec4(up, 0.0f);
    lookAtMatrix[2] = glm::vec4(-forward, 0.0f);

    lookAtMatrix = glm::transpose(lookAtMatrix);

    glm::mat4 transMat(1.0f);
    transMat[3] = glm::vec4(-glm::vec3(-eyePt.x, eyePt.y, -eyePt.z), 1.0f);

    glm::mat4 flip(1.0f);
    flip[0].x = -1.0f;
    flip[2].z = -1.0f;

    return lookAtMatrix * transMat * flip;
}

void Renderer::resize(int w, int h, glm::vec4 deferredArea) {
    screenDimensions.x = w;
    screenDimensions.y = h;

    this->deferredArea = deferredArea;
    float viewPortWidth = deferredArea.z - deferredArea.x;
    float viewPortHeight = deferredArea.w - deferredArea.y;

    bufferWindowScale(w, h);

    cameraToClipMatrix[0].x = fFrustumScale 
                            / (viewPortWidth / (float)viewPortHeight);

    glBindBuffer(GL_UNIFORM_BUFFER, shaderManager->getGlobalMatricesUBO());
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), 
                    glm::value_ptr(cameraToClipMatrix));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glm::mat4 inverseCameraToClipMatrix = glm::inverse(cameraToClipMatrix);

    glBindBuffer(GL_UNIFORM_BUFFER, shaderManager->getGlobalMatricesUBO());
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, sizeof(glm::mat4),
                    glm::value_ptr(inverseCameraToClipMatrix));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    if (deferredFBO != nullptr) {
        deferredFBO->resize(viewPortWidth, viewPortHeight);
    } else {
        deferredFBO = new DeferredFramebuffer(viewPortWidth, viewPortHeight);
    }
    setRenderArea(deferredArea, glm::vec2(w, h));
    render2D->resize(w, h);
}

void Renderer::bufferWindowScale(int w, int h) {
    float windowScale[] = {(float)w, (float)h};
    
    glBindBuffer(GL_UNIFORM_BUFFER, shaderManager->getWindowScaleUBO());
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float) * 2, windowScale);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

glm::vec4 Renderer::getTextBoundingBox(std::wstring text) {
    return render2D->getTextBoundingBox(text);
}

Render2D* Renderer::get2DRenderer() {
    return render2D;
}

const int kernelSize = 128;

float Renderer::randomFloat(float min, float max) {
    uint32_t rndInt = uintDist(rndEngine);

    float rangeLimited = (float)rndInt / uintDist.max();

    rangeLimited *= (max - min);
    rangeLimited += min;
    return rangeLimited;
}

void Renderer::genKernel() {
    glm::vec3 kernel[kernelSize];
    for (int i = 0; i < kernelSize;) {
        kernel[i] = glm::vec3(randomFloat(-1.0f, 1.0f),
                              randomFloat(-1.0f, 1.0f), 
                              randomFloat( 0.0f, 1.0f));

        kernel[i] = glm::normalize(kernel[i]);

        float scale = float(i) / float(kernelSize);
        
        auto lerp = [](float x, float y, float weight) {
            return x * (1 - weight) + weight * y;
        };

        scale = lerp(0.1f, 1.0f, scale * scale);
        kernel[i] *= scale;

        i++;
    }

    float kernelFloat[kernelSize * 3];

    for (int i = 0; i < kernelSize; i++) {
        kernelFloat[i * 3    ] = kernel[i].x;
        kernelFloat[i * 3 + 1] = kernel[i].y;
        kernelFloat[i * 3 + 2] = kernel[i].z; 
    }

    glUseProgram(shaderManager->getBaseLightingShader());
    glUniform3fv(glGetUniformLocation(
                     shaderManager->getBaseLightingShader(), "kernel"), 
                 16, kernelFloat);

    glUniform2fv(glGetUniformLocation(
                     shaderManager->getBaseLightingShader(), "texDimensions"),
                 1, &deferredArea.x);
}

const int noiseTexDimension = 5;

void Renderer::genNoiseTexture() {
    glm::vec3 noise[noiseTexDimension * noiseTexDimension];
    for (int i = 0; i < noiseTexDimension * noiseTexDimension; i++) {
        noise[i] = glm::vec3(randomFloat(-1.0f, 1.0f), randomFloat(-1.0f, 1.0f), 0.0f);
        glm::normalize(noise[i]);
    }

    glGenTextures(1, &noiseTex);  
    glBindTexture(GL_TEXTURE_2D, noiseTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, noiseTexDimension, noiseTexDimension,
                 0, GL_RGB, GL_FLOAT, noise);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

const float e = 2.71828f;

void Renderer::genGaussianTexture(int dimension, float sigma) {
    float gaussian[dimension * dimension];
    float limit = (dimension - 1.0f) / 2.0f;
    int i = 0;
    float total = 0.0f;
    for (float x = -limit; x <= limit; x++) {
        for (float y = -limit; y <= limit; y++) {
            gaussian[i] = (1.0f / (2.0f * Utility::PI * sigma * sigma)) 
                        * pow(e, -(x * x + y * y) / (2 * sigma * sigma));
            total += gaussian[i];
            i++;
        }
    }

    i = 0;
    for (float x = -limit; x <= limit; x++) {
        for (float y = -limit; y <= limit; y++) {
            gaussian[i] = gaussian[i] / total;
            i++;
        }
    }

    glGenTextures(1, &gaussianTex);  
    glBindTexture(GL_TEXTURE_2D, gaussianTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, noiseTexDimension, noiseTexDimension,
                 0, GL_RED, GL_FLOAT, gaussian);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

GLuint Renderer::genGaussianTexture1D(int dimension, float sigma) {
    GLuint gaussTex;
    float gaussian[dimension + 1];
    int i = 0;
    for (float x = 0; x <= dimension; x++) {
        gaussian[i] = (1.0f / sqrt(2.0f * Utility::PI * sigma * sigma)) 
                    * pow(e, -(x * x) / (2 * sigma * sigma));
        i++;
    }

    float topValue = gaussian[0];
    for (int j = 0; j < dimension + 1; j++) {
        gaussian[j] = gaussian[j] / topValue;
    }


    glGenTextures(1, &gaussTex);  
    glBindTexture(GL_TEXTURE_1D, gaussTex);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, dimension + 1, 0,
                 GL_RED, GL_FLOAT, gaussian);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAX_LEVEL, 0);
    glBindTexture(GL_TEXTURE_1D, 0);

    return gaussTex;
}