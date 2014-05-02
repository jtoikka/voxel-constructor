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
#ifndef COMPONENT_H
#define COMPONENT_H

#include <string>
#include "../lib/glm/gtc/type_ptr.hpp"

class Component {

};

struct RenderComponent : Component {
	static const long key = 0x1;
	std::string modelName = "";
	bool wireframe = false;
	bool radiosity = false;
};

struct SpatialComponent : Component {
	static const long key = 0x2;
	glm::vec3 location = glm::vec3(0.0f, 0.0f, 0.0f);  // For a camera, this is polar coordinates
	float rotY; // Rotation about the y axis, in radians
	float scale = 1.0f;
};

struct ControllableComponent : Component {
	static const long key = 0x4;
	enum Type {block, player, camera, other};
	Type movementType;
	glm::vec3 direction;
	glm::vec3 rotation;
	bool jump = false;
};

struct CameraComponent : Component {
	static const long key = 0x8;
	glm::vec4 screenPos = glm::vec4(0.0, 0.0, 1.0, 1.0); // x and y coordinates in percentages where on screen to draw, and
											             // x and y values for what percentage of screen to draw
	std::string targetId = "";
	glm::vec3 targetLocation = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 polarCoordinates; // Radius, polar angle (zenith), azimuth angle
};

struct PhysicsComponent : Component {
	static const long key = 0x10;
	glm::vec3 velocity = glm::vec3(0.0f, 0.0f, 0.0f);
	float terminalVelocity = 0.0f;
	float maxVelocity = 0.0f;
	struct Colliding {
		bool wallXP = false;
		bool wallXN = false;
		bool wallZP = false;
		bool wallZN = false;

		bool floor = false;
		bool ceiling = false;
	} colliding;
};

struct CollisionComponent : Component {
	static const long key = 0x20;
	enum Type {world, player, other};
	Type collisionType;
};

#endif