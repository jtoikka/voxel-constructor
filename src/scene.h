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
#ifndef SCENE_H
#define SCENE_H

#include <boost/unordered_map.hpp>
#include <string>

#include "entity.h"
#include "eventManager.h"

class Scene {
public:
    // For checking triangle visibility
    static const int NE =  3;
    static const int SE = -3;
    static const int SW =  4;
    static const int NW = -4;

    typedef struct Block {
        char rotation = 0; // About the y axis
        bool flipped = false; // Is it upside-down?
        enum class BlockType {
            CUBE, 
            SLOPE, RSLOPE,
            CORNERSLOPE, RCORNERSLOPE,
            INVCORNER, RINVCORNER,
            DIAGONAL, 
            DIAGONALCORNER, RDIAGONALCORNER,
            EMPTY};
        BlockType blockType = BlockType::EMPTY;
    } Block;

    typedef struct Tile {
        std::vector<Block> blocks;
        glm::ivec3 location;
    } Tile;

    std::vector<Tile*> tiles;

    Scene(std::string id, EventManager* eventManager);

    ~Scene();

    void update(double delta);

    void addBlock(glm::vec3 location, Block::BlockType blockType, float rotation, bool flipped);

	void addEntity(Entity* entity);

	void destroyEntity();

	Entity* getEntity(std::string entityId);

	boost::unordered_map<std::string, Entity*>& getEntities();

	Entity* getCamera();

    Tile* getTile(glm::ivec3 location);

    unsigned int getTileID(Tile*);

    std::vector<Tile*>& getTiles();

    Block& getBlock(glm::ivec3 blockLocation);

    glm::ivec3 getTileDimensions();

    std::vector<glm::vec3> checkVisibility(glm::ivec3 blockLocation);

    int checkVisibility(glm::ivec3 blockLocation, glm::vec3 direction);

    int checkVisibilityDirection(glm::ivec3 blockLocation, glm::vec3 direction);

    glm::ivec3 getBlockLocation(size_t index);

    void startRotating(glm::vec2 cursorPosition);

    void stopRotating();

    bool removeBlock(glm::ivec3 location);

    std::vector<glm::ivec3>& getModifiedTiles();

    unsigned int getMaxBytes();

    glm::vec3 getTileLocation(unsigned int index);

    void save(std::string fileName);

private:
    std::string id;
    boost::unordered_map<std::string, Entity*> entities;
    glm::ivec3 tileDimensions = glm::ivec3(8, 8, 8);
    Entity* camera;

    std::vector<glm::ivec3> modifiedTiles;

    EventManager* eventManager;
    Listener* listener;

    unsigned int maxColourIDBytes;

    std::map<Block::BlockType, std::vector<std::vector<int>>> blockVisibilities;

    Block emptyBlock;

    bool rotate = false;
    glm::vec2 prevCursorPos;

    bool modify = false;
    glm::ivec3 initialLocation;
    glm::vec3 initialNormal;

    enum class Mode {
        ADD,
        REMOVE,
        PAINT
    };

    Mode mode = Mode::REMOVE;

    Block::BlockType currentBlock = Block::BlockType::CUBE;
    unsigned int currentRotation = 0;

    void testScene();

    void modifyBlock(glm::ivec3 location, glm::vec3 normal);

    void buildVisibilityData();

    Tile* addTile(glm::ivec3 location);

    Tile* findTile(glm::ivec3 location);

    void calcMaxBytes();

    void removeTile(Tile* tile);
};

#endif