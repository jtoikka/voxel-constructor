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
#include "halfEdge.h"
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <algorithm>

#include "../utility.h"

HalfEdge::HalfEdge(const std::string& modelDirectory, const std::string& meshName) {
    loadModel(modelDirectory, meshName);
}

HalfEdge::HalfEdge(BasicMesh baseMesh) {
    populateHalfEdgeStructure(baseMesh);
}

HalfEdge::~HalfEdge() {
    std::cout << "Deleting half edge..." << std::endl;
    for (auto vertex : vertices) {
        delete vertex;
    }
    for (auto edge : edges) {
        delete edge;
    }
    for (auto face : faces) {
        delete face;
    }
    std::cout << "Finished deleting." << std::endl;
}

void HalfEdge::loadModel(const std::string& modelDirectory, const std::string& modelName) {
    BasicMesh baseMesh;
    std::string fullPath = modelDirectory + modelName + ".rawmodel";

    std::ifstream infile(fullPath.c_str());
    if (infile.is_open() || modelName == "empty") {
        std::string line;

        char type;

        while(std::getline(infile, line)) {
            if (line.empty()) {
                continue;
            }
            std::istringstream in(line);
            in >> type;

            switch (type) {
            case 'v': {
                float x, y, z, w;
                in >> x >> y >> z >> w;
                baseMesh.vertices.push_back(glm::vec3(x, y, z));
                continue;
            }
            case 'n': {
                float x, y, z, w;
                in >> x >> y >> z >> w;
                baseMesh.normals.push_back(glm::normalize(glm::vec3(x, y, z)));
                continue;
            }
            case 'f': {
                std::string face;
                unsigned int x, y, z, w;
                in >> x >> y >> z;
                if (in >> w) {
                    baseMesh.indices.push_back(glm::ivec4(x, y, z, w));
                } else {
                    baseMesh.indices.push_back(glm::ivec4(x, y, z, -1));
                }
                continue;
            }
            case '#': {
                continue;
            }
            default:
                break;
            }
        }
        int lastIndex = modelName.find_last_of(".");
        std::string rawName = modelName.substr(0, lastIndex);
        lastIndex = rawName.find_last_of("/");
        rawName = rawName.substr(lastIndex + 1);

        populateHalfEdgeStructure(baseMesh);
    } else {
        std::cerr << "ERROR: File '" << modelName << ".model' does not exist." << std::endl;
    }
}

void HalfEdge::populateHalfEdgeStructure(const BasicMesh& mesh) {
    std::map<std::pair<int, int>, Edge*> edgePairFinder; // Temporary storage for finding half edge pairs
    for (int i = 0; i < mesh.vertices.size(); i ++) {
        Vertex* vert = new Vertex();
        vert->location = mesh.vertices[i];
        vertices.push_back(vert);
    }

    for (int i = 0; i < (mesh.indices.size()); i++) {
        bool quad = false;
        if (mesh.indices[i].w != -1) {
            quad = true;
        }
        Edge* edge1, *edge2, *edge3, *edge4;
        edge1 = new Edge();
        edge2 = new Edge();
        edge3 = new Edge();
        if (quad) edge4 = new Edge();
        
        Face* face = new Face();
        face->edge = edge1;

        edge1->face = face;
        edge2->face = face;
        edge3->face = face;
        if (quad) edge4->face = face;

        if (mesh.indices[i].x > vertices.size() || 
            mesh.indices[i].y > vertices.size() || 
            mesh.indices[i].z > vertices.size()) {

            std::cerr << "Invalid mesh; cannot convert to half edge structure." << std::endl;
            return;
        }
        if (quad) {
            if (mesh.indices[i].w > vertices.size()) {
                std::cerr << "Invalid mesh; cannot convert to half edge structure." << std::endl;
                return;
            }
        }

        edge1->vert = vertices[mesh.indices[i].y];
        edge2->vert = vertices[mesh.indices[i].z];
        edge3->vert = vertices[mesh.indices[i].x];
        if (quad) {
            edge1->vert = vertices[mesh.indices[i].w];
            edge2->vert = vertices[mesh.indices[i].x];
            edge3->vert = vertices[mesh.indices[i].y];
            edge4->vert = vertices[mesh.indices[i].z];
        }

        edgePairFinder.insert(std::pair<std::pair<int,int>, Edge*>(std::pair<int, int>(mesh.indices[i].x, mesh.indices[i].y), edge1));
        edgePairFinder.insert(std::pair<std::pair<int,int>, Edge*>(std::pair<int, int>(mesh.indices[i].y, mesh.indices[i].z), edge2));

        if (quad) {
            edgePairFinder.insert(std::pair<std::pair<int,int>, Edge*>(std::pair<int, int>(mesh.indices[i].z, mesh.indices[i].w), edge3));
            edgePairFinder.insert(std::pair<std::pair<int,int>, Edge*>(std::pair<int, int>(mesh.indices[i].w, mesh.indices[i].x), edge4));
        } else {
            edgePairFinder.insert(std::pair<std::pair<int,int>, Edge*>(std::pair<int, int>(mesh.indices[i].z, mesh.indices[i].x), edge3));
        }

        vertices[mesh.indices[i].x]->edge = edge1;
        vertices[mesh.indices[i].y]->edge = edge2;
        vertices[mesh.indices[i].z]->edge = edge3;
        if (quad) vertices[mesh.indices[i].w]->edge = edge4;
        
        edge1->next = edge2;
        edge2->next = edge3;

        if (quad) {
            edge3->next = edge4;
            edge4->next = edge1;
        } else {
            edge3->next = edge1;
        }


        faces.push_back(face);
        edges.push_back(edge1);
        edges.push_back(edge2);
        edges.push_back(edge3);
        
        if (quad) edges.push_back(edge4);

        face->normal = mesh.normals[i];
    }

    for (std::map<std::pair<int, int>, Edge*>::iterator iter = edgePairFinder.begin(); iter != edgePairFinder.end(); iter++) {
        Edge* edge = iter->second;
        int firstVertex = iter->first.first;
        int secondVertex = iter->first.second;

        std::map<std::pair<int, int>, Edge*>::iterator iterPair = edgePairFinder.find(std::pair<int, int>(secondVertex, firstVertex));
        if (iterPair != edgePairFinder.end()) {
            edge->pair = iterPair->second;
        } else {
            edge->pair = NULL;
        }
    }
    for (auto it = vertices.begin(); it != vertices.end(); it++) {
        if ((*it)->edge == nullptr) {
            delete *it;
            vertices.erase(it);
        }
    }
}

