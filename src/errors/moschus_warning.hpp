#pragma once

#include <vector>
#include <iostream>
#include "moschus_string.hpp"

struct MoschusWarning {
  MoschusString warnstr;
};

struct MoschusWarningHandler {
  private:
    static MoschusWarningHandler* instance;
    std::vector<MoschusWarning> warnings;

    MoschusWarningHandler() = default;
  public:
    static MoschusWarningHandler* Instance() {
      if (MoschusWarningHandler::instance == nullptr) instance = new MoschusWarningHandler();
      return instance;
    }

    void push_warning(MoschusWarning _warning){
      warnings.push_back(_warning);
    }

    void log_warnings(){
      for (auto& _w : warnings){
        std::cout << _w.warnstr.to_string() << std::endl;
      }
      warnings.clear(); // log each warning only once
    }
};
