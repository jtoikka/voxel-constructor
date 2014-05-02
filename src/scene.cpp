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
#include "scene.h"

#include "camera.h"
#include "utility.h"

Scene::Scene(std::string id, EventManager* eventManager) : 
                            id(id), eventManager(eventManager) {
    addTile(glm::ivec3(0, 0, 0));
    testScene();
    camera = new Entity("camera");

    CameraComponent* cameraComponent = new CameraComponent();

    cameraComponent->targetLocation = glm::vec3(tileDimensions) / 2.0f 
                                    - glm::vec3(0.5f);

    SpatialComponent* spatialComponent = new SpatialComponent();
    spatialComponent->location = cameraComponent->targetLocation 
                               - glm::vec3(0.0f, 0.0f, 30.0f);

    camera->addComponent(CameraComponent::key, cameraComponent);
    camera->addComponent(SpatialComponent::key, spatialComponent);

    listener = new Listener();

    eventManager->addListener(id, listener);
    buildVisibilityData();

    calcMaxBytes();
}

void Scene::calcMaxBytes() {
    maxColourIDBytes = ceil(log2
                            (getTileDimensions().x
                           * getTileDimensions().y
                           * getTileDimensions().z
                           * 6 + 1));

    std::cout << "Max bytes: " << maxColourIDBytes << std::endl;
}

unsigned int Scene::getMaxBytes() {
    return maxColourIDBytes;
}

unsigned int Scene::getTileID(Tile* tile) {
    for (int i = 0; i < tiles.size(); i++) {
        if (tiles[i] == tile) {
            return i;
        }
    }
    return 0;
}

Scene::Tile* Scene::addTile(glm::ivec3 location) {
    Tile* tile = new Tile();
    tile->blocks.resize(tileDimensions.x * tileDimensions.y * tileDimensions.z);
    tile->location = location;

    tiles.push_back(tile);
    return tile;
}

Scene::~Scene() {
    std::cout << "Deleting scene..." << std::endl;
    for (auto entityPair : entities) {
        delete entityPair.second;
    }
    delete camera;

    eventManager->removeListener(id);
    delete listener;

    std::cout << "Scene deleted." << std::endl;
}

void Scene::save(std::string fileName) {

}


/* Block removal -------------------------------------------------------------*/

bool Scene::removeBlock(glm::ivec3 location) {
    glm::ivec3 tileLocation;
    tileLocation.x = floor((float)location.x / tileDimensions.x);
    tileLocation.y = floor((float)location.y / tileDimensions.y);
    tileLocation.z = floor((float)location.z / tileDimensions.z);

    glm::ivec3 blockLocation = location;

    blockLocation.x = blockLocation.x % tileDimensions.x;
    blockLocation.y = blockLocation.y % tileDimensions.y;
    blockLocation.z = blockLocation.z % tileDimensions.z;

    blockLocation.x += blockLocation.x < 0 ? tileDimensions.x : 0;
    blockLocation.y += blockLocation.y < 0 ? tileDimensions.y : 0;
    blockLocation.z += blockLocation.z < 0 ? tileDimensions.z : 0;

    int index = blockLocation.x + blockLocation.y * tileDimensions.x 
              + blockLocation.z * tileDimensions.x * tileDimensions.y;

    Tile* tile = findTile(tileLocation);

    if (tile == nullptr) {
        return false;
    }

    if (tile->blocks[index].blockType == Block::BlockType::EMPTY) {
        return false;
    }
    tile->blocks[index] = emptyBlock;


    bool blockEmpty = true;
    for (auto block : tile->blocks) {
        if (block.blockType != Block::BlockType::EMPTY) {
            blockEmpty = false;
            break;
        }
    }
    if (blockEmpty) {
        removeTile(tile);
    }
    return true;
}

void Scene::removeTile(Scene::Tile* tile) {
    for (auto it = tiles.begin(); it != tiles.end(); it++) {
        if (*it == tile) {
            tiles.erase(it);
            break;
        }
    }
    delete tile;
}

