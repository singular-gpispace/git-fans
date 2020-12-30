#include "singular_commands.hpp"

#include <boost/range/adaptor/transformed.hpp>

#include <util-generic/finally.hpp>

#include <pnetc/type/file/op.hpp>

#include <we/type/value.hpp>

#include <stdexcept>
#include <config.hpp>

namespace singular
{
  static void error_callback(const char* msg)
  {
    throw std::runtime_error("Singular error: " + std::string(msg));
  }

  bool init ()
  {
    if (currPack == NULL)
    {
      mp_set_memory_functions (omMallocFunc, omReallocSizeFunc, omFreeSizeFunc);
      siInit (const_cast<char*> (config::singularLibrary().string().c_str()));
      currentVoice = feInitStdin (NULL);
      errorreported = 0;
      myynest = 1;
      WerrorS_callback=error_callback;
      return true;
    }
    return false;
  }

  void call (std::string const& command)
  {
    int err = iiAllStart
      (NULL, const_cast<char*> ((command + " return();").c_str()), BT_proc, 0);
    if (err)
    {
      errorreported = 0;
      throw std::runtime_error (
        "Singular returned an error at \"" + command + "\"");
    }
  }

  void call_and_discard (std::string const& command)
  {
    SPrintStart();
    call (command);
    char* result_ptr = SPrintEnd();
    omFree (result_ptr);
  }

  std::string get_result (std::string const& command)
  {
    SPrintStart();
    call (command);
    char* result_ptr = SPrintEnd();
    std::string result (result_ptr);
    omFree (result_ptr);
    if (result.size() > 0)
    {
      result.pop_back();
    }
    return result;
  }

  void load_library (const std::string& library_name, bool enforce_reload)
  {
    bool load = enforce_reload;
    if (!load)
    {
      char *plib = iiConvName(library_name.c_str());
      FHG_UTIL_FINALLY([=] { omFree((ADDRESS)plib); });
      idhdl pl = basePack->idroot->get(plib,0);
      load = !((pl!=NULL)
        && IDTYP(pl)==PACKAGE_CMD
        && IDPACKAGE(pl)->language == LANG_SINGULAR);
    }
    if (load)
    {
      call_and_discard ("LIB \"" + library_name + "\";");
    }
  }

  void load_ssi_no_def( const std::string& symbol_name
                      , const gitfan::pnet_value& files)
  {
    if (files.type() == typeid(gitfan::pnet_list))
    {
      call_and_discard(symbol_name + " = list();");
      int i = 1;
      for ( const gitfan::pnet_value& elem
          : gitfan::as<gitfan::pnet_list>(files)
          )
      {
        load_ssi_no_def(symbol_name + "[" + std::to_string(i) + "]", elem);
        i++;
      }
    }
    else if (files.type() == typeid(pnet::type::value::structured_type))
    {
      const gitfan::pnet_file file = pnetc::type::file::from_value(files);
      call_and_discard(symbol_name + " = read(\"ssi:r " + file.path + "\");");
    }
    else if (files.type() == typeid(std::string))
    {
      call_and_discard(symbol_name + " = read(\"ssi:r "
        + gitfan::as<std::string>(files) + "\");");
    }
    else
    {
      throw std::runtime_error("Loading ssi files from nested pnet_lists "
          "containing other leaves than pnet_file or string is not supported.");
    }
  }

  void load_ssi (const std::string& symbol_name,
                 const gitfan::pnet_value& files)
  {
    call_and_discard("def " + symbol_name + ";");
    load_ssi_no_def(symbol_name, files);
  }

  void load_ssi_if_not_defined (const std::string& symbol_name,
                                const gitfan::pnet_value& files)
  {
    if (!symbol_exists(symbol_name))
    {
      load_ssi(symbol_name, files);
    }
  }

  void write_ssi (std::string const& symbol_name, std::string const& file_name)
  {
    call_and_discard ("link l = \"ssi:w " + file_name + "\"; write(l,"
      + symbol_name + "); close(l); kill l;");
  }

  bool symbol_exists(std::string const& symbol_name)
  {
    return ggetid(symbol_name.c_str()) != NULL;
  }

  intvec* getIntvec(const std::string& symbol)
  {
    idhdl h = ggetid(symbol.c_str());
    if (IDTYP(h) == INTVEC_CMD)
    {
      return IDINTVEC(h);
    }
    else
    {
      throw std::runtime_error(
        "Symbol " + symbol + " does not represent an intvec.");
    }
  }

  lists getList(const std::string& symbol)
  {
    idhdl h = ggetid(symbol.c_str());
    if (IDTYP(h) != LIST_CMD)
    {
      throw std::runtime_error(
        "Symbol " + symbol + " does not represent a list.");
    }
    else
    {
      return IDLIST(h);
    }
  }

  void put(const std::string& symbol, lists list)
  {
    idhdl handle = enterid(symbol.c_str(), 1, LIST_CMD, &IDROOT, FALSE);
		IDLIST(handle) = list;
  }

  void put(const std::string& symbol, intvec* iv)
  {
    idhdl handle = enterid(symbol.c_str(), 1, INTVEC_CMD, &IDROOT, FALSE);
		IDINTVEC(handle) = iv;
  }
}
