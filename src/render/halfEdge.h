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
#ifndef HALFEDGE_H
#define HALFEDGE_H

#include <vector>
#include <string>
#include "mesh.h"
#include "../../lib/glm/gtc/type_ptr.hpp"

struct BasicMesh {
    std::vector<glm::vec3> vertices;
    std::vector<glm::ivec4> indices;
    std::vector<glm::vec3> normals;
};

class HalfEdge {
public:
/* Struct declarations -------------------------------------------------------*/
    struct Vertex;
    struct Face;

    struct Edge {
        Vertex* vert;
        Edge* pair;
        Face* face;
        Edge* next;

        bool important = false;
    };

    struct Vertex {
        glm::vec3 location;

        Edge* edge;
    };

    struct Face {
        Edge* edge;
        glm::vec3 normal;
        glm::vec4 colour;

        unsigned int id;
    };
/*----------------------------------------------------------------------------*/
    HalfEdge(const std::string& modelDirectory, const std::string& meshName);

    HalfEdge(BasicMesh baseMesh);

    void decimate();

    void exportObj();

    ~HalfEdge();

    Mesh* toMesh();

    size_t getNumVertices();

    void removeEdge(Edge* edge);

private:
    std::vector<Vertex*> vertices;
    std::vector<Edge*> edges;
    std::vector<Face*> faces;

    void loadModel(const std::string& modelDirectory, const std::string& modelName);

    void populateHalfEdgeStructure(const BasicMesh& mesh);
};


#endif