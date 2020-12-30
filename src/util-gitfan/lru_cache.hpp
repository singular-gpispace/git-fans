#pragma once

#include <unordered_map>
#include <functional>
#include <list>
#include <utility>

namespace gitfan
{
  template <class Key, class T>
  class LRUCache
  {
  public:
    LRUCache(std::size_t size)
      : _size(size)
      , _values(2 * size)
    {
    }

    T get(const Key& key, const std::function<T(const Key&)>& factory)
    {
      if (_size == 0)
      {
        return factory(key);
      }

      _lru.push_front(key);
      auto pos = _values.find(key);
      if (pos == _values.end())
      {
        pos = _values.insert(
          { key
          , std::pair<typename std::list<Key>::const_iterator,T>
            ( _lru.cbegin()
            , factory(key)
            )
          }).first;
        if (_lru.size() > _size)
        {
          _values.erase(_values.find(_lru.back()));
          _lru.pop_back();
        }
      }
      else
      {
        _lru.erase(pos->second.first);
        pos->second.first = _lru.cbegin();
      }
      return pos->second.second;
    }

  private:
    const std::size_t _size;
    std::unordered_map< Key
                      , std::pair<typename std::list<Key>::const_iterator,T>
                      > _values;
    std::list<Key> _lru;
  };
}
