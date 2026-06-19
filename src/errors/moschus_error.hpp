#pragma once

#include <stdexcept>
#include "moschus_string.hpp"

struct MoschusWarning {
  std::string warnstr;
  MoschusWarning(const std::string& msg) : warnstr(msg) {}
  MoschusWarning(const MoschusString& msg) : warnstr(msg.to_string()) {}
};

enum class MoschusErrorType {
MoschusWarning, // warning as errors
MoschusGeneric, // generic throw code -- for accumulated errors (actually buffered not fatal immediately)

MuskParserError,
MuskStartError,
FileIOError,
NullDeref,
BadAccess,
};

class MoschusError : public std::runtime_error {
  MoschusErrorType err_type;
  public:
    MoschusError(MoschusErrorType type) : err_type(type), std::runtime_error("") {}
    MoschusError(const std::string& msg, MoschusErrorType type) : err_type(type), std::runtime_error(msg) {}
    MoschusError(const MoschusString& msg, MoschusErrorType type) : err_type(type), std::runtime_error(msg.to_string()) {}
};