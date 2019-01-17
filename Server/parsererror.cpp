#include "parsererror.h"

ParseError::ParseError(std::string msg)
    : message(msg)
{}

ParseError::~ParseError()
{}

std::string ParseError::what(){
    return message;
}
