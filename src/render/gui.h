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
#ifndef GUI_H
#define GUI_H

#include <vector>
#include <map>
#include "../../lib/glm/gtc/type_ptr.hpp"

#include "renderer.h"
#include "../scene.h"
#include "../eventManager.h"
#include "../textbuffer.h"


static const int LEFT_ALIGNED = 0;
static const int CENTER_ALIGNED = 1;
static const int RIGHT_ALIGNED = 2;

class GuiComponent;
/* GUI Container -------------------------------------------------------------*/
class GuiContainer {
public:
    GuiContainer(glm::vec2 dimensions, GuiContainer* parent,
                 glm::ivec4 alignment = glm::ivec4(1, 0, 0, 1), 
                 glm::vec4 colour = glm::vec4(0.0f), 
                 glm::vec4 padding = glm::vec4(0.0f));
    virtual void draw(Render2D* renderer);
    virtual bool input(glm::vec2 cursorLocation, int button, int state, 
                       int mods, Renderer* renderer, 
                       EventManager* eventManager);

    glm::vec4 getHardDimensions();
    glm::vec2 dimensions;
    glm::vec4 hardDimensions = glm::vec4(0.0f);
    glm::vec4 colour;
    void addChild(GuiComponent* component);
    void resize(int width, int height);
    std::vector<GuiComponent*> children;
private:
    GuiContainer* parent = nullptr;

    glm::vec4 padding;
    glm::ivec4 alignment;
};

/* GUI -----------------------------------------------------------------------*/
class GUI {
public:
    GUI(glm::vec2 dimensions);

    void addButton(glm::vec2 dimensions, Textbuffer* textBuffer, std::wstring text,
                   std::string parent,
                   std::shared_ptr<EventInterface> event,
                   glm::vec4 colourA = glm::vec4(0.0f),
                   glm::vec4 colourB = glm::vec4(0.0f),
                   glm::vec4 padding = glm::vec4(0.0f), 
                   glm::ivec4 alignment = glm::ivec4(1, 0, 0, 1),
                   std::string alignComponent = "", bool toggleable = false);

    void addContainer(glm::vec2 dimensions, std::string name, 
                      std::string parent = "screen",  
                      glm::vec4 colour = glm::vec4(0.0f),
                      glm::vec4 padding = glm::vec4(0.0f), 
                      glm::ivec4 alignment = glm::ivec4(1, 0, 0, 1),
                      std::string alignComponent = "");

    void addButtonLinker(glm::vec2 dimensions, std::string name, 
                         std::string parent = "screen",  
                         glm::vec4 colour = glm::vec4(0.0f),
                         glm::vec4 padding = glm::vec4(0.0f), 
                         glm::ivec4 alignment = glm::ivec4(1, 0, 0, 1),
                         std::string alignComponent = "");

    void addScene(glm::vec2 dimensions, Scene* scene, std::string parent);

    void addConsole(glm::vec2 dimensions, std::string parent,
                    Textbuffer* textbuffer,
                    glm::ivec4 alignment = glm::ivec4(1, 0, 0, 1), 
                    glm::vec4 padding = glm::vec4(0.0f), 
                    glm::vec4 colour = glm::vec4(1.0f));

    void draw(Render2D* renderer);
    void resize(int width, int height);
    bool input(glm::vec2 cursorLocation, int button, 
               int state, int mods, Renderer* renderer, 
               EventManager* eventManager);
private:
    GuiContainer* screen;
    std::map<std::string, GuiContainer*> containers;

    glm::vec2 dimensions;
};

/* GUI Component -------------------------------------------------------------*/
class GuiComponent : public GuiContainer {
public:
    GuiComponent(glm::vec2 dimensions, GuiContainer* parent,
                 glm::ivec4 alignment = glm::ivec4(1, 0, 0, 1), 
                 glm::vec4 colour = glm::vec4(0.0f), 
                 glm::vec4 padding = glm::vec4(0.0f));
private:
    typedef GuiContainer super;
};

/* GUI Button Linker ---------------------------------------------------------*/

class GuiButtonLinker : public GuiContainer {
public:
    GuiButtonLinker(glm::vec2 dimensions, GuiContainer* parent,
                    glm::ivec4 alignment = glm::ivec4(1, 0, 0, 1), 
                    glm::vec4 colour = glm::vec4(0.0f), 
                    glm::vec4 padding = glm::vec4(0.0f));

    bool input(glm::vec2 coordinates, int button, int state, 
               int mods, Renderer* renderer, EventManager* eventManager);
private:
    typedef GuiContainer super;
};


/* GUI Scene -----------------------------------------------------------------*/
class GuiScene : public GuiComponent {
public:
    GuiScene(glm::vec2 dimensions, Scene* scene, GuiContainer* parent);

    bool input(glm::vec2 coordinates, int button, int state, 
               int mods, Renderer* renderer, EventManager* eventManager);
private:
    void update(double delta, glm::vec2 coordinates, Renderer* renderer);
    typedef GuiComponent super;
    Scene* scene;
    bool removeBlocks = false;
    glm::ivec3 initialLocation;
    glm::vec3 initialNormal;
};

/* GUI Button ----------------------------------------------------------------*/
/// Button that performs action upon release.
class GuiButton : public GuiComponent {
public:
    GuiButton(glm::vec2 dimensions, GuiContainer* parent,
              std::shared_ptr<EventInterface> event,
              glm::ivec4 alignment, glm::vec4 padding,
              Textbuffer* textBuffer, std::wstring text, 
              glm::vec4 colourA, glm::vec4 colourB,
              bool toggleable);
    void draw(Render2D* renderer);
    bool input(glm::vec2 coordinates, int button, int state, 
               int mods, Renderer* renderer, EventManager* eventManager);

    void resetState();
private:
    typedef GuiComponent super;

    std::shared_ptr<EventInterface> event;
    std::wstring text;
    Textbuffer* textBuffer;
    glm::vec2 textDimensions;
    glm::vec4 colourA;
    glm::vec4 colourB;

    bool toggleable;
};
/*GUI Console ----------------------------------------------------------------*/
class GuiConsole : public GuiComponent {
public:
    GuiConsole(glm::vec2 dimensions, GuiContainer* parent, Textbuffer* textbuffer,
               glm::ivec4 alignment, glm::vec4 padding, glm::vec4 colour);
    void draw(Render2D* renderer);
    bool input(glm::vec2 coordinates, int button, int state, 
               int mods, Renderer* renderer, EventManager* eventManager);
private:
    typedef GuiComponent super;

    Textbuffer* textbuffer;

    std::shared_ptr<EventInterface> event;
};

#endif