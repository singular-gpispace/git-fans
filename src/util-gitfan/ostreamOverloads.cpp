#include "ostreamOverloads.hpp"
#include <we/type/value/show.hpp>

namespace std
{
  ostream& operator<<(ostream& o, const vector<int>& vec)
  {
    if (vec.size() != 0)
    {
      o << vec[0];
    }
    for (unsigned int i = 1; i < vec.size(); i++)
    {
      o << " " << vec[i];
    }
    return o;
  }

  ostream& operator<<(ostream& o, const gitfan::pnet_value& value)
  {
    o << pnet::type::value::show(value);
    return o;
  }

  ostream& operator<<(ostream& o, const gitfan::pnet_list_range& vec)
  {
    o << "Start: " << vec.start << endl;
    o << "Length: " << vec.length << endl;
    return o;
  }
}
