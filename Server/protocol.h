#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "ypipe/ypipe.hpp"
#include "parsing.h"

class Protocol
{
public:
    Protocol();

    virtual void run() = 0;
};

class SocketProtocol : public Protocol
{
public:
    SocketProtocol(int epollFd_, int sockFd_);
    ~SocketProtocol();

    void run();

private:
    int epollFd;
    int sockFd;
};

class ClientProtocol : public Protocol
{
public:
    ClientProtocol(int clientFd_, zmq::ypipe_t<Structure::Data, 10000> *m);
    ~ClientProtocol();

    void run();

private:
    int clientFd;

    zmq::ypipe_t<Structure::Data, 10000> *mem;
};

#endif // PROTOCOL_H
