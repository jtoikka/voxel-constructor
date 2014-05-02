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
#include "state.h"
#include "utility.h"

#include <GLFW/glfw3.h>

static const float sideFieldWidth = 150.0f;
static const float menuBarHeight = 20.0f;

State::State() {
    eventManager = new EventManager();
    listener = new Listener();
    eventManager->addListener("state", listener);

    renderer = new Renderer(width, height, glm::vec4(sideFieldWidth, 
                            menuBarHeight, width, height), "renderer",
                            eventManager);

    loadMeshes();

    scene = new Scene("scene", eventManager);
    buildGUI(glm::vec2(width, height));  
}

State::~State() {
    std::cout << "Deleting state..." << std::endl;
    delete renderer;
    delete scene;
    delete gui;
    delete eventManager;
    delete listener;
    for (auto bufferPair : textbuffers) {
        delete bufferPair.second;
    }
    std::cout << "State deleted." << std::endl;
}

void State::loadMeshes() {
    std::string modelDirectory = Utility::programDirectory + "data/models/";
    renderer->loadMesh(modelDirectory, "cube");
    renderer->loadMesh(modelDirectory, "slope");
    renderer->loadMesh(modelDirectory, "rslope");
    renderer->loadMesh(modelDirectory, "diagonal");
    renderer->loadMesh(modelDirectory, "cornerSlope");
    renderer->loadMesh(modelDirectory, "rcornerSlope");
    renderer->loadMesh(modelDirectory, "invCorner");
    renderer->loadMesh(modelDirectory, "rInvCorner");
}

