
#include <atomic>

#include "awaitify/awaitify.hpp"

#define CATCH_CONFIG_RUNNER
#include "catch/catch.hpp"

using namespace awf;

static std::unique_ptr<boost::asio::io_service::work> work;

int main(int argc, char** argv)
{
  // Initialize work
  work = std::make_unique<boost::asio::io_service::work>(system_scheduler());

  int const result = Catch::Session().run(argc, argv);

  system_scheduler().run();

  // Attach breakpoint here ,-)
  return result;
}

TEST_CASE("Basic future and promise tests", "[future & promise]")
{
  SECTION("The future is available after the value was passed to the promise")
  {
    promise<bool> promise;
    auto future = promise.get_future();
    REQUIRE_FALSE(future.is_ready());
    promise.set_value(true);
    REQUIRE(future.is_ready());
    CHECK(future.get());
  }
}

TEST_CASE("Basic executor tests", "[executor]")
{
  SECTION("The executor supports `dispatch` dispatching")
  {
    promise<bool> promise;
    auto future = promise.get_future();
    system_scheduler().dispatch([&]
    {
      promise.set_value(true);
    });
    CHECK(future.get());
  }

  SECTION("The executor supports `post` dispatching")
  {
    promise<bool> promise;
    auto future = promise.get_future();
    system_scheduler().post([&]
    {
      promise.set_value(true);
    });
    CHECK(future.get());
  }
}