Scene::Tile* Scene::findTile(glm::ivec3 location) {
    Tile* rTile = nullptr;
    for (Tile* tile : tiles) {
        if (tile->location == location) {
            rTile = tile;
            break;
        }
    }
    return rTile;
}

glm::vec3 Scene::getTileLocation(unsigned int index) {
    if (tiles.size() > index) {
        return glm::vec3(tiles[index]->location);
    }
    return glm::vec3(0.0f);
}

/*----------------------------------------------------------------------------*/

void Scene::update(double delta) {
    SpatialComponent* spatialComponent = (SpatialComponent*)camera
                                          ->getComponent(SpatialComponent::key);

    CameraComponent* cameraComponent = (CameraComponent*)camera
                                          ->getComponent(CameraComponent::key);

    for (auto event : listener->events) {
        switch (event->action) {
        case Action::START_MODIFYING : {
            auto eventVec3 = std::dynamic_pointer_cast<Event<glm::vec3>>(event);
            glm::vec3 location = eventVec3->args[0];
            glm::vec3 normal = eventVec3->args[1];
            unsigned int tileId = eventVec3->args[2].x;

            initialLocation = glm::ivec3(location);
            initialNormal = normal;
            modify = true;
            break;
        }
        case Action::STOP_MODIFYING : {
            modify = false;
            break;
        }
        case Action::MODIFY : {
            if (rotate) {
                auto eventVec3 = 
                    std::dynamic_pointer_cast<Event<glm::vec3>>(event);

                glm::vec2 cursorPosition = glm::vec2(eventVec3->args[2]);

                glm::vec2 displacement = cursorPosition - prevCursorPos;
                spatialComponent->location = 
                    Camera::swivel(cameraComponent->targetLocation, 
                                   spatialComponent->location, 
                                  -displacement.x / 50.0f, 
                                   displacement.y / 50.0f);
                prevCursorPos = cursorPosition;
            } else if (modify) {
                auto eventVec3 = 
                    std::dynamic_pointer_cast<Event<glm::vec3>>(event);

                glm::vec3 location = eventVec3->args[0];
                glm::vec3 normal = eventVec3->args[1];
                unsigned int tileId = eventVec3->args[2].x;

                modifyBlock(glm::ivec3(location), normal);
            }
            break;
        }
        case Action::START_ROTATING : {
            rotate = true;
            auto eventVec2 = std::dynamic_pointer_cast<Event<glm::vec2>>(event);
            prevCursorPos = eventVec2->args[0];
            break;
        }
        case Action::STOP_ROTATING : {
            rotate = false;
            break;
        }
        case Action::MODE_ADD : {
            mode = Mode::ADD;
            break;
        }
        case Action::MODE_REMOVE : {
            mode = Mode::REMOVE;
            break;
        }
        case Action::MODE_PAINT : {
            mode = Mode::PAINT;
            break;
        }
        case Action::BLOCK_CUBE : {
            currentBlock = Block::BlockType::CUBE;
            break;
        }
        case Action::BLOCK_SLOPE : {
            currentBlock = Block::BlockType::SLOPE;
            break;
        }
        case Action::BLOCK_RSLOPE : {
            currentBlock = Block::BlockType::RSLOPE;
            break;
        }
        case Action::BLOCK_DIAGONAL : {
            currentBlock = Block::BlockType::DIAGONAL;
            break;
        }
        case Action::BLOCK_CORNERSLOPE : {
            currentBlock = Block::BlockType::CORNERSLOPE;
            break;
        }
        case Action::BLOCK_RCORNERSLOPE : {
            currentBlock = Block::BlockType::RCORNERSLOPE;
            break;
        }
        case Action::BLOCK_INVCORNER : {
            currentBlock = Block::BlockType::INVCORNER;
            break;
        }
        case Action::BLOCK_RINVCORNER : {
            currentBlock = Block::BlockType::RINVCORNER;
            break;
        }
        case Action::ROTATE_BLOCK : {
            currentRotation += 1;
            currentRotation = currentRotation == 4 ? 0 : currentRotation;
            std::vector<unsigned int> args = {currentRotation};
            eventManager->addEvent({"state"}, Action::ROTATE_BLOCK, args);
            break;
        }
        default:
            std::cout << "Sent invalid event." << std::endl;
            break;
        }
    }
    listener->events.clear();
}

