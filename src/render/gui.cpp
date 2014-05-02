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
#include "gui.h"

/* GUI -----------------------------------------------------------------------*/
GUI::GUI(glm::vec2 dimensions) : dimensions(dimensions) {
    screen = new GuiContainer(dimensions, nullptr);
    containers.emplace("screen", screen);
}

void GUI::draw(Render2D* renderer) {
    for (auto pair : containers) {
        pair.second->draw(renderer);
    }
}

void GUI::resize(int width, int height) {
    dimensions.x = width;
    dimensions.y = height;

    for (auto containerPair : containers) {
        containerPair.second->resize(width, height);    
    }

    screen->dimensions = glm::vec2(width, height);
}

bool GUI::input(glm::vec2 cursorLocation, int button, int state, int mods, 
                Renderer* renderer, EventManager* eventManager) {
    for (auto child : containers) {
        if (child.second != nullptr) {
            if (renderer == nullptr || eventManager == nullptr) {
            }
        }
        child.second->input(cursorLocation, button, state, 
                           mods, renderer, eventManager);
    }
    return true;
}

void GUI::addContainer(glm::vec2 dimensions, std::string name, 
                       std::string parent, glm::vec4 colour,
                       glm::vec4 padding, glm::ivec4 alignment,
                       std::string alignComponent) {

    auto iterator = containers.find(parent);
    GuiContainer* parentContainer = nullptr;
    glm::vec4 parentDimensions;
    if (iterator != containers.end()) {
        parentContainer = iterator->second;
        parentDimensions = parentContainer->getHardDimensions();
    }

    glm::vec4 totalPadding = padding;
    if (padding.y == 1.0f && padding.w == 1.0f) {
        float width = parentDimensions.z - parentDimensions.x - dimensions.x;
        totalPadding.y = width / 2.0f;
        totalPadding.w = width / 2.0f;
    }

    // Check if centered
    if (alignment.y == 0 && alignment.w == 0) {
        totalPadding.w += ((parentDimensions.z - parentDimensions.x) 
                          - dimensions.x) / 2;
    }

    iterator = containers.find(alignComponent);
    if (iterator != containers.end()) {
        glm::vec4 hardAlign = iterator->second->getHardDimensions();
        totalPadding.w += (hardAlign.z) * (alignment.w > 0 ? 1 : 0);
        totalPadding.x += (hardAlign.w - parentDimensions.y) * alignment.x;
    }
    GuiContainer* container = new GuiContainer(dimensions, parentContainer, 
                                               alignment, colour, totalPadding);

    containers.insert(std::pair<std::string, GuiContainer*>(name, container));
}

void GUI::addScene(glm::vec2 dimensions, Scene* scene, std::string parent) {
    auto iterator = containers.find(parent);
    if (iterator != containers.end()) {
        GuiScene* guiScene = new GuiScene(dimensions, scene, iterator->second);
        iterator->second->addChild(guiScene);
    }
}

void GUI::addButtonLinker(glm::vec2 dimensions, std::string name, 
                          std::string parent, glm::vec4 colour,
                          glm::vec4 padding, glm::ivec4 alignment,
                          std::string alignComponent) {
    auto iterator = containers.find(parent);
    GuiContainer* parentContainer = nullptr;
    glm::vec4 parentDimensions;
    if (iterator != containers.end()) {
        parentContainer = iterator->second;
        parentDimensions = parentContainer->getHardDimensions();
    }

    glm::vec4 totalPadding = padding;
    if (padding.y == 1.0f && padding.w == 1.0f) {
        float width = parentDimensions.z - parentDimensions.x - dimensions.x;
        totalPadding.y = width / 2.0f;
        totalPadding.w = width / 2.0f;
    }

    // Check if centered
    if (alignment.y == 0 && alignment.w == 0) {
        totalPadding.w += ((parentDimensions.z - parentDimensions.x) 
                          - dimensions.x) / 2;
    }

    iterator = containers.find(alignComponent);
    if (iterator != containers.end()) {
        glm::vec4 hardAlign = iterator->second->getHardDimensions();
        totalPadding.w += (hardAlign.z) * alignment.w;
        totalPadding.x += (hardAlign.w - parentDimensions.y) * alignment.x;
    }
    GuiButtonLinker* container = new GuiButtonLinker(
                                               dimensions, parentContainer, 
                                               alignment, colour, totalPadding);
    containers.insert(std::pair<std::string, GuiContainer*>(name, container));
}

