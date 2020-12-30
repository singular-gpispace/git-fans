#include <util-gitfan/filecontent_operations.hpp>

#include <util-generic/read_file.hpp>

namespace gitfan {
namespace filecontent
{
  namespace fs=boost::filesystem;

  std::string getFileContent
    ( const std::shared_ptr<Cache>&
        cachePtr
    , const fs::path& file
    )
  {
    static const auto factory =
      [](const fs::path& file) -> std::string
      {
        return fhg::util::read_file<>(file);
      };

    if (cachePtr)
    {
      return cachePtr->get(file, factory);
    }
    else
    {
      return factory(file);
    }
  }

  std::size_t Hash::operator()(const fs::path& file) const
  {
    return _hash(getFileContent(_cache, file));
  }

  bool EqualTo::operator()( const fs::path& lhs
                                    , const fs::path& rhs
                                    ) const
  {
    return getFileContent(_cache, lhs) == getFileContent(_cache, rhs);
  }

  bool Compare::operator()( const fs::path& lhs
                                    , const fs::path& rhs
                                    ) const
  {
    return getFileContent(_cache, lhs) < getFileContent(_cache, rhs);
  }
}
}
