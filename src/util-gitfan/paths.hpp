#include <boost/filesystem.hpp>

namespace gitfan
{
  class Paths
  {
    public:
      static inline boost::filesystem::path orbitConesDirectory
        (const boost::filesystem::path& workingDirectory)
      {
        return workingDirectory / "result" / "orbit_cones";
      }

      static inline boost::filesystem::path intermediateOrbitInfoDirectory
        (const boost::filesystem::path& workingDirectory)
      {
        return workingDirectory / "tmp";
      }

      static inline boost::filesystem::path storageDirectory
        (const boost::filesystem::path& workingDirectory)
      {
        return workingDirectory / "storage";
      }

      static inline boost::filesystem::path orbitConeOrderFile
        (const boost::filesystem::path& workingDirectory)
      {
        return workingDirectory / "result" / "orbit_cones" /"list";
      }

      static inline boost::filesystem::path symmetryGroupOnHashesFile
        (const boost::filesystem::path& workingDirectory)
      {
        return workingDirectory / "result" / "symmetry_group_on_hashes";
      }

      static inline boost::filesystem::path gitConesFile
        (const boost::filesystem::path& workingDirectory)
      {
        return workingDirectory / "result" / "git_cones";
      }

      static inline boost::filesystem::path loadGitConesSingularFile
        (const boost::filesystem::path& workingDirectory)
      {
        return workingDirectory / "load_git_cones.sing";
      }

      static inline boost::filesystem::path loadGitConesSingularLibrary
        (const boost::filesystem::path& workingDirectory)
      {
        return workingDirectory / "load_git_cones.lib";
      }
  };
}
