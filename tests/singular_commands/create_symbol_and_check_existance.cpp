#include <gtest/gtest.h>

#include <util-gitfan/singular_commands.hpp>

namespace singular {
namespace testing
{
  TEST(SingularCommandsTest, createSymbolAndCheckItsExistance)
  {
    EXPECT_TRUE(init());
    EXPECT_FALSE(symbol_exists("S"));
    call_and_discard("int S = 0;");
    EXPECT_TRUE(symbol_exists("S"));
    call_and_discard("kill S;");
    EXPECT_FALSE(symbol_exists("S"));
  }
}}