void GUI::addButton(glm::vec2 dimensions, Textbuffer* textBuffer, std::wstring text,
                    std::string parent,
                    std::shared_ptr<EventInterface> event,
                    glm::vec4 colourA, glm::vec4 colourB, glm::vec4 padding, 
                    glm::ivec4 alignment, std::string alignComponent, 
                    bool toggleable) {

    auto iterator = containers.find(parent);
    if (iterator != containers.end()) {
        GuiContainer* parentContainer = iterator->second;
        glm::vec4 parentDimensions = parentContainer->getHardDimensions();
        glm::vec4 totalPadding = padding;
        if (padding.y == 1.0f && padding.w == 1.0f) {
            float width = parentDimensions.z - parentDimensions.x - dimensions.x;
            totalPadding.y = width / 2.0f;
            totalPadding.w = width / 2.0f;
        }
        if (alignment.x == 1) {
            if (iterator->second->children.size() > 0) {
                glm::vec4 prevDimensions = iterator->second
                                         ->children.back()
                                         ->getHardDimensions();

                totalPadding.x += prevDimensions.w - parentDimensions.y;
            }
        }
        if (alignment.w == 1) {
            if (iterator->second->children.size() > 0) {
                glm::vec4 prevDimensions = iterator->second
                                         ->children.back()
                                         ->getHardDimensions();

                totalPadding.w += prevDimensions.z;
            }
        }
        GuiButton* button = new GuiButton(dimensions, iterator->second, event, 
                                          alignment, totalPadding, textBuffer, 
                                          text, colourA, colourB, toggleable);
        parentContainer->addChild(button);
    }
}

void GUI::addConsole(glm::vec2 dimensions, std::string parent,
                     Textbuffer* textbuffer, glm::ivec4 alignment, 
                     glm::vec4 padding, glm::vec4 colour) {
    auto iterator = containers.find(parent);
    if (iterator != containers.end()) {
        GuiContainer* parentContainer = parentContainer = iterator->second;
        glm::vec4 parentDimensions = parentContainer->getHardDimensions();
        glm::vec4 totalPadding = padding;
        if (padding.y == 1.0f && padding.w == 1.0f) {
            float parentWidth = parentDimensions.z - parentDimensions.x;
            float width;
            if (dimensions.x > 1.0f)
                width = parentWidth - dimensions.x;
            else {
                width = parentWidth * (1.0f - dimensions.x);
            }
            totalPadding.y = width / 2.0f;
            totalPadding.w = width / 2.0f;
        }
        if (alignment.x == 1) {
            if (iterator->second->children.size() > 0) {
                glm::vec4 prevDimensions = iterator->second
                                         ->children.back()
                                         ->getHardDimensions();

                totalPadding.x += prevDimensions.w - parentDimensions.y;
            }
        }
        if (alignment.w == 1) {
            if (iterator->second->children.size() > 0) {
                glm::vec4 prevDimensions = iterator->second
                                         ->children.back()
                                         ->getHardDimensions();

                totalPadding.w += prevDimensions.z;
            }
        }
        GuiConsole* console = new GuiConsole(dimensions, parentContainer, textbuffer,
                                             alignment, totalPadding, colour);
        parentContainer->addChild(console);

        glm::vec4 hardDimensions = console->getHardDimensions();

        // std::cout << "totalPadding w: " << totalPadding.w << std::endl;

        // std::cout << "!!!!! hard dimensions x: " << hardDimensions.x
        //           << " y: " << hardDimensions.y 
        //           << " z: " << hardDimensions.z 
        //           << " w: " << hardDimensions.w << std::endl;
    }

}
/* ---------------------------------------------------------------------------*/

