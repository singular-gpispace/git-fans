#include "simplexfaceiterator.hpp"

#include <numeric>
#include <stdexcept>

using namespace std;

namespace
{
  std::vector<int> make_iota_vector (unsigned int k)
  {
    std::vector<int> result (k);
    std::iota (result.begin(), result.end(), 1);
    return result;
  }
}

namespace gitfan
{
  int startOfOneBlock(const vector<int>& sparseOneArray, int end)
  {
    int pos = end - 1;
    while (pos >= 0 && sparseOneArray[pos] + 1 == sparseOneArray[pos+1])
    {
      pos--;
    }
    return pos + 1;
  }

  SimplexFaceIterator::SimplexFaceIterator()
    : SimplexFaceIterator(0, vector<int>{})
  {}

  SimplexFaceIterator::SimplexFaceIterator(unsigned int n, unsigned int k)
    : SimplexFaceIterator(n, make_iota_vector(k))
  {}

  SimplexFaceIterator::SimplexFaceIterator(unsigned int n, const vector<int>& start)
    : n(n), k(start.size())
  {
    cur = vector<int>(start);
  }

  SimplexFaceIterator& SimplexFaceIterator::operator++()
  {
    return *this += 1;
  }

  SimplexFaceIterator& SimplexFaceIterator::operator+=(unsigned long inc)
  {
    increment(inc);
    return *this;
  }

  const vector<int>& SimplexFaceIterator::operator*() const
  {
    if (isEnd())
    {
      throw std::out_of_range("Dereferencing iterator at its end.");
    }
    return cur;
  }

  bool SimplexFaceIterator::isEnd() const
  {
    return k > n;
  }

  unsigned long SimplexFaceIterator::increment(unsigned long inc)
  {
    if (isEnd())
    {
      return 0;
    }
    unsigned long start_inc = inc;

    while (inc > 0 && k > 0)
    {
      // The amount of consecutive zeros at the end.
      unsigned int zeroCount = n - cur[k-1];

      if (zeroCount == 0)  // Last bit is 1
      {
        // Push the last one not occuring in the rightmost block of 1's to the
        // right and append the rightmost block of 1's.
        int oneIndex = startOfOneBlock(cur, k-1) - 1;

        // All 1's are already pushed to the right
        if (oneIndex < 0) { break; }

        cur[oneIndex]++;
        for (unsigned int i = oneIndex + 1; i < k; i++)
        {
          cur[i] = cur[i-1] + 1;
        }

        inc--;
      }
      // Last bit is 0 + last 1 can be shifted by "inc" steps
      else if (zeroCount >= inc)
      {
        cur[k-1] += inc;
        inc = 0;
      }
      // Last bit is 0 + multiple 1's have to be shifted
      else
      {
        int start = startOfOneBlock(cur, k-1);
        unsigned int shiftedBlockSize = 1;
        unsigned long diff = zeroCount + 1;
        while (shiftedBlockSize <= k-1 - start)
        {
          shiftedBlockSize++;
          unsigned long newDiff =
            diff * (zeroCount + shiftedBlockSize) / shiftedBlockSize;
          if (inc < newDiff - 1) {
            shiftedBlockSize--;
            break;
          }
          else
          {
            diff = newDiff;
          }
        }
        // Shift 1's-block of size "shiftedBlockSize"
        for (unsigned int i = 0; i < shiftedBlockSize; i++)
        {
          cur[k-1-i] = n-i;
        }
        inc -= diff - 1;
      }
    }

    if (inc > 0)
    {
      if (k < n)
      {
        for (unsigned int i = 0; i < k; i++)
        {
          cur[i] = 1 + i;
        }
        cur.push_back(k+1);
        k++;
        inc--;
        return increment(inc) + start_inc - inc;
      }
      else
      {
        inc--;
        k = n + 1;
        cur.clear();
      }
    }
    return start_inc - inc;
  }
}
