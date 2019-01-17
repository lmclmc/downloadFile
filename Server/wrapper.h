#ifndef WRAPPER_H
#define WRAPPER_H

#include <atomic>
#include <boost/shared_ptr.hpp>
#include "parsing.h"

class Protocol;
class Download;

class Wrapper
{
public:
    Wrapper();
    ~Wrapper();

    int init();

    int exec();

private:
    void setProtocol(std::shared_ptr<Protocol> p);

private:
    int epollFd;
    int sockFd;

    std::atomic<bool> bContinue;

    std::shared_ptr<Protocol> protocol;

    Parsing *parsing = nullptr;
    Download *download = nullptr;
};

#endif // WRAPPER_H
