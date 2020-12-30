#include "util.hpp"

#include <boost/filesystem/fstream.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <util-generic/read_lines.hpp>

#include <pnetc/type/file/op.hpp>

#include <stdexcept>
#include <numeric>

namespace gitfan {
namespace testing {
namespace util
{
  namespace fs = boost::filesystem;

  bool ssiFilesExistWithIdenticalContent(const fs::path& p1, const fs::path& p2)
  {
    fs::ifstream ifs1(p1, fs::ifstream::in);
    fs::ifstream ifs2(p2, fs::ifstream::in);
    if (!ifs1 || !ifs2)
    {
      return false;
    }
    std::string line1;
    std::string line2;
    // Skip first line which encodes stuff such as the singular version, active ring etc.
    //std::getline(ifs1, line1);
    //std::getline(ifs2, line2);
    while (std::getline(ifs1, line1))
    {
      if (!std::getline(ifs2, line2))
      {
        return false;
      }
      if (line1 != line2)
      {
        return false;
      }
    }
    return !std::getline(ifs2, line2);
  }

  bool fileIsEmpty(const fs::path& pFile)
  {
    fs::ifstream ifs(pFile);
    return ifs.peek() == fs::ifstream::traits_type::eof();
  }

  void applyPermutationToHash(const permutation& sigma, pnet_list& hash)
  {
    std::transform(hash.begin(), hash.end(), hash.begin(),
                   [&sigma](pnet_value& hashEntry) -> int
      {
        // sigma is 0-indexed, elements in hash lists however
        // index the k-th orbit cone with k instead of k-1
        return static_cast<int>(sigma[as<int>(hashEntry) - 1]) + 1;
      });
    hash.sort();
  }

  void applyPermutationToGroupAction(
    const permutation& sigma,
    singular::gitfan::GroupAction& action)
  {
    permutation id(sigma.size());
    std::iota(id.begin(), id.end(), 0);
    permutation invSigma = computePermutation
      ( id
      , sigma
      , [](const unsigned int& l, const unsigned int& r) {return l == r;}
      );
    for (auto& element : action)
    {
      element = sigma * element * invSigma;
    }
  }

  pnet_list toPnetList(const std::vector<fs::path>& paths)
  {
    pnet_list result;
    for (const auto& path : paths)
    {
    result.emplace_back(
        pnetc::type::file::to_value(pnet_file(path.string())));
    }
    return result;
  }

  std::vector<fs::path> toFilePaths(const pnet_list& paths)
  {
    std::vector<fs::path> result;
    for (const auto& path : paths)
    {
      result.emplace_back(pnetc::type::file::from_value(path).path);
    }
    return result;
  }

  std::vector<fs::path> loadOrbitConeListFile(const fs::path& path)
  {
    std::vector<fs::path> result;
    for ( const std::string& file
        : fhg::util::read_lines(path)
        )
    {
      result.push_back(path.parent_path() / file);
    }
    return result;
  }

  void copyDirectory(
    boost::filesystem::path source,
    boost::filesystem::path dest)
  {
    for (fs::path destAncestor = dest;
         destAncestor != fs::path();
         destAncestor = destAncestor.parent_path())
    {
      if (destAncestor == source)
      {
        throw std::runtime_error("Destination is located in Source.");
      }
    }
    if (!fs::is_directory(source))
    {
        throw std::runtime_error("Source is no directory.");
    }
    if (fs::exists(dest))
    {
        throw std::runtime_error("Destination exists already.");
    }

    for (const auto& entry : fs::recursive_directory_iterator{source})
    {
      auto relativePathStr = entry.path().string();
      boost::replace_first(relativePathStr, source.string(), "");
      auto entryDest = dest / relativePathStr;
      fs::create_directories(entryDest.parent_path());
      fs::copy(entry.path(), entryDest);
    }
  }

}}}
