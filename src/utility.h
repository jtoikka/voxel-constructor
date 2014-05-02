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
#ifndef UTILITY_H
#define UTILITY_H

#include "../lib/glm/gtc/type_ptr.hpp"

#include <vector>
#include <string>

namespace Utility {
    const float PI = 3.141592653589793238;

	const std::string textFileRead(const std::string& fileName);

    extern std::string programDirectory;

    glm::vec3 calcPolarCoordinates(glm::vec3 direction);

	glm::mat4 calcRotation(glm::vec3 axis, float angle);

    glm::mat4 calcXRotation(float angle);

    glm::mat4 calcYRotation(float angle);

    glm::mat4 calcZRotation(float angle);

    glm::mat4 translate(glm::vec3 location);

    glm::mat4 scale(float scale);

    template <typename Type>
    bool closeEnough(Type A, Type B, int decimalPlaces) {
        size_t arraySize = sizeof(A) / sizeof(A[0]);
        int decimalShift = pow(10, decimalPlaces);
        for (size_t i = 0; i < arraySize; i++) {
            if (round(A[i] * decimalShift) != round(B[i] * decimalShift)) {
                return false;
            }
        }
        return true;
    }

    template <typename Type>
	void pushBack(std::vector<Type>& vector, Type x, Type y, Type z) {
	    vector.push_back(x);
	    vector.push_back(y);
	    vector.push_back(z);
	}

	template <typename Type>
	void pushBack(std::vector<Type>& vector, Type x, Type y, Type z, Type w) {
	    vector.push_back(x);
	    vector.push_back(y);
	    vector.push_back(z);
	    vector.push_back(w);
	}
};

#endif