// GUI needs reworking
void State::buildGUI(glm::vec2 dimensions) {
    gui = new GUI(glm::vec2(width, height));

    /* Menu bar ============================================================= */

    gui->addContainer(glm::vec2(1.0f, 20.0f), "menuBar", "screen",
        glm::vec4(44.0f/255.0f, 44.0f/255.0f, 44.0f/255.0f, 1.0f));

    std::shared_ptr<EventInterface> eventFile(new Event<void*>);
    eventFile->action = Action::DO_NOTHING;

    Textbuffer* fileButtonText = new Textbuffer(L"File");
    gui->addButton(glm::vec2(50.0f, 20.0f), fileButtonText, L"File", "menuBar",
                   eventFile,
                   glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), 
                   glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));

    gui->addButton(glm::vec2(50.0f, 20.0f), nullptr, L"Edit", "menuBar",
                   eventFile,
                   glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), 
                   glm::vec4(0.1f, 0.1f, 0.1f, 1.0f),
                   glm::vec4(0.0f), glm::ivec4(0, 0, 0, 1));

    gui->addButton(glm::vec2(50.0f, 20.0f), nullptr, L"View", "menuBar",
                   eventFile,
                   glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), 
                   glm::vec4(0.1f, 0.1f, 0.1f, 1.0f),
                   glm::vec4(0.0f), glm::ivec4(0, 0, 0, 1));

    gui->addButton(glm::vec2(50.0f, 20.0f), nullptr, L"Tools", "menuBar",
                   eventFile,
                   glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), 
                   glm::vec4(0.1f, 0.1f, 0.1f, 1.0f),
                   glm::vec4(0.0f), glm::ivec4(0, 0, 0, 1));

    /* ====================================================================== */
    gui->addContainer(glm::vec2(1.0f, 1.0f), "mainField", "screen",
        glm::vec4(0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), 
        glm::ivec4(1, 0, 0, -1), "menuBar");

    /* Side bar ============================================================= */

    gui->addContainer(glm::vec2(150.0f, 1.0f), "SideBar", "mainField",
        glm::vec4(114.0f/255.0f, 114.0f/255.0f, 114.0f/255.0f, 1.0f),
        glm::vec4(0.0f), glm::ivec4(0, 0, 0, 1));


    // Tool buttons ------------------------------------------------------------

    gui->addButtonLinker(glm::vec2(120.0f, 100.0f),
        "toolButtonContainer", "SideBar",
        glm::vec4(70.0f/255.0f, 70.0f/255.0f, 70.0f/255.0f, 1.0f), 
        glm::vec4(10.0f, 0.0f, 0.0f, 0.0f), glm::ivec4(1, 0, 0, 0));

    std::shared_ptr<EventInterface> eventAdd(new Event<void*>);
    eventAdd->action = Action::MODE_ADD;
    eventAdd->ids = {"scene"};

    gui->addButton(glm::vec2(100.0f, 20.0f), nullptr, L"Add", "toolButtonContainer",
                   eventAdd,
                   glm::vec4(114.0f/255.0f, 114.0f/255.0f, 114.0f/255.0f, 1.0f), 
                   glm::vec4(0.2f, 0.2f, 0.2f, 1.0f),
                   glm::vec4(10.0f, 1.0f, 0.0f, 1.0f));

    std::shared_ptr<EventInterface> eventRemove(new Event<void*>);
    eventRemove->action = Action::MODE_REMOVE;
    eventRemove->ids = {"scene"};

    gui->addButton(glm::vec2(100.0f, 20.0f), nullptr, L"Remove", "toolButtonContainer",
                   eventRemove,
                   glm::vec4(114.0f/255.0f, 114.0f/255.0f, 114.0f/255.0f, 1.0f), 
                   glm::vec4(0.2f, 0.2f, 0.2f, 1.0f),
                   glm::vec4(10.0f, 1.0f, 0.0f, 1.0f),
                   glm::ivec4(1, 0, 0, 0));

    std::shared_ptr<EventInterface> eventPaint(new Event<void*>);
    eventPaint->action = Action::MODE_PAINT;
    eventPaint->ids = {"scene"};

    gui->addButton(glm::vec2(100.0f, 20.0f), nullptr, L"Paint", "toolButtonContainer",
                   eventPaint,
                   glm::vec4(114.0f/255.0f, 114.0f/255.0f, 114.0f/255.0f, 1.0f), 
                   glm::vec4(0.2f, 0.2f, 0.2f, 1.0f),
                   glm::vec4(10.0f, 1.0f, 0.0f, 1.0f),
                   glm::ivec4(1, 0, 0, 0));

    // Mode buttons ------------------------------------------------------------

    gui->addButtonLinker(glm::vec2(120.0f, 40.0f), 
        "modeButtonContainer", "SideBar",
        glm::vec4(70.0f/255.0f, 70.0f/255.0f, 70.0f/255.0f, 1.0f), 
        glm::vec4(10.0f, 0.0f, 0.0f, 0.0f), glm::ivec4(1, 0, 0, 0), 
        "toolButtonContainer");


    std::shared_ptr<EventInterface> eventWireframe(new Event<void*>);
    eventWireframe->action = Action::TOGGLE_WIREFRAME;
    eventWireframe->ids = {"renderer"};

    gui->addButton(glm::vec2(100.0f, 20.0f), nullptr, L"Wireframe", "modeButtonContainer",
                   eventWireframe,
                   glm::vec4(114.0f/255.0f, 114.0f/255.0f, 114.0f/255.0f, 1.0f), 
                   glm::vec4(0.2f, 0.2f, 0.2f, 1.0f),
                   glm::vec4(10.0f, 1.0f, 0.0f, 1.0f),
                   glm::ivec4(1, 0, 0, 0));

    // Blocks ------------------------------------------------------------------

    gui->addButtonLinker(glm::vec2(120.0f, 250.0f), 
        "blockContainer", "SideBar",
        glm::vec4(70.0f/255.0f, 70.0f/255.0f, 70.0f/255.0f, 1.0f), 
        glm::vec4(10.0f, 0.0f, 0.0f, 0.0f), glm::ivec4(1, 0, 0, 0), 
        "modeButtonContainer");

    // Cube
    std::shared_ptr<EventInterface> eventCube(new Event<void*>);
    eventCube->action = Action::BLOCK_CUBE;
    eventCube->ids = {"scene"};

    gui->addButton(glm::vec2(100.0f, 20.0f), nullptr, L"Cube", "blockContainer",
                   eventCube,
                   glm::vec4(114.0f/255.0f, 114.0f/255.0f, 114.0f/255.0f, 1.0f), 
                   glm::vec4(0.2f, 0.2f, 0.2f, 1.0f),
                   glm::vec4(10.0f, 1.0f, 0.0f, 1.0f),
                   glm::ivec4(1, 0, 0, 0));

    // Slope
    std::shared_ptr<EventInterface> eventSlope(new Event<void*>);
    eventSlope->action = Action::BLOCK_SLOPE;
    eventSlope->ids = {"scene"};

    gui->addButton(glm::vec2(100.0f, 20.0f), nullptr, L"Slope", "blockContainer",
                   eventSlope,
                   glm::vec4(114.0f/255.0f, 114.0f/255.0f, 114.0f/255.0f, 1.0f), 
                   glm::vec4(0.2f, 0.2f, 0.2f, 1.0f),
                   glm::vec4(10.0f, 1.0f, 0.0f, 1.0f),
                   glm::ivec4(1, 0, 0, 0));

    // Flipped Slope
    std::shared_ptr<EventInterface> eventRSlope(new Event<void*>);
    eventRSlope->action = Action::BLOCK_RSLOPE;
    eventRSlope->ids = {"scene"};

    gui->addButton(glm::vec2(100.0f, 20.0f), nullptr, L"rSlope", "blockContainer",
                   eventRSlope,
                   glm::vec4(114.0f/255.0f, 114.0f/255.0f, 114.0f/255.0f, 1.0f), 
                   glm::vec4(0.2f, 0.2f, 0.2f, 1.0f),
                   glm::vec4(10.0f, 1.0f, 0.0f, 1.0f),
                   glm::ivec4(1, 0, 0, 0));

    // Diagonal
    std::shared_ptr<EventInterface> eventDiagonal(new Event<void*>);
    eventDiagonal->action = Action::BLOCK_DIAGONAL;
    eventDiagonal->ids = {"scene"};

    gui->addButton(glm::vec2(100.0f, 20.0f), nullptr, L"Diagonal", "blockContainer",
                   eventDiagonal,
                   glm::vec4(114.0f/255.0f, 114.0f/255.0f, 114.0f/255.0f, 1.0f), 
                   glm::vec4(0.2f, 0.2f, 0.2f, 1.0f),
                   glm::vec4(10.0f, 1.0f, 0.0f, 1.0f),
                   glm::ivec4(1, 0, 0, 0));

    // Corner slope
    std::shared_ptr<EventInterface> eventCornerSlope(new Event<void*>);
    eventCornerSlope->action = Action::BLOCK_CORNERSLOPE;
    eventCornerSlope->ids = {"scene"};

    gui->addButton(glm::vec2(100.0f, 20.0f), nullptr, L"Corner", "blockContainer",
                   eventCornerSlope,
                   glm::vec4(114.0f/255.0f, 114.0f/255.0f, 114.0f/255.0f, 1.0f), 
                   glm::vec4(0.2f, 0.2f, 0.2f, 1.0f),
                   glm::vec4(10.0f, 1.0f, 0.0f, 1.0f),
                   glm::ivec4(1, 0, 0, 0));

    // Flipped corner slope
    std::shared_ptr<EventInterface> eventRCornerSlope(new Event<void*>);
    eventRCornerSlope->action = Action::BLOCK_RCORNERSLOPE;
    eventRCornerSlope->ids = {"scene"};

    gui->addButton(glm::vec2(100.0f, 20.0f), nullptr, L"rCorner", "blockContainer",
                   eventRCornerSlope,
                   glm::vec4(114.0f/255.0f, 114.0f/255.0f, 114.0f/255.0f, 1.0f), 
                   glm::vec4(0.2f, 0.2f, 0.2f, 1.0f),
                   glm::vec4(10.0f, 1.0f, 0.0f, 1.0f),
                   glm::ivec4(1, 0, 0, 0));

    // Inverse corner slope
    std::shared_ptr<EventInterface> eventInvCorner(new Event<void*>);
    eventInvCorner->action = Action::BLOCK_INVCORNER;
    eventInvCorner->ids = {"scene"};

    gui->addButton(glm::vec2(100.0f, 20.0f), nullptr, L"Inv Corner", "blockContainer",
                   eventInvCorner,
                   glm::vec4(114.0f/255.0f, 114.0f/255.0f, 114.0f/255.0f, 1.0f), 
                   glm::vec4(0.2f, 0.2f, 0.2f, 1.0f),
                   glm::vec4(10.0f, 1.0f, 0.0f, 1.0f),
                   glm::ivec4(1, 0, 0, 0));

    // Inverse corner slope
    std::shared_ptr<EventInterface> eventRInvCorner(new Event<void*>);
    eventRInvCorner->action = Action::BLOCK_RINVCORNER;
    eventRInvCorner->ids = {"scene"};

    gui->addButton(glm::vec2(100.0f, 20.0f), nullptr, L"rInv Corner", "blockContainer",
                   eventRInvCorner,
                   glm::vec4(114.0f/255.0f, 114.0f/255.0f, 114.0f/255.0f, 1.0f), 
                   glm::vec4(0.2f, 0.2f, 0.2f, 1.0f),
                   glm::vec4(10.0f, 1.0f, 0.0f, 1.0f),
                   glm::ivec4(1, 0, 0, 0));


    // Block modifiers ---------------------------------------------------------

    gui->addContainer(
        glm::vec2(120.0f, 70.0f), "blockModifiers", "SideBar",
        glm::vec4(70.0f/255.0f, 70.0f/255.0f, 70.0f/255.0f, 1.0f),
        glm::vec4(10.0f, 0.0f, 0.0f, 0.0f), glm::ivec4(1, 0, 0, 0),
        "blockContainer");

    std::shared_ptr<EventInterface> eventRotate(new Event<void*>);
    eventRotate->action = Action::ROTATE_BLOCK;
    eventRotate->ids = {"scene"};

    Textbuffer* rotateText = new Textbuffer(L"Rotate (0)");
    textbuffers.emplace("rotate", rotateText);
    gui->addButton(glm::vec2(100.0f, 20.0f), rotateText, L"Rotate", "blockModifiers",
                   eventRotate, 
                   glm::vec4(114.0f/255.0f, 114.0f/255.0f, 114.0f/255.0f, 1.0f),
                   glm::vec4(0.2f, 0.2f, 0.2f, 1.0f),
                   glm::vec4(10.0f, 1.0f, 0.0f, 1.0f),
                   glm::ivec4(1, 0, 0, 0));

    /* Scene ================================================================ */

    gui->addContainer(glm::vec2(1.0f, 1.0f), "sceneContainer", "mainField",
        glm::vec4(0.0f), glm::vec4(0.0f), glm::ivec4(0, 0, 0, 1), "SideBar");

    gui->addScene(glm::vec2(1.0f, 1.0f), scene, "sceneContainer");

    /* Console ============================================================== */
    gui->addContainer(glm::vec2(1.0f, 25.0f), "consoleField", "screen",
                      glm::vec4(44.0f/255.0f, 44.0f/255.0f, 44.0f/255.0f, 1.0f), 
                      glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), 
                      glm::ivec4(0, 0, 1, -1));

    Textbuffer* consoleText = new Textbuffer(L"");
    textbuffers.emplace("console", consoleText);

    gui->addConsole(glm::vec2(0.99f, 21.0f), "consoleField",
                    consoleText, glm::ivec4(1, 0, 0, 0), 
                    glm::vec4(3.0f, 1.0f, 0.0f, 1.0f),
                    glm::vec4(114.0f/255.0f, 114.0f/255.0f, 114.0f/255.0f, 1.0f));

    /* ====================================================================== */
}

