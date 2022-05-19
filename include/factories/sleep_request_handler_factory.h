#ifndef sleep_request_handler_factory_h
#define sleep_request_handler_factory_h

#include "request_handler_factory.h"
#include "sleep_request_handler.h"

class sleep_request_handler_factory: public request_handler_factory
{
    friend class RequestHandlerFactoryTest;
    public:
        request_handler* create(std::string location, std::string url);
};

#endif
