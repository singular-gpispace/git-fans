#ifndef OSTREAM_OVERLOADS_H
#define OSTREAM_OVERLOADS_H

#include "type_aliases.hpp"
#include <vector>
#include <iostream>

namespace std
{
  ostream& operator<<(ostream& o, const vector<int>& vec);

  ostream& operator<<(ostream& o, const gitfan::pnet_value& value);

  ostream& operator<<(ostream& o, const gitfan::pnet_list_range& vec);
}

#endif
