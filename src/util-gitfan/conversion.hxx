#include <util-generic/divru.hpp>
#include <util-gitfan/singular_commands.hpp>

#include <boost/filesystem/fstream.hpp>
#include <boost/range/adaptor/transformed.hpp>

#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <climits>

namespace gitfan {
namespace conversion
{
  template <typename T>
  std::vector<T> unwrap(const pnet_list& values)
  {
    std::vector<T> lv;
    for (const auto & value  : values
        | boost::adaptors::transformed(&as<T>)
        )
    {
      lv.push_back(value);
    }
    return lv;
  }

  template <typename T>
  pnet_list wrap(const std::vector<T>& values)
  {
    pnet_list lv;
    for (const T& value : values)
    {
      lv.push_back(value);
    }
    return lv;
  }

  inline intvec* createIntvec(const std::vector<int>& vector)
  {
    intvec* iv = new intvec(vector.size());
    int i = 0;
    for (const int& value : vector)
    {
      (*iv)[i++] = value;
    }
    return iv;
  }

  inline intvec* createIntvec(const pnet_list& list)
  {
    intvec* iv = new intvec(list.size());
    int i = 0;
    for (int val : list
        | boost::adaptors::transformed (&as<int>)
        )
    {
      (*iv)[i++] = val;
    }
    return iv;
  }

  inline lists toSingularList
    ( const pnet_list& list
    , bool lastLayerAsIntvec
    )
  {
    singular::init();
    lists result = static_cast<lists>(omAllocBin (slists_bin));
    result->Init(list.size());
    int currentIndex = 0;
    for (const pnet_value& val : list)
    {
      if(val.type() == typeid(int))
      {
        result->m[currentIndex].rtyp = INT_CMD;
        result->m[currentIndex].data = reinterpret_cast<void*>(as<int>(val));
      }
      else if(val.type() == typeid(pnet_list))
      {
        const pnet_list& typedVal = as<pnet_list>(val);
        bool asIntvec = lastLayerAsIntvec &&
          std::all_of(typedVal.cbegin(), typedVal.cend(),
            [](const pnet_value& val)
            {
              return typeid(int) == val.type();
            });
        if (asIntvec)
        {
          intvec* iv = createIntvec(typedVal);
          result->m[currentIndex].rtyp = INTVEC_CMD;
          result->m[currentIndex].data = static_cast<void*>(iv);
        }
        else
        {
          lists subList = toSingularList(typedVal, lastLayerAsIntvec);
          result->m[currentIndex].rtyp = LIST_CMD;
          result->m[currentIndex].data = static_cast<void*>(subList);
        }
      }
      else
      {
        throw std::runtime_error("Converting other types than int and "
          "pnet_list to singular lists is not supported.");
      }
      currentIndex++;
    }
    return result;
  }

  template <class OutputIt>
  inline void insertSingularObj(const sleftv& obj, OutputIt dest)
  {
    switch (obj.rtyp)
    {
      case INT_CMD:
      {
        int val = static_cast<int>(reinterpret_cast<long>(obj.data));
        *dest = val;
        break;
      }
      /*case STRING_CMD: // untested
      {
        char* cstr = static_cast<char*>(obj.data);
        list.push_back(std::string(cstr));
      }*/
      case INTVEC_CMD:
      {
        intvec* iv = static_cast<intvec*>(obj.data);
        *dest = toList(iv);
        break;
      }
      case LIST_CMD:
      {
        lists singularList = static_cast<lists>(obj.data);
        *dest = toList(singularList);
        break;
      }
      default:
      {
        throw std::runtime_error("Converting object of singular RType "
          + std::to_string(obj.rtyp) + " is not supported.");
      }
    }
    ++dest;
  }

  inline pnet_list toList(lists singularList)
  {
    pnet_list list;
    insert(singularList, std::back_inserter(list));
    return list;
  }

  inline pnet_list toList(const intvec* const iv)
  {
    pnet_list list;
    insert(iv, std::back_inserter(list));
    return list;
  }

  template <class OutputIt>
  void insert(lists singularList, OutputIt dest)
  {
    // lists->nr returns the size of the list - - 1
    for (int i = 0; i < singularList->nr + 1; i++)
    {
      insertSingularObj(singularList->m[i], dest);
    }
  }

