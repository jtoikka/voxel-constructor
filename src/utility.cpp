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
#include "utility.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>

std::string Utility::programDirectory = "";

const std::string Utility::textFileRead(const std::string& fileName) {
    std::ifstream file(fileName.c_str());

    if (file.is_open()) {
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    } else {
        std::cerr << "ERROR: File " << fileName << " does not exist!" << std::endl;
    }
    return "";
}

glm::vec3 Utility::calcPolarCoordinates(glm::vec3 direction) {
    glm::vec3 polarCoordinates;
    polarCoordinates.x = glm::length(direction);
    polarCoordinates.y = acos(direction.z / polarCoordinates.x);
    if (direction.x < 0.0f) {
        polarCoordinates.y = 2 * PI - polarCoordinates.y;
    }
    polarCoordinates.z = 0.0f; // We'll be back to fix this, if necessary
    return polarCoordinates;
}

glm::mat4 Utility::calcRotation(glm::vec3 axis, float angle) {
    //const float PI = 3.14159265359;
    glm::mat4 rMatrix;

    float fAngle = angle;
    float fCos = cos(fAngle);
    float fSin = sin(fAngle);
    float fInvCos = 1.0f - fSin;

    rMatrix[0].x = (axis.x * axis.x) + (1 - axis.x * axis.x) * fCos;
    rMatrix[1].x = fInvCos * axis.x * axis.y + axis.z * fSin;
    rMatrix[2].x = fInvCos * axis.x * axis.z - axis.y * fSin;

    rMatrix[0].y = fInvCos * axis.x * axis.y - axis.z * fSin;
    rMatrix[1].y = (axis.y * axis.y) + (1 - axis.y * axis.y) * fCos;
    rMatrix[2].y = fInvCos * axis.y * axis.z + axis.x * fSin;

    rMatrix[0].z = fInvCos * axis.x * axis.z + axis.y * fSin;
    rMatrix[1].z = fInvCos * axis.y * axis.z - axis.x * fSin;
    rMatrix[2].z = (axis.z * axis.z) + (1 - axis.z * axis.z) * fCos;

    return rMatrix;
}

glm::mat4 Utility::calcXRotation(float angle) {
    glm::mat4 matrix(1.0f);

    matrix[1].y = cosf(angle); matrix[2].y = -sinf(angle);
    matrix[1].z = sinf(angle); matrix[2].z = cosf(angle);

    return matrix;
}

glm::mat4 Utility::calcYRotation(float angle) {
    glm::mat4 matrix(1.0f);

    matrix[0].x = cosf(angle); matrix[2].x = sinf(angle);
    matrix[0].z = -sinf(angle); matrix[2].z = cosf(angle);

    return matrix;
}

glm::mat4 Utility::calcZRotation(float angle) {
    glm::mat4 matrix(1.0f);

    matrix[0].x = cosf(angle); matrix[1].y = -sinf(angle);
    matrix[0].x = sinf(angle); matrix[1].y = cosf(angle);

    return matrix;
}

glm::mat4 Utility::translate(glm::vec3 location) {
    glm::mat4 translationMatrix(1.0f);
    translationMatrix[3] = glm::vec4(location, 1.0f);
    return translationMatrix;
}

glm::mat4 Utility::scale(float scale) {
    glm::mat4 scaleMatrix(1.0f);
    scaleMatrix[0].x = scale;
    scaleMatrix[1].y = scale;
    scaleMatrix[2].z = scale;
    return scaleMatrix;
}