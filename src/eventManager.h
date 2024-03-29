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
#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H

#include <boost/unordered_map.hpp>
#include "../lib/glm/gtc/type_ptr.hpp"

#include "event.h"

#include <vector>

class Listener {
public:
    std::vector<std::shared_ptr<EventInterface>> events;
};

class EventManager {
public:
    std::vector<std::shared_ptr<EventInterface>> events;
    boost::unordered_map<std::string, Listener*> listeners;

    void addEvent(std::shared_ptr<EventInterface> sptr);
    void addListener(std::string id, Listener* listener);
    void removeListener(std::string id);

    template <typename Arg_Type>
    void addEvent(std::vector<std::string> ids, Action action, std::vector<Arg_Type> args) {
        std::shared_ptr<Event<Arg_Type>> event(new Event<Arg_Type>());
        for (auto arg : args) {
            event->args.push_back(arg);
        }
        event->action = action;
        for (auto id : ids) {
            event->ids.push_back(id);
        }
        events.push_back(event);
    }

    void delegateEvents();
};

#endif 

