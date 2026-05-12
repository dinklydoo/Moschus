#pragma once
#include "musk_tokens.hpp"
#include <string>
#include <vector>

struct MuskError;
struct MuskWarning;

using musk_errors = std::vector<MuskError>;
using musk_warnings = std::vector<MuskWarning>;

struct MuskError {
    std::string msg;
    Location loc;
};

struct MuskWarning {
    std::string msg;
    Location loc;
};


void eof_error(musk_errors& err);

void log_errors(const musk_errors& err);
void log_warnings(const musk_warnings& warn);