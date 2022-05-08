#ifndef static_request_handler_factory_h
#define static_request_handler_factory_h

#include "request_handler_factory.h"
#include "static_request_handler.h"

class static_request_handler_factory: public request_handler_factory
{
    friend class RequestHandlerFactoryTest;
    public:
        static_request_handler_factory(std::string root);
        request_handler* create(std::string location, std::string url);
    private:
        std::string root_;
};

#endif