/* GUI container -------------------------------------------------------------*/

GuiContainer::GuiContainer(glm::vec2 dimensions, GuiContainer* parent,
                           glm::ivec4 alignment, glm::vec4 colour, 
                           glm::vec4 padding) : 
                               dimensions(dimensions), 
                               colour(colour), 
                               parent(parent),
                               padding(padding),
                               alignment(alignment) { }

glm::vec4 GuiContainer::getHardDimensions() {
    if (hardDimensions != glm::vec4(0.0f)) {
        return hardDimensions;
    }
    glm::vec4 parentDimensions(0.0f);
    if (parent != nullptr) {
        parentDimensions = parent->getHardDimensions();
    }

    if (alignment.z) {
        hardDimensions = glm::vec4(parentDimensions.x, 
                                   parentDimensions.w - dimensions.y, 
                                   parentDimensions.z, parentDimensions.w);
 
        // std::cout << "!!!!! hard dimensions x: " << hardDimensions.x
        //           << " y: " << hardDimensions.y 
        //           << " z: " << hardDimensions.z 
        //           << " w: " << hardDimensions.w << std::endl;

        return hardDimensions;
    }

    hardDimensions.x = parentDimensions.x;
    hardDimensions.y = parentDimensions.y;

    hardDimensions.x += padding.w;
    hardDimensions.y += padding.x;

    if (dimensions.x <= 1.0f && dimensions.x > 0.0f) {
        hardDimensions.z = parentDimensions.z * dimensions.x;
    } else {
        hardDimensions.z = hardDimensions.x + dimensions.x;
    }
    if (dimensions.y <= 1.0f && dimensions.y > 0.0f) {
        hardDimensions.w = parentDimensions.w * dimensions.y - padding.z;
    } else {
        hardDimensions.w = hardDimensions.y + dimensions.y;
    }
    return hardDimensions;
}

void GuiContainer::draw(Render2D* renderer) {
    glm::vec4 hardDimensions = getHardDimensions();
    renderer->drawRectangle(glm::vec2(hardDimensions), 
                            glm::vec2(hardDimensions.z, hardDimensions.w)
                          - glm::vec2(hardDimensions), colour);
    for (auto child : children) {
        child->draw(renderer);
    }
}

bool GuiContainer::input(glm::vec2 cursorLocation, int button, int state, 
                         int mods, Renderer* renderer, 
                         EventManager* eventManager) {
    bool childHit = false;
    glm::vec4 hardDimensions = getHardDimensions();
    if (cursorLocation.x < hardDimensions.z 
     && cursorLocation.x > hardDimensions.x
     && cursorLocation.y < hardDimensions.w
     && cursorLocation.y > hardDimensions.y) {
        for (auto child : children) {
            if (child->input(cursorLocation, button, state, 
                     mods, renderer, eventManager)) {
            childHit = true;
            }
        }
    }
    return childHit;
}

void GuiContainer::addChild(GuiComponent* component) {
    children.push_back(component);
}

void GuiContainer::resize(int width, int height) {
    hardDimensions = glm::vec4(0.0f);
    for (auto child : children) {
        child->resize(width, height);
    }
}
/* ---------------------------------------------------------------------------*/

/* GUI Button Linker -------------------------------------------------------- */

GuiButtonLinker::GuiButtonLinker(glm::vec2 dimensions, GuiContainer* parent,
                              glm::ivec4 alignment, glm::vec4 colour, 
                              glm::vec4 padding) : super(dimensions, parent, 
                                                         alignment, colour, 
                                                         padding) { }

