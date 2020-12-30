#include <gtest/gtest.h>

#include <util-gitfan/singular_commands.hpp>

#include <util-generic/syscall.hpp>

#include <util.hpp>
#include <temporaryUniquePath.hpp>

using namespace gitfan::testing;
namespace fs = boost::filesystem;

namespace singular {
namespace testing
{
  TEST(SingularCommandsTest, discardsInterpreterOutput)
  {
    // Redirect stdout to file
    util::TemporaryUniquePath tmpOutput("stdout");
    ASSERT_NE(nullptr, freopen(tmpOutput.c_str(), "w", stdout))
      << "errno: " << errno;
    int stdout_fd = fhg::util::syscall::dup(STDOUT_FILENO);

    EXPECT_TRUE(init());
    call_and_discard("intvec i = 1,2,3;i;");

    // Reset stdout
    ASSERT_NE(EOF, fclose(stdout)) << errno;
    fhg::util::syscall::dup(stdout_fd, STDOUT_FILENO);
    ASSERT_NE(nullptr, stdout = fdopen(STDOUT_FILENO, "w"))
      << "errno: " << errno;
    fhg::util::syscall::close(stdout_fd);

    EXPECT_PRED1(&util::fileIsEmpty, (fs::path)tmpOutput);
  }
}}
