TEMPLATE = app
CONFIG += console c++11 thread
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    wrapper.cpp \
    protocol.cpp \
    parsererror.cpp \
    parsing.cpp \
    threadpool/threadpool.cpp \
    analytical.cpp \
    download.cpp

HEADERS += \
    wrapper.h \
    protocol.h \
    parsererror.h \
    parsing.h \
    ypipe/yqueue.hpp \
    ypipe/ypipe_base.hpp \
    ypipe/ypipe.hpp \
    ypipe/likely.hpp \
    ypipe/atomic_ptr.hpp \
    threadpool/threadpool.h \
    analytical.h \
    structure.h \
    download.h