void State::display(GLFWwindow* window) {
    // GLenum err;
    renderer->start();
    renderer->renderScene(scene);
    renderer->finish();

    renderer->start2D();
    gui->draw(renderer->get2DRenderer());
    renderer->finish2D();
    
    glfwSwapBuffers(window);
}

void State::resizeWindow(int width, int height) {
    this->width = width;
    this->height = height;
    if (renderer != nullptr) {
        renderer->resize(width, height, glm::vec4(sideFieldWidth, 
                                                  menuBarHeight, 
                                                  width, height));
    }
    if (gui != nullptr) {
        gui->resize(width, height);
    }
}

void State::save(std::wstring title) {
    std::wcout << L"Saving to: " << title << std::endl;
}

void State::handleEvents() {
    for (auto event : listener->events) {
        switch (event->action) {
        case Action::ROTATE_BLOCK : {
            auto it = textbuffers.find("rotate");
            if (it != textbuffers.end()) {
                std::wstringstream sstm;
                sstm << L"Rotate (" << 
                    (std::dynamic_pointer_cast<Event<unsigned int>>(event))
                        ->args[0] 
                    << L")";
                it->second->changeString(sstm.str());
            }
            break;
        }
        case Action::TEXT_INPUT : {
            inputText = true;
        }
        default:
            break;
        }
    }
    listener->events.clear();
}

