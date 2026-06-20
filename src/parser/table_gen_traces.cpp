#include "table_gen.hpp"

size_t ConflictTrace::trace_length() const {
  if (_terminal) return 1;
  size_t len = 0;
  for (const auto& subtrace : _trace){
    len += subtrace.trace_length();
  }
  return len;
}

void ConflictTrace::push_subtrace(const ConflictTrace& subtrace){
  _trace.push_back(subtrace);
}

std::string ConflictTrace::format_trace() const {
  return "";
}