void Scene::modifyBlock(glm::ivec3 location, glm::vec3 normal) {
    bool samePlane = true;
    if (!Utility::closeEnough(normal, initialNormal, 5)) {
        samePlane = false;
    }
    if (fabs(initialNormal.x) > 0.001) {
        if (location.x != initialLocation.x) {
            samePlane = false;
        }
    }
    if (fabs(initialNormal.y) > 0.001) {
        if (location.y != initialLocation.y) {
            samePlane = false;
        }
    }
    if (fabs(initialNormal.z) > 0.001) {
        if (location.z != initialLocation.z) {
            samePlane = false;
        }
    }

    if (samePlane) {
        switch (mode) {
        case Mode::REMOVE : {
            glm::ivec3 tileLocation;
            tileLocation.x = floor((float)location.x / tileDimensions.x);
            tileLocation.y = floor((float)location.y / tileDimensions.y);
            tileLocation.z = floor((float)location.z / tileDimensions.z);
            if (removeBlock(location)) {
                std::vector<std::string> ids = {"renderer"};
                std::vector<Scene*> args = {this};
                eventManager->addEvent(ids, Action::REBUILD_TILE, args);
                bool alreadyAdded = false;
                for (glm::ivec3 modTileLocation : modifiedTiles) {
                    if (modTileLocation == tileLocation) {
                        alreadyAdded = true;
                        break;
                    }
                }
                if (!alreadyAdded) modifiedTiles.push_back(tileLocation);
            }
            break;
        }
        case Mode::ADD : {
            glm::ivec3 tileLocation;
            tileLocation.x = floor((float)(location.x + normal.x) / tileDimensions.x);
            tileLocation.y = floor((float)(location.y + normal.y) / tileDimensions.y);
            tileLocation.z = floor((float)(location.z + normal.z) / tileDimensions.z);
            addBlock(glm::vec3(location) + normal, 
                     currentBlock, currentRotation, false);

            std::vector<std::string> ids = {"renderer"};
            std::vector<Scene*> args = {this};
            eventManager->addEvent(ids, Action::REBUILD_TILE, args);
            bool alreadyAdded = false;
            for (glm::ivec3 modTileLocation : modifiedTiles) {
                if (modTileLocation == tileLocation) {
                    alreadyAdded = true;
                    break;
                }
            }
            if (!alreadyAdded) modifiedTiles.push_back(tileLocation);
            break;
        }
        case Mode::PAINT : {
            break;
        }
        default : 
            break;
        }
    }
}

void Scene::addEntity(Entity* entity) {
    entities.insert(std::pair<std::string, Entity*>(entity->getId(), entity));
}

void Scene::destroyEntity() {

}

void Scene::testScene() {
    if (tiles.size() == 0) {
        return;
    }
    for (size_t i = 0; i < tiles[0]->blocks.size(); i++) {
        int z = i / (tileDimensions.x * tileDimensions.y);
        int y = (i - (z * tileDimensions.x * tileDimensions.y)) / tileDimensions.x;
        int x = i - (z * tileDimensions.x * tileDimensions.y) - y * tileDimensions.x;

        addBlock(glm::vec3(x, y, z), Block::BlockType::CUBE, 0, false);
    }
}

