#include <iostream>
#include "wrapper.h"
#include <signal.h>

int main()
{
    signal(SIGPIPE, SIG_IGN);
    Wrapper wrapper;
    wrapper.init();
    return wrapper.exec();
}
