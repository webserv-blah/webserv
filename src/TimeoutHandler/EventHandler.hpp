#ifndef EVENT_HANDLER_HPP
#define EVENT_HANDLER_HPP

#include "Client.hpp"

class EventHandler {
    public:
        virtual void handleError(int code, ClientSession& client) = 0;
};

#endif