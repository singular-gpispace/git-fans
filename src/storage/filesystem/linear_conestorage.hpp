#pragma once

#include <storage/conestorage.hpp>
#include <storage/options.hpp>

namespace gitfan {
namespace storage {
namespace filesystem
{
  /**
   * A ConeFileStorage implements the IConeStorage interface by managing all
   * cones in the file system. Each cone corresponds to a subdirectory. Marked
   * cones contains a file in the corresponding directory. If the underlying
   * file system ensures thread safety for "atomic" operations (mkdir, touch,
   * rm), this ConeFileStorage preserves thread safety for executing arbitrary
   * many addMarkedCone() operations and one unmarkCones() operation at once.
   */
  class LinearConeStorage : public IConeStorage
  {
    private:
      const boost::filesystem::path storageDirectory;
      const size_t subdirectoryPathLength;

      Cone getCone(const boost::filesystem::path& path);
      boost::filesystem::path pathForCone(const Cone& cone);

    public:
      LinearConeStorage(const Options& options);

      // IConeStorage members
      virtual bool addMarkedCone(const Cone& cone) override;
      virtual std::vector<Cone> unmarkCones(size_t n) override;
      virtual bool saveConesPersistently
        (const boost::filesystem::path& file) override;
  };
}
}
}
