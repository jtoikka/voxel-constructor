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
#include "trimesh.h"

TriMesh::TriMesh(BasicMesh baseMesh) {
    mesh.request_face_normals();
    if (!mesh.has_face_normals()) {
        std::cerr << "ERROR: Standard face property 'Normals' \
                      not available!" << std::endl;
    }
    std::vector<MyMesh::VertexHandle> vHandle;
    vHandle.resize(baseMesh.vertices.size());
    for (int i = 0; i < baseMesh.vertices.size(); i++) {
        glm::vec3 vert = baseMesh.vertices[i];
        auto point = MyMesh::Point(vert.x, vert.y, vert.z);
        vHandle[i] = mesh.add_vertex(point);
    }

    std::vector<MyMesh::VertexHandle> fHandle;
    for (int i = 0; i < baseMesh.indices.size(); i++) {
        // Note: this class focuses only on triangulated meshes.
        fHandle.clear(); 
        glm::ivec4 face = baseMesh.indices[i];
        fHandle.push_back(vHandle[face.x]);
        fHandle.push_back(vHandle[face.y]);
        fHandle.push_back(vHandle[face.z]);
        auto faceHandle = mesh.add_face(fHandle);
        glm::vec3 normal = baseMesh.normals[i];
        mesh.set_normal(faceHandle,
                        MyMesh::Point(normal.x, normal.y, normal.z));
    }

    // Define write options
    //wopt += OpenMesh::IO::Options::VertexNormal;
}

void TriMesh::decimate() {
    Decimater decimater(mesh);
    HModNormal hModNormal;

    decimater.add(hModNormal);

    std::cout << decimater.module( hModNormal ).name() << std::endl;

    decimater.module(hModNormal).set_normal_deviation(1);  
    decimater.module(hModNormal).set_binary(false);  

    decimater.initialize();
    std::cout << "Max normal deviation: " << decimater.module(hModNormal).normal_deviation() << std::endl;
    std::cout << "Decimated n: " << decimater.decimate() << std::endl;
    mesh.garbage_collection();
}

void TriMesh::writeObj() {
    std::cout << "Writing to obj" << std::endl;
    //mesh.request_vertex_normals();
    //mesh.update_normals();
    OpenMesh::IO::write_mesh(mesh, "output.obj", wopt);
}