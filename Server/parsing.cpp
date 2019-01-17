#include "parsing.h"
#include "threadpool/threadpool.h"
#include "analytical.h"
#include <memory>

Parsing::Parsing()
{
    vFun.reserve((int)Structure::Function::Resolve);

    ThreadPool *t = ThreadPool::getInstance();
    for (int i=0; i<4; i++){
        t->addThread();
    }

    t->addTask([this](){
        Structure::Data data;
        while (1){
            if (this->memoryPool.read(&data)){
                this->vFun[static_cast<int>(data.prot.PFunction)].get()->analytical(data);
            }
        }
    });
}

void Parsing::attach(std::shared_ptr<Analytical> a)
{
    vFun[static_cast<int>(a.get()->getF())] = a;
}
