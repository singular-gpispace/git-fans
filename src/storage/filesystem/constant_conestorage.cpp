#include <storage/filesystem/constant_conestorage.hpp>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <logging/logging.hpp>

#include <vector>
#include <stdexcept>

#define BOUNDARY 1000

namespace fs = boost::filesystem;

namespace gitfan {
namespace storage {
namespace filesystem
{
  ConstantConeStorage::ConstantConeStorage
    (const Options& options)
  : _all(options.workingDirectory / "all", options.subdirectoryNameLength)
  , _marked(options.workingDirectory / "marked", options.subdirectoryNameLength)
  { }

  bool ConstantConeStorage::addMarkedCone(const Cone& cone)
  {
    //
    for (int i = 0; i <= BOUNDARY; i++)
    {
      try
      {
        //
        if (_all.add(cone))
        {
          if (!_marked.add(cone))
          {
            throw std::logic_error
              ("Inconsistent ConstantUnmarkFileStorage. "
               "_marked is supposed to be a subset of _all, thus a successfull "
               "insertion in _all should guarantee an insertion in _marked");
          }
          return true;
        }
        return false;
        //
        break;
      }
      catch (const std::exception& e)
      {
        auto& log = logging::Log::instance();
        log << "Error at " << i+1 << "-th try of addMarkedCone(" << cone
            << "): " << e.what() << "\n";
        log.counter("addMarkedConeFails")++;
        if (i >= BOUNDARY) { throw; }
      }
    }
    //
    return false;
  }

  std::vector<Cone> ConstantConeStorage::unmarkCones(size_t n)
  {
    std::vector<Cone> cones;
    for ( auto cone = _marked.begin()
        ; cone != _marked.end() && cones.size() < n
        ; ++cone)
    {
      cones.push_back(*cone);
    }
    for (const auto& cone : cones)
    {
      //
      for (int i = 0; i <= BOUNDARY; i++)
      {
        try
        {
          //
          _marked.remove(cone);
          //
          break;
        }
        catch (const std::exception& e)
        {
          auto& log = logging::Log::instance();
          log << "Error at " << i+1 << "-th try of _marked.remove(" << cone
              << "): " << e.what() << "\n";
          log.counter("markedRemoveFails")++;
          if (i >= BOUNDARY) { throw; }
        }
      }
      //
    }
    return cones;
  }

  bool ConstantConeStorage::saveConesPersistently
    (const boost::filesystem::path& path)
  {
    if (!path.parent_path().empty())
    {
      fs::create_directories(path.parent_path());
    }
    fs::ofstream o(path);
    if (!o) { return false; }

    bool firstEntry = true;
    for (const auto& cone : _all)
    {
      if (firstEntry) { firstEntry = false; }
      else            { o << '\n'; }
      o << cone;
    }
    return true;
  }
}
}
}
