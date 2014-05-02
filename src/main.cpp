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
#ifndef MAIN_CPP
#define MAIN_CPP

#include <iostream>
#include <time.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "utility.h"
#include "state.h"

int windowWidth, windowHeight;

double cursorX, cursorY;

bool paused = 0;

State* state;

static void error_callback(int error, const char* description) {
    fputs(description, stderr);
}


static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        paused = !paused;
    }
    if  (action == GLFW_PRESS) {
        state->keyPressed(key);
    }

}

static void focus_callback(GLFWwindow* window, int focus) {
    if (focus) {
        paused = false;
    } else {
        paused = true;
    }
}

static void mouseButton_callback(GLFWwindow* window, int mouseButton, int buttonState, int mods) {
    switch (mouseButton) {
    case GLFW_MOUSE_BUTTON_LEFT: {
        state->mouseButton(State::MOUSE_BUTTON_LEFT, buttonState, mods);
        break;
    }
    default:
        break;
    }
}

static void cursorPos_callback(GLFWwindow* window, double xpos, double ypos) {
    cursorX = xpos;
    cursorY = ypos;
}

static void char_callback(GLFWwindow* window, unsigned int codePoint) {
    state->charInput(codePoint);
}

// OS X related, mostly (reminder: add macros to detect OS)
static void setCoreProfile() {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

static void window_size_callback(GLFWwindow* window, int width, int height) {
    // -should also send information to renderer
    windowWidth = width;
    windowHeight = height;
    state->resizeWindow(width, height);
    state->display(window);
}

int main(int argc, char* argv[]) {
    if (argv[0] != NULL) {
        std::string progDirectory(argv[0]);
        std::cout << "Program directory: " << progDirectory << std::endl;
        progDirectory = progDirectory.substr(0, progDirectory.find_last_of("/") + 1);
        Utility::programDirectory = progDirectory;
    }

    GLFWwindow* window;
    windowWidth = 640;
    windowHeight = 480;

    glfwSetErrorCallback(error_callback);
    
    if (!glfwInit())
        exit(EXIT_FAILURE);

    setCoreProfile();

    window = glfwCreateWindow(windowWidth, windowHeight, "Voxel Constructor", NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);

    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouseButton_callback);
    glfwSetCursorPosCallback(window, cursorPos_callback);
    glfwSetWindowSizeCallback(window, window_size_callback);
    glfwSetWindowFocusCallback(window, focus_callback);
    glfwSetCharCallback(window, char_callback);


    double previousFrameTime = glfwGetTime();
    double frameTime = glfwGetTime();
    double delta = 0.0;

    struct timespec timeSleep;
    timeSleep.tv_sec = 0;
    timeSleep.tv_nsec = 33333333;

    state = new State();

    mouseButton_callback(window, GLFW_MOUSE_BUTTON_LEFT, 0, 0); // Ensure mouse button is released upon starting

    double secondCounter = 0.0;
    int frames = 0;

    while (!glfwWindowShouldClose(window)) {
        frameTime = glfwGetTime();
        delta = frameTime - previousFrameTime;
        if (paused) {
            nanosleep(&timeSleep, NULL);
        } else {
            state->update(delta);
            //pollKeys(state, window, delta);
        }
        state->display(window);
        glfwPollEvents();
        previousFrameTime = frameTime;
        state->setCursorPosition(cursorX, cursorY);
        frames++;
        secondCounter += delta;
        if (secondCounter > 1.0) {
            secondCounter = 0.0;
            // std::cout << "FPS: " << frames << std::endl;
            frames = 0;
        }
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

#endif