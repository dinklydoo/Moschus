#include "moschus_error.hpp"

enum class EXCEPTION_FLAGS {
  THROW_IMMEDIATELY = 1ull,
  THROW_ON_WARNING = (1ull << 1),
  IGNORE_WARNINGS = (1ull << 2)
};

using FLAG_MASK = unsigned long long;

namespace MoschusExceptHandler {
  inline FLAG_MASK flags = 0;

  void set_flag(EXCEPTION_FLAGS);

  void push_warning(MoschusWarning);
  void push_error(MoschusError);

  void log_warnings();
  void log_errors();
}