bool GuiButtonLinker::input(glm::vec2 coordinates, int button, 
                            int state, int mods, Renderer* renderer, 
                            EventManager* eventManager) {
    glm::vec4 hardDimensions = getHardDimensions();

    if (coordinates.x < hardDimensions.z 
     && coordinates.x > hardDimensions.x
     && coordinates.y < hardDimensions.w
     && coordinates.y > hardDimensions.y) {

        for (auto child : children) {
            if (child->input(coordinates, button, state, 
                         mods, renderer, eventManager)) {
                if (button > 0) {
                    for (auto child2 : children) {
                        if (child != child2)
                            ((GuiButton*)child2)->resetState();
                    }
                    break;
                }
            }
        }
        return true;
    }
    return false;
}
/* ---------------------------------------------------------------------------*/

/* GUI Component -------------------------------------------------------------*/
GuiComponent::GuiComponent(glm::vec2 dimensions, GuiContainer* parent,
                           glm::ivec4 alignment, 
                           glm::vec4 colour, 
                           glm::vec4 padding) : super(dimensions, parent, 
                                                      alignment, colour, 
                                                      padding) { }
// /*-------------------------------------------------------------------------*/

// /* GUI Button -------------------------------------------------------------*/
GuiButton::GuiButton(glm::vec2 dimensions, GuiContainer* parent, 
                     std::shared_ptr<EventInterface> event,
                     glm::ivec4 alignment, glm::vec4 padding,
                     Textbuffer* textBuffer, std::wstring text, 
                     glm::vec4 colourA, glm::vec4 colourB, bool toggleable) : 
                            super(dimensions, parent, alignment, colourA, padding), 
                            event(event), text(text), textBuffer(textBuffer),
                                                    toggleable(toggleable) {

    this->colourA = colourA;
    this->colourB = colourB;
}

void GuiButton::resetState() {
    colour = colourA;
}


void GuiButton::draw(Render2D* renderer) {
    std::wstring textToDraw;
    if (textBuffer != nullptr) {
        textToDraw = textBuffer->getText();
    } else {
        textToDraw = text;
    }
    if (textDimensions == glm::vec2(0.0f)) {
        glm::vec4 boundingBox = renderer->getTextBoundingBox(textToDraw);
        this->textDimensions = glm::vec2(boundingBox.z - boundingBox.x, 
                                         boundingBox.w - boundingBox.y);
    }
    glm::vec4 hardDimensions = this->getHardDimensions();

    renderer->drawRectangle(glm::vec2(hardDimensions), 
                            glm::vec2(hardDimensions.z, hardDimensions.w)
                          - glm::vec2(hardDimensions.x, hardDimensions.y), 
                            colour);

    float width = hardDimensions.z - hardDimensions.x;
    float height = hardDimensions.w - hardDimensions.y;

    glm::vec2 textLocation = glm::vec2(hardDimensions) 
                           + glm::vec2(width / 2 - textDimensions.x / 2,
                                       textDimensions.y + 
                                       (height - textDimensions.y) / 2);

    textLocation.x = floor(textLocation.x);  // Round down to ensure pixels are
    textLocation.y = floor(textLocation.y);  // centered for better font 
                                             // rendering
    renderer->drawText(textToDraw, textLocation);
}

bool GuiButton::input(glm::vec2 coordinates, int button, int state, 
           int mods, Renderer* renderer, EventManager* eventManager) {
    glm::vec4 ownDimensions = getHardDimensions();
    if (coordinates.x <= ownDimensions.z 
        && coordinates.x >= ownDimensions.x 
        && coordinates.y <= ownDimensions.w 
        && coordinates.y >= ownDimensions.y) {
        if (state == 1) {
            eventManager->addEvent(event);
            if (colour == colourA) {
                colour = colourB;
            } else {
                colour = colourA;
            }
        }
        return true;
    }
    return false;
}
// /*-------------------------------------------------------------------------*/

