#pragma once

#include <storage/options.hpp>

#include <boost/filesystem.hpp>

#include <vector>
#include <functional>

namespace gitfan {
namespace storage
{
  using Cone = std::string;

  /**
   * Interfaces a storage of cones. Added cones are marked initially and each
   * marked cone may be retrieved once by unmarkCones().
   */
  class IConeStorage
  {
    public:
      static std::vector<std::string> availableImplementations();
      static std::string defaultImplementation();

      static std::unique_ptr<IConeStorage> build(const std::string& options);

      /**
       * Add a marked cone to the storage. If the cone already exists (marked or
       * unmarked), nothing happens. Implementations have to guarantee thread
       * safety for this method.
       *
       * @return true, if a new cone has been added successfully. false
       * otherwise.
       */
      virtual bool addMarkedCone(const Cone& cone) = 0;

      /**
       * Signals the storage that a transaction of adding cones has finished.
       */
      virtual void endOfTransaction(const Cone& cone) {}

      /**
       * Unmarks at most n cones. This method is guaranteed to be invoked
       * sequentially. However, addMarkedCone-invocations may be active.
       *
       * @return all modified cones (at most n)
       */
      virtual std::vector<Cone> unmarkCones(std::size_t n) = 0;

      /**
       * Saves all cones located in the storage to persistent memory (e.g. hard
       * disk)
       *
       * @return false in case of failure
       */
      virtual bool saveConesPersistently
        (const boost::filesystem::path& file) = 0;

      virtual ~IConeStorage() = default;
  };
}
}
