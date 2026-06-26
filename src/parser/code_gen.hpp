#pragma once

#include "frontend/musk_parser.hpp"

#include <string>

namespace CodeGenerator {
  void generate_parser(const std::string& OUTPUT_DIR, const musk_ptr& ast);
}
