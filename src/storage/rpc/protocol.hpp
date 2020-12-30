#pragma once

#include <storage/conestorage.hpp>

#include <util-generic/serialization/boost/filesystem/path.hpp>

#include <rpc/function_description.hpp>

#include <boost/filesystem/path.hpp>

#include <vector>

namespace gitfan
{
  namespace storage
  {
    namespace rpc
    {
      namespace protocol
      {
        FHG_RPC_FUNCTION_DESCRIPTION (put, bool (Cone));
        FHG_RPC_FUNCTION_DESCRIPTION (finish, void(Cone));
        FHG_RPC_FUNCTION_DESCRIPTION (get, std::vector<Cone> (std::size_t));
        FHG_RPC_FUNCTION_DESCRIPTION (save, bool (boost::filesystem::path));
      }
    }
  }
}