void Scene::addBlock(glm::vec3 location, Block::BlockType blockType,
                     float rotation, bool flipped) {
    glm::ivec3 tileLocation;
    tileLocation.x = floor((float)location.x / tileDimensions.x);
    tileLocation.y = floor((float)location.y / tileDimensions.y);
    tileLocation.z = floor((float)location.z / tileDimensions.z);

    glm::ivec3 blockLocation = glm::ivec3(location);

    blockLocation.x = blockLocation.x % tileDimensions.x;
    blockLocation.y = blockLocation.y % tileDimensions.y;
    blockLocation.z = blockLocation.z % tileDimensions.z;

    blockLocation.x += blockLocation.x < 0 ? tileDimensions.x : 0;
    blockLocation.y += blockLocation.y < 0 ? tileDimensions.y : 0;
    blockLocation.z += blockLocation.z < 0 ? tileDimensions.z : 0;

    Tile* tile = findTile(tileLocation);

    if (tile == nullptr) {
        tile = addTile(tileLocation);
    }

    int index = blockLocation.x + blockLocation.y * tileDimensions.x
                       + blockLocation.z * tileDimensions.x * tileDimensions.y;


    tile->blocks[index].rotation = rotation;
    tile->blocks[index].flipped = flipped;
    tile->blocks[index].blockType = blockType;
}

Entity* Scene::getEntity(std::string entityId) {
    auto entityIterator = entities.find(entityId);
    if (entityIterator != entities.end())
        return entityIterator->second;
    return nullptr;
}

Scene::Tile* Scene::getTile(glm::ivec3 location) {
    return findTile(location);
}

std::vector<Scene::Tile*>& Scene::getTiles() {
    return tiles;
}

std::vector<glm::ivec3>& Scene::getModifiedTiles() {
    return modifiedTiles;
}

Scene::Block& Scene::getBlock(glm::ivec3 location) {
    glm::ivec3 tileLocation;
    tileLocation.x = floor((float)location.x / tileDimensions.x);
    tileLocation.y = floor((float)location.y / tileDimensions.y);
    tileLocation.z = floor((float)location.z / tileDimensions.z);

    glm::ivec3 blockLocation = glm::ivec3(location);

    blockLocation.x = blockLocation.x % tileDimensions.x;
    blockLocation.y = blockLocation.y % tileDimensions.y;
    blockLocation.z = blockLocation.z % tileDimensions.z;

    blockLocation.x += blockLocation.x < 0 ? tileDimensions.x : 0;
    blockLocation.y += blockLocation.y < 0 ? tileDimensions.y : 0;
    blockLocation.z += blockLocation.z < 0 ? tileDimensions.z : 0;

    Tile* tile = findTile(tileLocation);

    if (tile == nullptr) {
        return emptyBlock;
    }

    unsigned int index = blockLocation.x + blockLocation.y * tileDimensions.x + 
                         blockLocation.z * tileDimensions.x * tileDimensions.y;
    return tile->blocks[index];
}

