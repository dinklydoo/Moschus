#pragma once
#include <concepts>
#include <string>

enum struct Color {
nothing,
red,
yellow,
magenta,
green,
cyan,
white,
black
};

enum struct Modifier {
nothing,
underline,
squiggly,
bold,
italic,
};

namespace {
#define ANSI_esc std::string("\033[")
#define ANSI_end std::string("\033[0m")
};

struct MoschusString {
  private:
    std::string content;
    Color color;
    Modifier modifier;
  public:

    template<typename... Args>
    requires (std::same_as<Args, const char*> && ...)
    MoschusString(Args... _content) : color(Color::nothing), modifier(Modifier::nothing) {
      content = (_content + ...);
    }

    MoschusString(Color _color, const char* _content, ...) : MoschusString(_content) {
      color = _color;
    }

    MoschusString(Color _color, Modifier _modifier, const char* _content, ...) : MoschusString(_color, _content) {
      modifier = _modifier;
    }

    // transform a moschus_string to a cstring
    const std::string to_string() const {
      std::string constructed_str = content;
      std::string _mod = ANSI_esc;
      switch (modifier){
        case Modifier::underline :
          _mod += "4"; break;
        case Modifier::bold :
          _mod += "1"; break;
        case Modifier::italic :
          _mod += "3"; break;
        case Modifier::squiggly :
          constructed_str += std::string("\n")+std::string(content.size(), '^');
        default: _mod = "";
      }

      std::string _col;
      if (color != Color::nothing){
        switch (color){
          case Color::green :
            _col = "32"; break;
          case Color::cyan :
            _col = "36"; break;
          case Color::magenta :
            _col = "35"; break;
          case Color::red :
            _col = "31"; break;
          case Color::yellow :
            _col = "33"; break;
          case Color::white :
            _col = "37"; break;
          case Color::black :
            _col = "30"; break;
          default : _col = "";
        }
      }
      if (!_col.empty()){
        if (_mod.empty()){
          _mod = ANSI_esc+_col;
        } else {
          _mod += ";"+_col;
        }
      }
      if (!_mod.empty()){
        _mod+="m";
        constructed_str.insert(0, _mod);
        constructed_str+=ANSI_end;
      }
      return constructed_str;
    }
};
