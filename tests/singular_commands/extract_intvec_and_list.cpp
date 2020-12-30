#include <gtest/gtest.h>

#include <util-gitfan/singular_commands.hpp>
#include <util-gitfan/conversion.hpp>
#include <util-gitfan/ostreamOverloads.hpp>

using namespace gitfan;
using namespace gitfan::conversion;

namespace singular {
namespace testing
{
  TEST(SingularCommandsTest, canExtractIntvecAndList)
  {
    EXPECT_TRUE(init());
    call_and_discard("intvec v = 1,2,3;");
    call_and_discard("list l = 1,2,3;");
    const pnet_list expected { 1, 2, 3 };

    intvec* v = getIntvec("v");
    const pnet_list intvec = toList(v);
    EXPECT_EQ(expected, intvec);

    lists l = getList("l");
    const pnet_list list = toList(l);
    EXPECT_EQ(expected, list);
  }
}}
