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
#ifndef STATE_H
#define STATE_H

#include "render/renderer.h"
#include "scene.h"
#include "render/gui.h"
#include "eventManager.h"
#include "textbuffer.h"

#include <map>

class State {
public:
    static const int MOUSE_BUTTON_LEFT = 1;

    State();

    ~State();

    void update(double delta);

    void display(GLFWwindow *window);

    void resizeWindow(int width, int height);

    void setCursorPosition(double cursorX, double cursorY);

    void mouseButton(int button, int state, int mods);

    void guiInput(std::string input);

    bool getTextInputState();

    Textbuffer* getConsoleTextbuffer();

    void charInput(unsigned int);

    void keyPressed(int key);
    
private:
    Renderer* renderer;
    Scene* scene;
    GUI* gui;
    EventManager* eventManager;
    Listener* listener;

    bool inputText = false;

    std::map<std::string, Textbuffer*> textbuffers;

    int width = 640;
    int height = 480;

    double cursorX, cursorY;

    void loadMeshes();

    void handleEvents();

    void buildGUI(glm::vec2 dimensions);

    void runCommand();

    void save(std::wstring title);

    void backspace();
};

#endif