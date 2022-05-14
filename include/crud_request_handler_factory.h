#ifndef crud_request_handler_factory_h
#define crud_request_handler_factory_h

#include <string>

#include "request_handler_factory.h"
#include "crud_request_handler.h"

class FileSystem;

class crud_request_handler_factory: public request_handler_factory
{
    friend class RequestHandlerFactoryTest;
    public:
        crud_request_handler_factory(std::string root, FileSystem* fs);
        request_handler* create(std::string location, std::string url);
    private:
        std::string root_;
        FileSystem* fs_; // used for primitive dependency injection
};

#endif