size_t HalfEdge::getNumVertices() {
    return vertices.size();
}

void HalfEdge::removeEdge(Edge* edge) {
    Vertex* from = edge->pair->vert;
    Vertex* zeroVert = new Vertex();
    zeroVert->location = glm::vec3(0.0f);
    for (auto checkEdge : edges) {
        if (checkEdge != edge && checkEdge->vert == edge->vert) {
            checkEdge->vert = from;
        }
    }
    // Note: Not deleting vertices at the moment
    //delete edge->vert;
    Face* face = edge->face;
    Edge* firstEdge = face->edge;
    Edge* currEdge = face->edge;

    Edge* match1 = edge->next->pair;
    Edge* match2 = edge->next->next->pair;

    match1->pair = match2;
    match2->pair = match1;

    do {
        currEdge->vert = zeroVert;
        currEdge = currEdge->next;
    } while (currEdge != firstEdge);

    // Linear search! Slow.
    for (auto it = faces.begin(); it != faces.end(); it++) {
        if (*it == face) {
            faces.erase(it);
            break;
        }
    }

    delete face;

    face = edge->pair->face;
    firstEdge = face->edge;
    currEdge = face->edge;

    match1 = edge->pair->next->pair;
    match2 = edge->pair->next->next->pair;

    match1->pair = match2;
    match2->pair = match1;

    do {
        currEdge->vert = zeroVert;
        currEdge = currEdge->next;
    } while (currEdge != firstEdge);

    for (auto it = faces.begin(); it != faces.end(); it++) {
        if (*it == face) {
            faces.erase(it);
            break;
        }
    }

    delete face;

    for (auto it = edges.begin(); it != edges.end();) {
        if ((*it)->vert == zeroVert) {
            delete *it;
            it = edges.erase(it);
        } else {
            it++;
        }
    }
    delete zeroVert;
}

void HalfEdge::decimate() {
    std::cout << "Decimating..." << std::endl;
    for (Edge* edge : edges) {
        if (edge->face->normal != edge->pair->face->normal) {
            edge->important = true;
        }
    }

    auto printLocation = [](glm::vec3 location) {
        std::cout << "Location x: " << location.x 
                  << " y: " << location. y 
                  << " z: " << location.z << std::endl;
    };

    auto checkIfEdge = [](Vertex* vertex) {
        Edge* firstEdge = vertex->edge;
        Edge* currEdge = firstEdge;
        bool onEdge = false;
        do {
            if (currEdge->face->normal != currEdge->pair->face->normal) {
                onEdge = true;
                break;
            }
            currEdge = currEdge->pair->next;
        } while (currEdge != firstEdge);
        return onEdge;
    };

    for (auto it = edges.begin(); it != edges.end();) {
        Edge* edge = *it;
            if (!checkIfEdge(edge->vert)) {
                removeEdge(edge);
                it = edges.begin();
                continue;
            } else {
                bool prevImportant = checkIfEdge(edge->pair->vert);

                if (prevImportant) {
                    glm::vec3 edgeDirection = edge->vert->location
                                            - edge->pair->vert->location;

                    Edge* nextEdge = edge->next;

                    do {
                        glm::vec3 nextDirection = nextEdge->vert->location
                                                - nextEdge->pair->vert->location;
                        if (nextDirection == edgeDirection) {
                            break;
                        }
                        nextEdge = nextEdge->pair->next;
                    } while (nextEdge != edge->next);

                    if (checkIfEdge(nextEdge->vert)) {
                        glm::vec3 nextDirection = nextEdge->vert->location
                                                - nextEdge->pair->vert->location;

                        if (edgeDirection == nextDirection) {
                            removeEdge(edge);
                            it = edges.begin();
                            continue;
                        } 
                    }
                }
            }
       // }
        it++;
    }

    std::cout << "done" << std::endl;
}

