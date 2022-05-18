#ifndef error_request_handler_factory_h
#define error_request_handler_factory_h

#include "request_handler_factory.h"
#include "error_request_handler.h"

class error_request_handler_factory: public request_handler_factory
{
    friend class RequestHandlerFactoryTest;
    public:
        request_handler* create(std::string location, std::string url);
};

#endif
