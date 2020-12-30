#ifndef CONVERSION_H
#define CONVERSION_H

#include "type_aliases.hpp"

#include <vector>
#include <ios>
#include <Singular/libsingular.h>
#include <boost/filesystem.hpp>

namespace gitfan {
namespace conversion
{
  template <typename T>
  std::vector<T> unwrap(const pnet_list& list);
  template <typename T>
  pnet_list wrap(const std::vector<T>& vector);

  inline intvec* createIntvec(const std::vector<int>& vector);
  inline intvec* createIntvec(const pnet_list& list);

  inline lists toSingularList
    ( const pnet_list& list
    , bool lastLayerAsIntvec = false
    );

  template <class OutputIt>
  inline void insertSingularObj(const sleftv& obj, OutputIt dest);
  inline pnet_list toList(lists singularList);
  inline pnet_list toList(const intvec* const iv);
  template <class OutputIt>
  void insert(lists singularList, OutputIt dest);
  template <class OutputIt>
  void insert(const intvec* const iv, OutputIt dest);

  inline std::string listToHexString(const pnet_list& list, int maxValue);
  inline pnet_list hexStringToList(const std::string& hexString);

  inline pnet_list loadListPartially
    ( const boost::filesystem::path& file
    , std::ios::pos_type pos
    , unsigned long linesToRead
    );

  inline pnet_list loadList_TESTING_ONLY(const boost::filesystem::path& file);
}}

#include "conversion.hxx"

#endif
