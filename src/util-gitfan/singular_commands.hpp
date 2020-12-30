#ifndef SINGULAR_COMMANDS_H
#define SINGULAR_COMMANDS_H

#include "type_aliases.hpp"
#include <Singular/libsingular.h>

namespace singular
{
  /**
   * @return true, if singular has been successfully initialized.
   * false, if singular already has been initialized
   */
  bool init ();

  void call (std::string const& command);

  void call_and_discard (std::string const& command);

  std::string get_result (std::string const& command);

  void load_library (std::string const& library_name,
                     bool enforce_reload = false);

  void load_ssi (const std::string& symbol_name,
                 const ::gitfan::pnet_value& files);

  void load_ssi_if_not_defined (const std::string& symbol_name,
                                const ::gitfan::pnet_value& files);

  void write_ssi (const std::string& symbol_name, std::string const& file_name);

  bool symbol_exists(const std::string& symbol_name);

  intvec* getIntvec(const std::string& symbol);

  lists getList(const std::string& symbol);

  void put(const std::string& symbol, lists list);

  void put(const std::string& symbol, intvec* iv);

  template<typename T, typename... Args>
  void kill(const T& t, Args&&... args);
}

#include "singular_commands.hxx"

#endif
