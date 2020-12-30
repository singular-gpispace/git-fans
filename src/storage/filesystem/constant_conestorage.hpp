#pragma once

#include <storage/conestorage.hpp>
#include <storage/options.hpp>
#include <storage/filesystem/string_container.hpp>

namespace gitfan {
namespace storage {
namespace filesystem
{
  /**
   * A ConeFileStorage implements the IConeStorage interface by managing all
   * cones in the file system. Each cone corresponds to a subdirectory. Marked
   * cones are duplicated at another root path. If the underlying file system
   * ensures thread safety for "atomic" operations (mkdir, touch, rm), this
   * ConeFileStorage preserves thread safety for executing arbitrary many
   * addMarkedCone() operations and one unmarkCones() operation at once.
   */
  class ConstantConeStorage : public IConeStorage
  {
    private:
      StringContainer _all;
      StringContainer _marked;

    public:
      ConstantConeStorage(const Options& options);

      // IConeStorage members
      virtual bool addMarkedCone(const Cone& cone) override;
      virtual std::vector<Cone> unmarkCones(size_t n) override;
      virtual bool saveConesPersistently
        (const boost::filesystem::path& file) override;
  };
}
}
}
