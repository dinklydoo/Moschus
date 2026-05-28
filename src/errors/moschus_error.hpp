#pragma once

#include <stdexcept>

enum class MoschusErrorType {
  MuskParserError
};

class MoschusError : public std::runtime_error {
  MoschusErrorType err_type;
  public:
    MoschusError(const std::string& msg, MoschusErrorType type) : err_type(type), std::runtime_error(msg) {}
};
