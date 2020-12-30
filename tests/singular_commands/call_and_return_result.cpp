#include <gtest/gtest.h>

#include <util-gitfan/singular_commands.hpp>

namespace singular {
namespace testing
{
  TEST(SingularCommandsTest, canCallInterpreterAndReturnResult)
  {
    EXPECT_TRUE(init());
    call("intvec i = 1,2,3;");
    EXPECT_EQ("1,2,3", get_result("i;"));
  }
}}
