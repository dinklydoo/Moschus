#include "musk_error.hpp"
#include <iostream>

void eof_error(musk_errors &err){
    err.push_back(
        MuskError(
            "Unexpected end of musk file, musk file is missing key blocks",
            Location(-1, -1)
        )
    );
}

std::string format_location(const Location& loc){
    if (loc.line < 0 && loc.col < 0) return "";
    return std::format("line({}:{})", loc.line, loc.col);
}


void log_errors(const musk_errors &err){
    if (err.empty()) return;
    for (const auto& error : err){
        std::cout << "\033[1;31m"<<format_location(error.loc)<<" musk error: "<< error.msg << "\033[0m\n";
    }
    exit(1);
}

void log_warnings(const musk_warnings &warn){
    for (const auto& warning : warn){
        std::cout << "\033[1;31m"<<format_location(warning.loc)<<" musk warning: "<< warning.msg << "\033[0m\n";
    }
}