void Scene::buildVisibilityData() {
    // Cube
    std::vector<std::vector<int>> cubeVisibility 
                            = {{0, 0, 0, 0, 0, 0},   // Front, right, back, left, bottom, top
                               {0, 0, 0, 0, 0, 0},
                               {0, 0, 0, 0, 0, 0},
                               {0, 0, 0, 0, 0, 0}};


    std::vector<std::vector<int>> slopeVisibility 
                            = {{1, SE, 0, SW, 0, 1},
                               {SW, 1, SE, 0, 0, 1},
                               {0, SW, 1, SE, 0, 1},
                               {SE, 0, SW, 1, 0, 1}};

    std::vector<std::vector<int>> diagonalVisibility
                            = {{1, 1, 0, 0, SW, NW},
                               {0, 1, 1, 0, NW, SW},
                               {0, 0, 1, 1, NE, SE},
                               {1, 0, 0, 1, SE, NE}};

    std::vector<std::vector<int>> rSlopeVisibility
                            = {{ 1, NE,  0, NW, 1, 0},
                               {NW,  1, NE,  0, 1, 0},
                               { 0, NW,  1, NE, 1, 0},
                               {NE,  0, NW,  1, 1, 0}};

    std::vector<std::vector<int>> cornerSlopeVisibility
                            = {{ 1,  1, SE, SW, SW, 1},
                               {SW,  1,  1, SE, NW, 1},
                               {SE, SW,  1,  1, NE, 1},
                               { 1, SE, SW,  1, SE, 1}};

    std::vector<std::vector<int>> rCornerSlopeVisibility
                            = {{ 1,  1, NE, NW, 1, NW},
                               {NW,  1,  1, NE, 1, SW},
                               {NE, NW,  1,  1, 1, SE},
                               { 1, NE, NW,  1, 1, NE}};

    std::vector<std::vector<int>> invCornerVisibility
                            = {{SW, SE,  0,  0, 0, NW},
                               { 0, SW, SE,  0, 0, SW},
                               { 0,  0, SW, SE, 0, SE},
                               {SE,  0,  0, SW, 0, NE}};

    std::vector<std::vector<int>> rInvCornerVisibility 
                            = {{NW, NE,  0,  0, SW, 0},   // Front, right, back, left, bottom, top
                               { 0, NW, NE,  0, NW, 0},
                               { 0,  0, NW, NE, NE, 0},
                               {NE,  0,  0, NW, SE, 0}};

    blockVisibilities.emplace(Block::BlockType::CUBE, cubeVisibility);
    blockVisibilities.emplace(Block::BlockType::SLOPE, slopeVisibility);
    blockVisibilities.emplace(Block::BlockType::RSLOPE, rSlopeVisibility);
    blockVisibilities.emplace(Block::BlockType::DIAGONAL, diagonalVisibility);
    blockVisibilities.emplace(Block::BlockType::CORNERSLOPE, cornerSlopeVisibility);
    blockVisibilities.emplace(Block::BlockType::RCORNERSLOPE, rCornerSlopeVisibility);
    blockVisibilities.emplace(Block::BlockType::INVCORNER, invCornerVisibility);
    blockVisibilities.emplace(Block::BlockType::RINVCORNER, rInvCornerVisibility);
}

int Scene::checkVisibility(glm::ivec3 blockLocation, glm::vec3 direction) {
    if (fabs(direction.x) + fabs(direction.y) + fabs(direction.z) != 1.0f) {
        return 1;
    }
    Block& blockToCheck = getBlock(blockLocation + glm::ivec3(direction));
    if (blockToCheck.blockType == Block::BlockType::EMPTY) {
        return 1;
    }
    unsigned int dir = 0;
    if (direction == glm::vec3(1, 0, 0)) dir = 1;
    else if (direction == glm::vec3(-1,  0,  0)) dir = 3;
    else if (direction == glm::vec3( 0,  1,  0)) dir = 4;
    else if (direction == glm::vec3( 0, -1,  0)) dir = 5;
    else if (direction == glm::vec3( 0,  0,  1)) dir = 0;
    else if (direction == glm::vec3( 0,  0, -1)) dir = 2;
    
    return blockVisibilities[blockToCheck.blockType][blockToCheck.rotation][dir];
}

int Scene::checkVisibilityDirection(glm::ivec3 blockLocation, glm::vec3 direction) {
    int visibilityValue = checkVisibility(blockLocation, direction);
    int dir = 0;
    Block blockToCheck = getBlock(blockLocation);
    if (     direction == glm::vec3( 1,  0,  0)) dir = 3;
    else if (direction == glm::vec3(-1,  0,  0)) dir = 1;
    else if (direction == glm::vec3( 0,  1,  0)) dir = 5;
    else if (direction == glm::vec3( 0, -1,  0)) dir = 4;
    else if (direction == glm::vec3( 0,  0,  1)) dir = 2;
    else if (direction == glm::vec3( 0,  0, -1)) dir = 0;
    if (visibilityValue != 0) {
        int ownVisibility 
            = blockVisibilities[blockToCheck.blockType][blockToCheck.rotation][dir];
        if (ownVisibility + visibilityValue != 0)
            return blockVisibilities[blockToCheck.blockType][blockToCheck.rotation][dir];
    }
    return -1;
}

