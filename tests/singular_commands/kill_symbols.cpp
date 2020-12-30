#include <gtest/gtest.h>

#include <util-gitfan/singular_commands.hpp>

namespace singular {
namespace testing
{
  TEST(SingularCommandsTest, canKillOneOrTwoSymbols)
  {
    EXPECT_TRUE(init());

    call_and_discard("int i = 1;");
    EXPECT_TRUE(symbol_exists("i"));
    kill("i");
    EXPECT_FALSE(symbol_exists("i"));

    call_and_discard("int i = 1;");
    call_and_discard("int j = 1;");
    EXPECT_TRUE(symbol_exists("i"));
    EXPECT_TRUE(symbol_exists("j"));
    kill("i", "j");
    EXPECT_FALSE(symbol_exists("i"));
    EXPECT_FALSE(symbol_exists("j"));
  }
}}