void State::backspace() {
    if (inputText) {
        auto it = textbuffers.find("console");
        if (it != textbuffers.end()) {
            Textbuffer* buffer = it->second;
            std::wstring text = buffer->getText();
            buffer->changeString(text.substr(0, text.length()-1));
        }
    }
}

void State::runCommand() {
    auto it = textbuffers.find("console");
    if (it != textbuffers.end()) {
        std::wstringstream commandStream;
        commandStream << it->second->getText();
        
        std::wstring command;
        if (commandStream >> command) {
            std::wcout << command << std::endl;
            if (command == L"save") {
                std::wstring title;
                if (!(commandStream >> title)) {
                    title = L"untitled.tile";
                }
                save(title);
            } else if (command == L"export") {
                std::vector<Scene*> args = {scene};
                eventManager->addEvent({"renderer"}, Action::EXPORT_TILE, args);
            }
        }
        renderer->removeText(it->second->getText());
        it->second->changeString(L"");
    }
    inputText = false;
}

void State::keyPressed(int key) {
    switch (key) {
    case GLFW_KEY_ENTER: {
        runCommand();
        break;
    }
    case GLFW_KEY_BACKSPACE: {
        backspace();
        break;
    }
    default: 
        break; 
    }
}

Textbuffer* State::getConsoleTextbuffer()  {
    auto it = textbuffers.find("console");
    if (it != textbuffers.end()) {
        return it->second;
    }
    return nullptr;
}

void State::update(double delta) {
    handleEvents();
    eventManager->delegateEvents();
    scene->update(delta);
    eventManager->delegateEvents();
    renderer->update(delta);
}

void State::setCursorPosition(double cursorX, double cursorY) {
    this->cursorX = cursorX;
    this->cursorY = cursorY;
    gui->input(glm::vec2(cursorX, cursorY), -1, -1, 0, renderer, eventManager);
}

void State::mouseButton(int button, int state, int mods) {
    switch (button) {
    case MOUSE_BUTTON_LEFT : {
        gui->input(glm::vec2(cursorX, cursorY), button, 
                   state, mods, renderer, eventManager);
    }
    }
}

bool State::getTextInputState() {
    return inputText;
}

void State::charInput(unsigned int codepoint) {
    wchar_t c = (wchar_t)codepoint;
    if (inputText) {
        auto it = textbuffers.find("console");
        if (it != textbuffers.end()) {
            renderer->removeText(it->second->getText());
            std::wstring s(1, c);
            it->second->addToText(s);
        }
    }
}