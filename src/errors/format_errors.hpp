#pragma once

#include <string>
#include "../parser/frontend/musk_tokens.hpp"

void register_file_path(const std::string& fpath);
std::string format_message_loc(const std::string& err_msg, const Location& loc);
std::string format_file_except(const std::string& err_msg, bool error, const Location& start_loc, const Location& end_loc);