void HalfEdge::exportObj() {
    std::cout << "Exporting half edge" << std::endl;
    auto myHash = [](const glm::vec3& vert) {
        return std::hash<float>()(vert.x) 
             ^ std::hash<float>()(vert.y) 
             ^ std::hash<float>()(vert.z);
    };
    std::unordered_map<glm::vec3, std::vector<glm::vec3>, 
                       decltype(myHash)> sortedVertices(10, myHash);

    auto searchVector = [](const std::vector<glm::vec3>& vec, glm::vec3 vert) {
        for (auto vertex : vec) {
            if (vertex == vert) return true;
        }
        return false;
    };
    for (auto face : faces) {
        std::vector<glm::vec3> faceVertices;
        Edge* firstEdge = face->edge;
        Edge* currentEdge = face->edge;
        do {
            faceVertices.push_back(currentEdge->vert->location);
            currentEdge = currentEdge->next;
        } while (currentEdge != firstEdge);
        std::vector<glm::vec3>& currentVerts = sortedVertices[face->normal];
        for (glm::vec3 vertex : faceVertices) {
            if (!searchVector(currentVerts, vertex)) {
                currentVerts.push_back(vertex);
            }
        }
    }

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<int> indices;

    for (auto pair : sortedVertices) {
        glm::vec3 normal = pair.first;
        for (glm::vec3 vertex : pair.second) {
            vertices.push_back(vertex);
            normals.push_back(normal);
        }
    }

    for (auto face : faces) {
        std::vector<glm::vec3> faceVertices;
        Edge* firstEdge = face->edge;
        Edge* currentEdge = face->edge;
        do {
            faceVertices.push_back(currentEdge->vert->location);
            currentEdge = currentEdge->next;
        } while (currentEdge != firstEdge);
        for (auto vertex : faceVertices) {
            for (int i = 0; i < vertices.size(); i++) {
                if (vertices[i] == vertex && normals[i] == face->normal) {
                    indices.push_back(i);
                    break;
                }
            }
        }
    }




    std::ofstream outFile("test.obj");
    for (glm::vec3 vertex : vertices) {
        outFile << "v " << vertex.x 
                << " "  << vertex.y
                << " "  << vertex.z << std::endl;
    }
    outFile << std::endl;

    for (glm::vec3 normal : normals) {
        outFile << "vn " << normal.x 
                << " "   << normal.y 
                << " "   << normal.z << std::endl;
    }

    outFile << std::endl;

    for (int i = 0; i < indices.size(); i+=3) {
        // Add 1 to each index, as wavefront .obj 
        // files start indexing at 1, not 0
        outFile << "f " << indices[i] + 1
                << " "  << indices[i + 1] + 1 
                << " "  << indices[i + 2] + 1 << std::endl;
    }
}

Mesh* HalfEdge::toMesh() {
    int indexCounter = 0;
    Mesh* model = new Mesh();
    for (auto face : faces) {
        std::vector<Edge*> edges;

        Edge* firstEdge = face->edge;
        Edge* edge = firstEdge;
        do {
            edges.push_back(edge);
            edge = edge->next;
        } while (edge != firstEdge);

        if (edges.size() > 3) {
            model->vertices.push_back(edges[0]->vert->location);
            model->vertices.push_back(edges[1]->vert->location);
            model->vertices.push_back(edges[2]->vert->location);
            model->vertices.push_back(edges[3]->vert->location);

            model->normals.push_back(face->normal);
            model->normals.push_back(face->normal);
            model->normals.push_back(face->normal);
            model->normals.push_back(face->normal);

            model->indices.push_back(indexCounter + 0);
            model->indices.push_back(indexCounter + 1);
            model->indices.push_back(indexCounter + 2);

            model->indices.push_back(indexCounter + 0);
            model->indices.push_back(indexCounter + 2);
            model->indices.push_back(indexCounter + 3);

            indexCounter += 4;
        } else {
            model->vertices.push_back(edges[0]->vert->location);
            model->vertices.push_back(edges[1]->vert->location);
            model->vertices.push_back(edges[2]->vert->location);

            model->normals.push_back(face->normal);
            model->normals.push_back(face->normal);
            model->normals.push_back(face->normal);

            model->indices.push_back(indexCounter + 0);
            model->indices.push_back(indexCounter + 1);
            model->indices.push_back(indexCounter + 2);

            indexCounter += 3;
        }
    }
    return model;
}