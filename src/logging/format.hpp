#pragma once

#include <string>

namespace gitfan {
namespace logging {
  std::string withNoLineBreaks(const std::string& in);
  std::string withLineBreaks(const std::string& in);
}
}
