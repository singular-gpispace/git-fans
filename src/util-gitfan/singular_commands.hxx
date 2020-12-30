#include <util-generic/divru.hpp>
#include <sstream>
#include <boost/range/adaptor/transformed.hpp>

namespace singular
{
  inline void kill() {}
  template<typename T, typename... Args>
  void kill(const T& t, Args&&... args)
  {
    killid(t, &IDROOT);
    kill(std::forward<Args>(args)...);
  }
}