std::vector<glm::vec3> Scene::checkVisibility(glm::ivec3 blockLocation) {
    std::vector<glm::vec3> directions;
    Block blockToCheck = getBlock(blockLocation);
    if (blockToCheck.blockType == Block::BlockType::EMPTY) {
        return directions;
    }
    glm::vec3 direction;
    int dir;
    int visibility;

    direction = glm::vec3( 1,  0,  0);
    dir = 3;
    visibility = checkVisibility(blockLocation, direction);
    if (visibility != 0) {
        int ownVisibility 
            = blockVisibilities[blockToCheck.blockType][blockToCheck.rotation][dir];

        if (abs(ownVisibility + visibility) != 1 || ownVisibility == 0)
            directions.push_back(glm::vec3( 1,  0,  0));
    }

    direction = glm::vec3(-1,  0,  0);
    dir = 1;
    visibility = checkVisibility(blockLocation, direction);
    if (visibility != 0) {
        int ownVisibility 
            = blockVisibilities[blockToCheck.blockType][blockToCheck.rotation][dir];

        if (abs(ownVisibility + visibility) != 1 || ownVisibility == 0)
            directions.push_back(glm::vec3(-1,  0,  0));
    }

    direction = glm::vec3( 0,  1,  0);
    dir = 5;
    visibility = checkVisibility(blockLocation, direction);
    if (visibility != 0) {
        int ownVisibility 
            = blockVisibilities[blockToCheck.blockType][blockToCheck.rotation][dir];

        if (ownVisibility + visibility != 0)
            directions.push_back(glm::vec3( 0,  1,  0));
    }

    direction = glm::vec3( 0,  -1,  0);
    dir = 4;
    visibility =    checkVisibility(blockLocation, direction);
    if (visibility != 0) {
        int ownVisibility 
            = blockVisibilities[blockToCheck.blockType][blockToCheck.rotation][dir];

        if (ownVisibility + visibility != 0)
            directions.push_back(glm::vec3( 0,  -1,  0));
    }

    direction = glm::vec3( 0,  0,  1);
    dir = 2;
    visibility = checkVisibility(blockLocation, direction);
    if (visibility != 0) {
        int ownVisibility 
            = blockVisibilities[blockToCheck.blockType][blockToCheck.rotation][dir];

        if (abs(ownVisibility + visibility) != 1 || ownVisibility == 0)
            directions.push_back(glm::vec3( 0,  0,  1));
    }

    direction = glm::vec3( 0,  0,  -1);
    dir = 0;
    visibility = checkVisibility(blockLocation, direction);
    if (visibility != 0) {
        int ownVisibility 
            = blockVisibilities[blockToCheck.blockType][blockToCheck.rotation][dir];

        if (abs(ownVisibility + visibility) != 1 || ownVisibility == 0)
            directions.push_back(glm::vec3( 0,  0,  -1));
    }

    return directions;
}

boost::unordered_map<std::string, Entity*>& Scene::getEntities() {
    return entities;
}

Entity* Scene::getCamera() {
    return camera;
}

glm::ivec3 Scene::getBlockLocation(size_t index) {
    int z = index / (tileDimensions.x * tileDimensions.y);
    int y = (index - (z * tileDimensions.x * tileDimensions.y)) / tileDimensions.x;
    int x = index - (z * tileDimensions.x * tileDimensions.y) - y * tileDimensions.x;

    return glm::ivec3(x, y, z);
}

glm::ivec3 Scene::getTileDimensions() {
    return tileDimensions;
}