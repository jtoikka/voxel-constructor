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
#ifndef EVENT_H
#define EVENT_H

enum class Action {
    DO_NOTHING,
    START_MODIFYING,
    MODIFY,
    STOP_MODIFYING,
    START_ROTATING,
    STOP_ROTATING,
    REBUILD_TILE,
    MODE_ADD,
    MODE_REMOVE,
    MODE_PAINT,
    TOGGLE_WIREFRAME,
    BLOCK_CUBE,
    BLOCK_SLOPE,
    BLOCK_RSLOPE,
    BLOCK_DIAGONAL,
    BLOCK_CORNERSLOPE,
    BLOCK_RCORNERSLOPE,
    BLOCK_INVCORNER,
    BLOCK_RINVCORNER,
    ROTATE_BLOCK,
    TEXT_INPUT,
    EXPORT_TILE
};

struct EventInterface {
    std::vector<std::string> ids;
    Action action;
    virtual ~EventInterface() {ids.clear();}
};

template <typename Arg_Type> struct Event : public EventInterface {
    std::vector<Arg_Type> args;
};

#endif