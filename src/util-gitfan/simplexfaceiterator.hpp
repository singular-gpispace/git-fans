#ifndef SIMPLEXFACEITERATOR_H
#define SIMPLEXFACEITERATOR_H

#include <vector>

namespace gitfan
{
  class SimplexFaceIterator
  {
    private:
      std::vector<int> cur;
      unsigned int n;
      unsigned int k;

    public:
      SimplexFaceIterator();
      SimplexFaceIterator(unsigned int n, const std::vector<int>& start);
      SimplexFaceIterator(unsigned int n, unsigned int k);

      SimplexFaceIterator& operator++();
      SimplexFaceIterator& operator+=(unsigned long inc);

      bool isEnd() const;
      unsigned long increment(unsigned long inc);

      const std::vector<int>& operator*() const;
  };
}
#endif
