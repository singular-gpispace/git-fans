#include <storage/filesystem/linear_conestorage.hpp>

#include <boost/filesystem/fstream.hpp>

namespace fs = boost::filesystem;

namespace gitfan {
namespace storage {
namespace filesystem
{
  Cone LinearConeStorage::getCone(const fs::path& path)
  {
    std::string pathString(path.string());
    pathString.erase(0, storageDirectory.string().size());
    for (std::size_t pos = pathString.find('/');
         pos != std::string::npos;
         pos = pathString.find('/'))
    {
      pathString.erase(pos, 1);
    }
    return pathString;
  }

  fs::path LinearConeStorage::pathForCone(const Cone& cone)
  {
    fs::path path(storageDirectory);
    const std::string& coding = cone;
    for (size_t pos = 0; pos < coding.size(); pos += subdirectoryPathLength)
    {
      path /= coding.substr(pos, subdirectoryPathLength);
    }
    return path;
  }

  LinearConeStorage::LinearConeStorage
    (const Options& options)
    : storageDirectory(options.workingDirectory)
    , subdirectoryPathLength(options.subdirectoryNameLength)
    {}

  void touch(const fs::path& path)
  {
    fs::ofstream o(path);
    o.close();
  }

  bool LinearConeStorage::addMarkedCone(const Cone& cone)
  {
    fs::path path = pathForCone(cone);
    if (!path.parent_path().empty())
    {
      fs::create_directories(path.parent_path());
    }
    // fs::create_directory functions as arbiter in case of racing conditions.
    if (fs::create_directory(path))
    {
      touch(path / "mark");
      return true;
    }
    return false;
  }

  std::vector<Cone> LinearConeStorage::unmarkCones(size_t n)
  {
    if (!fs::exists(storageDirectory))
    {
      return std::vector<Cone>();
    }

    std::vector<fs::path> markFiles;
    std::vector<Cone> cones;
    for (fs::recursive_directory_iterator file(storageDirectory), end;
         file != end && markFiles.size() < n;
         ++file)
    {
      auto& p = file->path();
      if (fs::is_regular(p))
      {
        markFiles.push_back(p);
        cones.push_back(getCone(p.parent_path()));
      }
    }

    for (const fs::path& file : markFiles)
    {
      fs::remove(file);
    }
    return cones;
  }

  bool LinearConeStorage::saveConesPersistently
    (const boost::filesystem::path& path)
  {
    if (!path.parent_path().empty())
    {
      fs::create_directories(path.parent_path());
    }
    fs::ofstream o(path);
    if (!o) { return false; }
    if (!fs::exists(storageDirectory)) { return true; }

    bool firstEntry = true;

    for (fs::recursive_directory_iterator file(storageDirectory), end;
         file != end;
         ++file)
    {
      auto& p = file->path();
      if (fs::is_regular_file(p))
      {
        if (firstEntry) { firstEntry = false; }
        else            { o << '\n'; }

        o << getCone(p.parent_path());
      }
      else if (fs::is_directory(p) && fs::is_empty(p))
      {
        if (firstEntry) { firstEntry = false; }
        else            { o << '\n'; }

        o << getCone(p);
      }
    }
    return true;
  }
}
}
}
