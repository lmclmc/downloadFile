TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    threadpool.cpp

LIBS += -lpthread

HEADERS += \
    threadpool.h \
    ypipe.hpp \
    yqueue.hpp \
    ypipe_base.hpp \
    atomic_ptr.hpp
