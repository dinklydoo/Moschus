#include "except_handler.hpp"

#include <vector>
#include <iostream>

namespace MoschusExceptHandler {
  namespace {
    std::vector<MoschusWarning> _warnings;
    std::vector<MoschusError> _errors;

    bool check_flag(EXCEPTION_FLAGS flag){ return (flags & static_cast<FLAG_MASK>(flag)); }
  }

  void set_flag(EXCEPTION_FLAGS flag){ flags |= static_cast<FLAG_MASK>(flag); }

  void push_warning(MoschusWarning warning){
    if (check_flag(EXCEPTION_FLAGS::IGNORE_WARNINGS)) return;
    _warnings.push_back(warning); 
  }
  void push_error(MoschusError error){
    if (check_flag(EXCEPTION_FLAGS::THROW_IMMEDIATELY)){
      throw error;
    }
    else _errors.push_back(error);
  }

  void log_warnings(){
    if (flags & static_cast<FLAG_MASK>(EXCEPTION_FLAGS::IGNORE_WARNINGS)) return;
    for (const auto& _w : _warnings){
      std::cout << _w.warnstr << std::endl;
    }
    _warnings.clear();
    if (check_flag(EXCEPTION_FLAGS::THROW_ON_WARNING)){
      throw MoschusError(MoschusErrorType::MoschusWarning);
    }
  }

  void log_errors(){
    for (const auto& _e : _errors){
      std::cout << _e.what() << std::endl;
    }
    bool throws = !_errors.empty();
    _errors.clear();
    if (throws) throw MoschusError(MoschusErrorType::MoschusGeneric);
  }
};