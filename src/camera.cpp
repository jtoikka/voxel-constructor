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
#include "camera.h"
#include "utility.h"

#include <iostream>

glm::vec3 Camera::swivel(glm::vec3 target, glm::vec3 currentLocation,
                         float rotationHorizontal, float rotationVertical) {
    glm::vec3 direction = currentLocation - target;

    float radius = glm::length(direction);
    float theta = asin(direction.y / radius);
    float phi = acos(glm::normalize(glm::vec3(direction.x, 0.0f, direction.z)).x);

    theta += rotationVertical;

    if (theta > (Utility::PI / 2.1f)) {
        theta = (Utility::PI / 2.1f);
    } else if (theta < -(Utility::PI / 2.1f)) {
        theta = -(Utility::PI / 2.1f);
    }

    if (direction.z > 0.0f) {
        phi = 2 * Utility::PI - phi;
    }

    phi += rotationHorizontal;

    glm::vec3 newLocation;
    newLocation.x = cos(phi) * cos(theta) * radius + target.x;
    newLocation.y = sin(theta) * radius + target.y;
    newLocation.z = -sin(phi) * cos(theta) * radius + target.z;

    return newLocation;
}

glm::vec3 Camera::swivel(glm::vec3 target, glm::vec3 currentLocation, float rotation) {
    glm::vec3 direction = currentLocation - target;
    float radius = glm::length(direction);
    float currentAngleX = acos(direction.x / radius);
    float currentAngleY = asin(direction.y / radius);

    if (direction.z > 0.01f) {
        currentAngleX = 2 * Utility::PI - currentAngleX;
    }

    glm::vec3 newLocation;
    newLocation.x = cos(currentAngleX + rotation) * cos(currentAngleY) * radius + target.x;
    newLocation.y = currentLocation.y;
    newLocation.z = -sin(currentAngleX + rotation)* cos(currentAngleY) * radius + target.z;

    return newLocation;
}

glm::vec3 Camera::swivelVertical(glm::vec3 target, glm::vec3 currentLocation, float rotation) {
    glm::vec3 direction = currentLocation - target;
    float radius = glm::length(direction);
    float currentAngleY = asin(direction.y / radius);
    float currentAngleX = acos(direction.x / radius);

    float totalAngle = currentAngleY + rotation;
    if (totalAngle >= Utility::PI / 2.0f) {
        totalAngle = Utility::PI / 2.0f - 0.00001f;
    } else if (totalAngle <= -Utility::PI / 2.0f) {
        totalAngle = -Utility::PI / 2.0f - 0.00001f;
    }

    if (direction.z > 0.01f) {
        currentAngleX = 2 * Utility::PI - currentAngleX;
    }

    glm::vec3 newLocation;
    newLocation.x = cos(currentAngleX) * cos(totalAngle) * radius + target.x;
    newLocation.y = sin(totalAngle) * radius + target.y;
    newLocation.z = -sin(currentAngleX) * cos(totalAngle) * radius + target.z;

    return newLocation;
}