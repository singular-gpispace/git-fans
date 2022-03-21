#include <gtest/gtest.h>

#include <storage/conestorage.hpp>
#include <storage/service_provider.hpp>

#include <util-generic/read_lines.hpp>


#include <util.hpp>
#include <temporaryUniquePath.hpp>

#include <memory>
#include <vector>
#include <string>


namespace gitfan {
namespace storage {
namespace testing
{
  namespace fs = boost::filesystem;

  class ConeStorageTest : public ::testing::TestWithParam<std::string>
  {
    public:
      std::unique_ptr<IConeStorage> storage;


      void SetUp() override
      {
        _path = std::make_unique
          <gitfan::testing::util::TemporaryUniquePath>
            ("test-storage-" + GetParam());

        Options opt;
        opt.implementationTag = GetParam();
        opt.workingDirectory = *_path;

        _serviceProvider = std::make_unique<ServiceProvider>(opt);
        storage = IConeStorage::build (opt.serialize());
      }

      void TearDown() override
      {
        _serviceProvider = nullptr;
        _path = nullptr;
      }

    private:
      std::unique_ptr<gitfan::testing::util::TemporaryUniquePath> _path;
      std::unique_ptr<ServiceProvider> _serviceProvider;
  };

  INSTANTIATE_TEST_CASE_P
    (
    , ConeStorageTest
    , ::testing::ValuesIn(IConeStorage::availableImplementations())
    );

  TEST_P(ConeStorageTest, conestorageCanBeInitialized)
  {
    // Do nothing here. It is enough to enforce the construction
    // (and deconstruction) of ConeStorageTest.
  }

  TEST_P(ConeStorageTest, insert100ConesAndRemoveIn5Steps)
  {
    std::vector<size_t> unmarkConesParameters { 0, 1, 5, 90, 10 };
    std::vector<size_t> expectedResultSizes   { 0, 1, 5, 90,  4 };

    std::list<Cone> cones;
    for (int i = 0; i < 100; i++)
    {
      cones.push_back("myEncodedCone_" + std::to_string(i));
    }
    for (const Cone& cone : cones)
    {
      storage->addMarkedCone(cone);
    }

    for (unsigned int opNr = 0; opNr < unmarkConesParameters.size(); opNr++)
    {
      std::vector<Cone> result =
        storage->unmarkCones(unmarkConesParameters[opNr]);
      EXPECT_EQ(expectedResultSizes[opNr], result.size());

      for (const Cone& unmarkedCone : result)
      {
        const auto pos = std::find(cones.begin(), cones.end(), unmarkedCone);
        EXPECT_NE(pos, cones.end())
          << "Returned cone has not been inserted previsously";
        if (pos != cones.end())
        {
          cones.erase(pos);
        }
      }
    }
    EXPECT_EQ(0u, cones.size())
      << "Not all inserted cones have been returned";
  }

  TEST_P(ConeStorageTest, conestorageDetectsAndDropsDuplicates)
  {
    const Cone cone = "myEncodedDuplicateCone";
    storage->addMarkedCone(cone);
    storage->addMarkedCone(cone);
    {
      std::vector<Cone> result = storage->unmarkCones(1);
      EXPECT_EQ((std::vector<Cone>{ cone }), result);
    }
    {
      std::vector<Cone> result = storage->unmarkCones(1);
      EXPECT_EQ((std::vector<Cone>{}), result);
    }
  }

  TEST_P(ConeStorageTest, canWriteConesToFile)
  {
    const Cone cone1 = "myEncodedCone1";
    const Cone cone2 = "myEncodedCone2";
    storage->addMarkedCone(cone1);
    storage->addMarkedCone(cone2);
    std::vector<Cone> result = storage->unmarkCones(1);

    gitfan::testing::util::TemporaryUniquePath outputFile
      ("test-output-" + GetParam());
    storage->saveConesPersistently(outputFile);

    const std::vector<std::string>& actualOutput =
      fhg::util::read_lines(outputFile);
    const std::set<std::string> actualOutputSet
      (actualOutput.begin(), actualOutput.end());
    const std::set<std::string> expectedOutputSet
      { "myEncodedCone1"
      , "myEncodedCone2"
      };

    EXPECT_EQ(expectedOutputSet, actualOutputSet);
  }
}
}
}
