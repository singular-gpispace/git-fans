#include <gtest/gtest.h>

#include "main.hpp"

namespace gitfan {
namespace testing {

  std::vector<std::string> _args;

  const std::vector<std::string>& args()
  {
    return _args;
  }

  bool containedInArgs(const std::string& arg)
  {
    const auto pos = std::find(_args.begin(), _args.end(), arg);
    return pos != _args.end();
  }
}}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  for (int i = 1; i < argc; i++)
  {
    gitfan::testing::_args.emplace_back(argv[i]);
  }
  return RUN_ALL_TESTS();
}
