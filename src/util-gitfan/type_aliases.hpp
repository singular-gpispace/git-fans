#ifndef TYPE_ALIASES_H
#define TYPE_ALIASES_H

#include <we/type/value.hpp>
#include <pnetc/type/list_range.hpp>
#include <pnetc/type/range.hpp>
#include <pnetc/type/problem_data.hpp>
#include <pnetc/type/directory.hpp>
#include <pnetc/type/file.hpp>
#include <pnetc/type/optional_file.hpp>
#include <pnetc/type/config.hpp>

namespace gitfan
{
  using pnet_value = pnet::type::value::value_type;
  using pnet_list = std::list<pnet_value>;
  using pnet_map = std::map<pnet_value, pnet_value>;
  using pnet_set = std::set<pnet_value>;
  using pnet_list_of_sets = std::list<pnet_set>;
  using pnet_problem_data = pnetc::type::problem_data::problem_data;
  using pnet_list_range = pnetc::type::list_range::list_range;
  using pnet_range = pnetc::type::range::range;
  using pnet_directory = pnetc::type::directory::directory;
  using pnet_file = pnetc::type::file::file;
  using pnet_optional_file = pnetc::type::optional_file::optional_file;
  using pnet_config = pnetc::type::config::config;

  struct Keys
  {
    public:
      inline static const std::string& ELEMENTS()
        { static std::string const _("elements"); return _; }
      inline static const std::string& GROUP_ACTION()
        { static std::string const _("group_action"); return _; }
  };

  // Provide non-overloaded wrapper for boost::get.
  template <typename T>
  const T& as (const gitfan::pnet_value& v)
  {
    return boost::get<T> (v);
  }
}

#endif
