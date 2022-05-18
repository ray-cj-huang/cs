#ifndef request_handler_factory_h
#define request_handler_factory_h

#include "request_handler.h"

class request_handler_factory
{
    public:
        virtual request_handler* create(
            std::string location, std::string url) = 0;
};

#endif
