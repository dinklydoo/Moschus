#include "format_errors.hpp"
#include "moschus_string.hpp"

#include <fstream>

namespace {
  std::string FILE_PATH;
  std::vector<std::string> MUSK_FILE_IN;

  // read all lines in .musk file and cache for individual access
  void read_file_in(const std::string& fpath){
    if (fpath == FILE_PATH) return;

    FILE_PATH = fpath;
    MUSK_FILE_IN.clear();

    std::ifstream MUSK_FILE(fpath);
    std::string read_line;
    while (std::getline(MUSK_FILE, read_line)){
      MUSK_FILE_IN.push_back(read_line);
    }
  }

  // read a specific line in .musk file
  std::string read_line(int line_no){    
    if (line_no <= 0 || line_no >= MUSK_FILE_IN.size()){
      return "Error: line out of range";
    }
    return MUSK_FILE_IN[line_no-1];
  }

  // get length of a number in string form
  size_t num_len(int n){
    size_t res = 0;
    while (n > 0){
      res++;
      n /= 10;
    }
    return res;
  }
}

void register_file_path(const std::string& fpath){
  read_file_in(fpath);
}

// Format a message with a location preamble ex. "(row:col) error_message"
std::string format_message_loc(const std::string& err_msg, const Location& loc){
  std::string res;
  if (loc.line > 0 && loc.col > 0) res += std::format("line({}:{}) ", loc.line, loc.col);
  res += err_msg;
  return res;
}

/*
  EXAMPLE TEMPLATING FOR FILE WINDOW EXCEPT MESSAGES

  Line Numbers: coloured green
  Error message (TOP): coloured red (for error), coloured magenta (for warning)
  Single line: erraneous chars (b/w start_loc.col and end_loc.col) should be red/magenta
  Single underline: coloured green
  Multiple line: all chars spanning start_loc to end_loc should be red/magenta

  Example: single line error (start_loc.line == end_loc.line)

  (1:1) Moschus Error: Symbol "APPLE" is undefined
    |
  1 | APPLE -> B {
    | ^^^^^
  
  Example: multiple line error (start_loc.line < end_loc.line)

  (12, 0) Moschus Error: ... placeholder err ...
     |
  12 | A -> B C D {
  13 |   ... some code ...
  14 | }
     | 

  @param -- FILE_PATH: .musk file path
  @param -- err_msg: top level descriptive error message (single line)
  @param -- error: is this an error(true) or a warning(false)
  @param -- start_loc: start of erraneous code
  @param -- end_loc: end of erraneous code
*/
std::string format_file_except(const std::string& err_msg, bool error, const Location& start_loc, const Location& end_loc){
  Color highlight_color = (error)? Color::red : Color::magenta;

  std::string result = MoschusString(highlight_color, err_msg.data()).to_string();
  result += '\n'; // new-line after error message
  size_t line_count_pad = std::max(num_len(start_loc.line), num_len(end_loc.line)) + 1;

  // add a padding line above critical section
  result += std::string(line_count_pad, ' ')+"|\n";
  
  if (start_loc.line == end_loc.line){
    result += MoschusString(Color::green, std::to_string(start_loc.line).data()).to_string() + " | ";
    const std::string& file_line = read_line(start_loc.line);

    int critical_length = end_loc.col - start_loc.col;

    result += file_line.substr(0, start_loc.col-1);
    result += MoschusString(highlight_color, file_line.substr(start_loc.col-1, critical_length).data()).to_string();
    result += file_line.substr(end_loc.col-1);

    result += '\n'; // end the critical line

    // end pad line, with underline of critical section
    result += std::string(line_count_pad, ' ')+"|";
    result += std::string(start_loc.col, ' ');
    result += MoschusString(Color::green, std::string(critical_length, '^').data()).to_string()+'\n';
  } else {
    for (int curr_line = start_loc.line; curr_line <= end_loc.line; curr_line++){
      result += MoschusString(Color::green, std::to_string(curr_line).data()).to_string() + " | ";
      const std::string& file_line = read_line(curr_line);

      result += MoschusString(highlight_color, file_line.data()).to_string() + '\n';
    }
    // add a padding line below critical section
    result += std::string(line_count_pad, ' ')+"|\n";
  }
  return result;
}