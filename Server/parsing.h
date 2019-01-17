#ifndef PARSING_H
#define PARSING_H

#include "ypipe/ypipe.hpp"
#include "structure.h"
#include "analytical.h"

#include <vector>
#include <functional>
#include <boost/shared_ptr.hpp>

class Parsing
{
public:
    Parsing();

    inline zmq::ypipe_t<Structure::Data, 10000> &getMempool(){
        return memoryPool;
    }

    void attach(std::shared_ptr<Analytical> a);

private:
    std::vector<std::shared_ptr<Analytical>> vFun;
    zmq::ypipe_t<Structure::Data, 10000> memoryPool;
};

#endif // PARSING_H
