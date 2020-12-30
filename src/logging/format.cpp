#include <logging/format.hpp>

#include <sstream>

namespace gitfan {
namespace logging
{
  std::string withNoLineBreaks(const std::string& in)
  {
    std::stringstream out;
    for (char c : in)
    {
      switch (c)
      {
        case '\r':
          out << '\\' << 'r';
          break;
        case '\n':
          out << '\\' << 'n';
          break;
        case '\\':
          out << '\\' << '\\';
          break;
        default:
          out << c;
      }
    }
    return out.str();
  }

  std::string withLineBreaks(const std::string& in)
  {
    std::stringstream out;
    bool escaped = false;
    for (char c : in)
    {
      if (escaped)
      {
        switch(c)
        {
          case 'r':
            out << '\r';
            break;
          case 'n':
            out << '\n';
            break;
          case '\\':
            out << '\\';
            break;
          default:
            throw std::runtime_error
              ("Unknown escape sequence in \"" + in + "\".");
        }
        escaped = false;
      }
      else
      {
        if (c == '\\')
        {
          escaped = true;
        }
        else
        {
          out << c;
        }
      }
    }
    return out.str();
  }
}
}