  template <class OutputIt>
  void insert(const intvec* const iv, OutputIt dest)
  {
    for (int i = 0; i < iv->length(); i++)
    {
      *dest = (*iv)[i];
      ++dest;
    }
  }

  // Precondition: list contains only ints in ascending order
  inline std::string listToHexString(const pnet_list& list, int maxValue)
  {
    if (!list.empty() && boost::get<int>(list.back()) > maxValue)
    {
      int lastElem = boost::get<int>(list.back());
      if (lastElem > maxValue)
      {
        throw std::invalid_argument("Last element of the list ("
          + std::to_string(lastElem) + ") exceeds maxValue ("
          + std::to_string(maxValue) + ").");
      }
    }

    int number_of_blocks = fhg::util::divru(maxValue, 4);
    auto it = list.crbegin();
    std::stringstream str;
    for (int blockNumber = number_of_blocks; blockNumber > 0; blockNumber--)
    {
      int blockValue = 0;
      while (it != list.crend())
      {
        int listValue = boost::get<int>(*it);
        if (listValue <= (blockNumber-1)*4)
        {
          break;
        }
        switch (listValue - (blockNumber-1)*4)
        {
          case 1:
            blockValue += 1;
            break;
          case 2:
            blockValue += 2;
            break;
          case 3:
            blockValue += 4;
            break;
          case 4:
            blockValue += 8;
            break;
          default:
            throw std::runtime_error("This should never happen.");
        }
        ++it;
      }
      str << std::hex << blockValue;
    }
    return str.str();
  }

  inline pnet_list hexStringToList(const std::string& hexString)
  {
    pnet_list lv;
    int base = 0;
    for (auto it = hexString.crbegin(); it != hexString.crend(); ++it)
    {
      switch (::tolower(*it))
      {
        case 'f': // 1111
          lv.push_back(base + 1);
        case 'e': // 1110
          lv.push_back(base + 2);
        case 'c': // 1100
          lv.push_back(base + 3);
        case '8': // 1000
          lv.push_back(base + 4);
          break;
        case 'd': // 1101
          lv.push_back(base + 1);
          lv.push_back(base + 3);
          lv.push_back(base + 4);
          break;
        case 'b': // 1011
          lv.push_back(base + 1);
        case 'a': // 1010
          lv.push_back(base + 2);
          lv.push_back(base + 4);
          break;
        case '7': // 0111
          lv.push_back(base + 1);
        case '6': // 0110
          lv.push_back(base + 2);
        case '4': // 0100
          lv.push_back(base + 3);
          break;
        case '9': // 1001
          lv.push_back(base + 1);
          lv.push_back(base + 4);
          break;
        case '5': // 0101
          lv.push_back(base + 1);
          lv.push_back(base + 3);
          break;
        case '3': // 0011
          lv.push_back(base + 1);
        case '2': // 0010
          lv.push_back(base + 2);
          break;
        case '1': // 0001
          lv.push_back(base + 1);
          break;
        case '0':
          break;
        default:
          throw std::invalid_argument("Error when converting \""
            + hexString + "\": No hex string.");
      }
      base += 4;
    }
    return lv;
  }

  inline pnet_list loadListPartially
    ( const boost::filesystem::path& file
    , std::ios::pos_type pos
    , unsigned long linesToRead
    )
  {
    boost::filesystem::ifstream ifs
      ( file
      , boost::filesystem::ifstream::in | boost::filesystem::ifstream::binary
      );
    if (!ifs)
    {
      throw std::runtime_error("Could not open file " + file.string());
    }
    ifs.seekg(pos);

    pnet_list listFromFile;
    unsigned long readLines = 0;
    std::string line;
    while (readLines < linesToRead && std::getline(ifs, line))
    {
      pnet_list listFromLine;
      std::stringstream lineStream(line);
      std::string number;
      while (std::getline(lineStream, number, ','))
      {
        listFromLine.push_back(std::stoi(number));
      }
      listFromFile.push_back(listFromLine);
      readLines++;
    }
    return listFromFile;
  }

  inline pnet_list loadList_TESTING_ONLY(const boost::filesystem::path& file)
  {
    return loadListPartially(file, 0, ULONG_MAX);
  }
}}
