#include <gtest/gtest.h>

#include <stdexcept>

#include <util-gitfan/simplexfaceiterator.hpp>

namespace gitfan {
namespace testing
{
  TEST(SimplexFaceIteratorTest, defaultOutputsEmptySetOnly)
  {
    // SimplexFaceIterator is not implemented in a typical STL iterator pattern
    SimplexFaceIterator it;
    EXPECT_FALSE(it.isEnd());
    EXPECT_EQ((std::vector<int>()), *it);
    EXPECT_EQ(1u, it.increment(1));
    EXPECT_TRUE(it.isEnd());
    EXPECT_EQ(0u, it.increment(3));
  }

  TEST(SimplexFaceIteratorTest, dereferencingAtEndThrows)
  {
    SimplexFaceIterator it(1,1);
    while (!it.isEnd())
    {
      ++it;
    }
    EXPECT_THROW(*it, std::out_of_range);
  }

  TEST(SimplexFaceIteratorTest, canBeConstructedWithCorrectInitialValue)
  {
    {
      SimplexFaceIterator it(5,3);
      EXPECT_EQ((std::vector<int>{1,2,3}), *it);
    }
    {
      const std::vector<int> start {2, 4, 5};
      SimplexFaceIterator it(5,start);
      EXPECT_EQ(start, *it);
    }
  }

  TEST(SimplexFaceIteratorTest, canStepWithVariousOffsets)
  {
    for (unsigned int step = 1; step < 10; step++)
    {
      unsigned int remainingSteps = 16; // (5 over 3) + (5 over 4) + (5 over 5)
      SimplexFaceIterator it(5,3);
      while (remainingSteps >= step)
      {
        EXPECT_FALSE(it.isEnd());
        EXPECT_EQ(step, it.increment(step));
        remainingSteps -= step;
      }
      EXPECT_EQ(remainingSteps, it.increment(step));
      EXPECT_TRUE(it.isEnd());
    }
  }

  TEST(SimplexFaceIteratorTest, overloadedOperatorsAndReturnSamples)
  {
    SimplexFaceIterator it(5,3);
    ++it;
    EXPECT_EQ((std::vector<int>{1,2,4}), *it);
    it += 5;
    EXPECT_EQ((std::vector<int>{2,3,4}), *it);
    it += 6;
    EXPECT_EQ((std::vector<int>{1,2,4,5}), *it);
    it += 3;
    EXPECT_EQ((std::vector<int>{1,2,3,4,5}), *it);
    ++it;
    EXPECT_TRUE(it.isEnd());
  }
}}
