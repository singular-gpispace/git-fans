#include <gtest/gtest.h>

#include <util-gitfan/singular_commands.hpp>
#include <util-gitfan/conversion.hpp>

using namespace gitfan;
using namespace gitfan::conversion;

namespace singular {
namespace testing
{
  TEST(SingularCommandsTest, canPutIntvecAndList)
  {
    EXPECT_TRUE(init());
    intvec* v = createIntvec(pnet_list { 1, 2, 3});
    put("v", v);
    call_and_discard("intvec expected_v = 1,2,3;");
    EXPECT_EQ("1", get_result("expected_v == v;"));

    lists l = static_cast<lists>(omAllocBin (slists_bin));
    l->Init(3);
    for (unsigned int i = 0; i < 3; i++)
    {
      l->m[i].rtyp = INT_CMD;
      l->m[i].data = reinterpret_cast<void*>(i+1);
    }
    put("l", l);
    call_and_discard("list expected_l = 1,2,3;");
    EXPECT_EQ("1", get_result("string(expected_l) == string(l);"));
  }
}}