/* GUI Console ---------------------------------------------------------------*/
GuiConsole::GuiConsole(glm::vec2 dimensions, GuiContainer* parent,
                       Textbuffer* textbuffer,
                       glm::ivec4 alignment, glm::vec4 padding, 
                       glm::vec4 colour) :
                                super(dimensions, parent,
                                      alignment, colour, padding) {
    this->textbuffer = textbuffer;
}
void GuiConsole::draw(Render2D* renderer) {
    glm::vec4 hardDimensions = this->getHardDimensions();

    renderer->drawRectangle(glm::vec2(hardDimensions),
                            glm::vec2(hardDimensions.z, hardDimensions.w)
                          - glm::vec2(hardDimensions.x, hardDimensions.y),
                            colour);

    float height = hardDimensions.w - hardDimensions.y;

    glm::vec2 textDimensions = glm::vec2(10.0f, 12.0f);

    glm::vec2 textLocation = glm::vec2(hardDimensions)
                           + glm::vec2(4.0f,
                                       textDimensions.y + 
                                       (height - textDimensions.y) / 2);

    textLocation.x = floor(textLocation.x);  // Round down to ensure pixels are
    textLocation.y = floor(textLocation.y);  // centered for better font 
                                             // rendering
    renderer->drawText(textbuffer->getText(), textLocation);
}
bool GuiConsole::input(glm::vec2 coordinates, int button, int state, int mods,
           Renderer* renderer, EventManager* eventManager) {
    if (button == 1)
        eventManager->addEvent({"state"}, Action::TEXT_INPUT, std::vector<Event<void*>>());
    return true;
}

/* ---------------------------------------------------------------------------*/

/* GUI Scene -----------------------------------------------------------------*/
GuiScene::GuiScene(glm::vec2 dimensions, Scene* scene, GuiContainer* parent) 
                   : super(dimensions, parent) {

    this->scene = scene;
}

bool GuiScene::input(glm::vec2 coordinates, int button, int state, 
                     int mods, Renderer* renderer, EventManager* eventManager) {
    glm::vec4 hardDimensions = this->getHardDimensions();
    glm::vec2 relativeCoordinates = coordinates - glm::vec2(hardDimensions);
    std::vector<std::string> ids = {"scene"};

    if (coordinates.x <= hardDimensions.z 
     && coordinates.x >= hardDimensions.x 
     && coordinates.y <= hardDimensions.w 
     && coordinates.y >= hardDimensions.y) {

        glm::ivec3 location;
        glm::vec3 normal;

        bool hitModel = renderer->getMouseLocation(scene, 
                                                   relativeCoordinates, 
                                                   location, normal);

        //std::cout << "Tile ID: " << tileId << std::endl;
        if (button == 1 && state == 1) {
            if (hitModel) {
                std::vector<glm::vec3> args = {glm::vec3(location), normal, 
                                               glm::vec3(relativeCoordinates, 
                                               0.0f)};
                eventManager->addEvent(ids, Action::START_MODIFYING, args);
            } else {
                std::vector<glm::vec2> args = {relativeCoordinates};
                eventManager->addEvent(ids, Action::START_ROTATING, args);
            }
        } else if (button == -1) {
            std::vector<glm::vec3> args = {glm::vec3(location), normal,
                                           glm::vec3(relativeCoordinates, 
                                           0.0f)};
            eventManager->addEvent(ids, Action::MODIFY, args);
        }
    } else {
        if (button == -1) {
            std::vector<glm::vec3> args = {glm::vec3(0.0f), glm::vec3(0.0f),
                                           glm::vec3(relativeCoordinates, 
                                           0.0f)};
            eventManager->addEvent(ids, Action::MODIFY, args);
        }
    }
    if (state == 0) {
        std::vector<glm::vec3> args = {};
        eventManager->addEvent(ids, Action::STOP_MODIFYING, args);
        eventManager->addEvent(ids, Action::STOP_ROTATING, args);
    }
    return true;
}
/*----------------------------------------------------------------------------*/