#pragma once

#include <boost/optional.hpp>

#include <stdexcept>

namespace gitfan {
namespace logging {
namespace statistics
{
  template <typename T>
  class Distribution
  {
    public:
      void add(T elem)
      {
        _sum += elem;
        _squareSum += elem*elem;
        if (!_max || *_max < elem)
        {
          _max = elem;
        }
        if (!_min || *_min > elem)
        {
          _min = elem;
        }
        _count++;
      }

      unsigned long count() const
      {
        return _count;
      }

      T sum() const
      {
        return _sum;
      }

      T max() const
      {
        if (!_max)
        {
          throw std::runtime_error
            ("Trying to retrieve a maximum over an empty set.");
        }
        return *_max;
      }

      T min() const
      {
        if (!_min)
        {
          throw std::runtime_error
            ("Trying to retrieve a minimum over an empty set.");
        }
        return *_min;
      }

      T mean() const
      {
        if (_count == 0)
        {
          throw std::runtime_error
            ("Trying to compute a mean over an empty set.");
        }
        return _sum / _count;
      }

      T variance() const
      {
        if (_count == 0)
        {
          throw std::runtime_error
            ("Trying to compute a variance over an empty set.");
        }
        return (_squareSum * _count - _sum * _sum) / (_count * _count);
      }
    private:
      T _sum = T();
      T _squareSum = T();
      boost::optional<T> _max;
      boost::optional<T> _min;
      unsigned long _count = 0;
  };
}
}
}
