
#include "awaitify/awaitify.hpp"

#include <atomic>
#include <thread>
#include <boost/thread.hpp>
#include <boost/asio/signal_set.hpp>

#define CATCH_CONFIG_RUNNER
#include "catch/catch.hpp"

#define MULTITHREADED

using namespace awf;

static std::unique_ptr<boost::asio::io_service::work> work;

template<typename T>
auto invoke(std::true_type /*void*/, T&& task)
{
  using result_t = decltype(std::forward<T>(task)());
  auto promise = std::make_shared<promise_t<result_t>>();
  system_scheduler().post([promise, task = std::forward<T>(task)]() mutable
  {
    std::forward<T>(task)();
    promise->set_value();
  });
  return promise->get_future();
}

template<typename T>
auto invoke(std::false_type /*non void*/, T&& task)
{
  using result_t = decltype(std::forward<T>(task)());
  auto promise = std::make_shared<promise_t<result_t>>();
  system_scheduler().post([promise, task = std::forward<T>(task)]() mutable
  {
    promise->set_value(std::forward<T>(task)());
  });
  return promise->get_future();
}

template<typename T>
auto invoke(T&& task)
{
  using result_t = decltype(std::forward<T>(task)());
  return invoke(std::integral_constant<bool,
    std::is_same<result_t, void>::value>{}, std::forward<T>(task));
}

int main(int argc, char** argv)
{
  // Initialize work
  work = std::make_unique<boost::asio::io_service::work>(system_scheduler());

  auto result = boost::async([=]
  {
    return Catch::Session().run(argc, argv);
  });

#ifdef MULTITHREADED
  boost::thread_group threads;
  for (auto i : { 1, 2, 3, 4 })
  {
    (void)i;
    threads.create_thread(
      boost::bind(&boost::asio::io_service::run, &system_scheduler()));
  }
#endif // MULTITHREADED

  system_scheduler().run();
  
#ifdef MULTITHREADED
  threads.join_all();
#endif // MULTITHREADED

  int value = result.get();

  // Attach breakpoint here ,-)
  return value;
}

TEST_CASE("Basic future_t and promise_t tests", "[future_t & promise_t]")
{
  SECTION("The future_t is available after the value was passed to the promise")
  {
    promise_t<bool> promise;
    auto future = promise.get_future();
    REQUIRE_FALSE(future.is_ready());
    promise.set_value(true);
    REQUIRE(future.is_ready());
    CHECK(future.get());
  }

  SECTION("Futures are chainable")
  {
    promise_t<int> promise;
    auto future = promise.get_future().then(
      [](future_t<int> future)
    {
      REQUIRE(future.is_ready());
      CHECK(future.get() == 100);
      return true;
    });
    REQUIRE_FALSE(future.is_ready());
    promise.set_value(100);
    CHECK(future.get());
  }
}

TEST_CASE("Basic executor tests", "[executor]")
{
  SECTION("The executor supports `dispatch` dispatching")
  {
    promise_t<bool> promise_t;
    auto future_t = promise_t.get_future();
    system_scheduler().dispatch([&]
    {
      promise_t.set_value(true);
    });
    CHECK(future_t.get());
  }

  SECTION("The executor supports `post` dispatching")
  {
    promise_t<bool> promise_t;
    auto future_t = promise_t.get_future();
    system_scheduler().post([&]
    {
      promise_t.set_value(true);
    });
    CHECK(future_t.get());
  }
}

TEST_CASE("Awaitify & await tests", "[awaitify & await]")
{
  SECTION("Empty awaitify expressions return the correct result")
  {
    auto future = awaitify([]
    {
      return true;
    });
    CHECK(future.get());
  }

  SECTION("Ready awaitify expressions return the correct result")
  {
    auto future = awaitify([]
    {
      return await boost::make_ready_future(true);
    });
    CHECK(future.get());
  }

  SECTION("Contexts are suspendable and work with void results")
  {
    auto future = awaitify([]
    {
      std::atomic<bool> invoked = false;
      await invoke([&]
      {
        invoked = true;
      });
      return invoked.load();
    });
    CHECK(future.get());
  }

  SECTION("Contexts are suspendable and work with future results")
  {
    auto future = awaitify([]
    {
      auto result = await invoke([]
      {
        return true;
      });
      CHECK(result);
      return result;
    });
    CHECK(future.get());
  }

  SECTION("Contexts are suspendable and work with compound future results")
  {
    auto future = awaitify([]
    {
      auto result1 = await invoke([]
      {
        return true;
      });
      auto result2 = await invoke([]
      {
        return true;
      });
      CHECK(result1);
      CHECK(result2);
      return result1 && result2;
    });
    CHECK(future.get());
  }
}

TEST_CASE("Executor suspender", "[executor]")
{
  SECTION("Stop the executor")
  {
    work.reset();
  }
}
