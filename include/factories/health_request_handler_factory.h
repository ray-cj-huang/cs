#ifndef health_request_handler_factory_h
#define health_request_handler_factory_h

#include "request_handler_factory.h"
#include "request_handler.h"

class health_request_handler_factory: public request_handler_factory
{
    friend class RequestHandlerFactoryTest;
    public:
        request_handler* create(std::string location, std::string url);
};

#endif
