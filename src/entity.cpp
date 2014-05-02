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
#include "entity.h"

Entity::Entity(std::string id) : id(id){

}

Entity::~Entity() {
    for (auto componentPair : components) {
        delete componentPair.second;
    }
}

void Entity::addComponent(long key, Component* component) {
    components.insert(std::pair<long, Component*>(key, component));
    this->key = this->key | key;
}

long Entity::getKey() {
    return key;
}

Component* Entity::getComponent(long key) {
	std::map<long, Component*>::const_iterator iter = components.find(key);
	if (iter != components.end()) {
		return iter->second;
	}
    return nullptr;
}

std::string Entity::getId() {
	return id;
}