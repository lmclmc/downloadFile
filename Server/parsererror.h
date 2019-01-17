#ifndef PARSERERROR_H
#define PARSERERROR_H

#include <iostream>

class ParseError : public std::exception {
public:
    ParseError(std::string msg);
    virtual ~ParseError();

    std::string what();

private:
    std::string message;
};

#endif // PARSERERROR_H
