#ifndef ANALYTICAL_H
#define ANALYTICAL_H

#include "structure.h"

class Download;

class Analytical
{
public:
    Analytical(Structure::Function f_);
    inline Structure::Function &getF(){
        return f;
    }

    virtual void analytical(Structure::Data &data) = 0;

private:
    Structure::Function f;
};

class NormalAnalytical : public Analytical{
public:
    NormalAnalytical();

    void analytical(Structure::Data &data);
};

class EnsureAnalytical : public Analytical{
public:
    EnsureAnalytical(Download *d);

    void analytical(Structure::Data &data);

private:
    Download *pDownload;
};

class ContinuinglyAnalytical : public Analytical{
public:
    ContinuinglyAnalytical(Download *d);

    void analytical(Structure::Data &data);

private:
    Download *pDownload;
};

#endif // ANALYTICAL_H
