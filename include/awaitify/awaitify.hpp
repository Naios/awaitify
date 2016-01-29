
//  Copyright 2015-2016 Denis Blank <denis.blank at outlook dot com>
//     Distributed under the Boost Software License, Version 1.0
//       (See accompanying file LICENSE_1_0.txt or copy at
//             http://www.boost.org/LICENSE_1_0.txt)

#ifndef INCLUDED_AWAITIFY_HPP
#define INCLUDED_AWAITIFY_HPP

#include <memory>
#include <type_traits>
#include <boost/optional.hpp>
#include <boost/coroutine2/coroutine.hpp>

#if !defined(AWAITIFY_PROVIDE_FUTURE_TYPE) || \
    !defined(AWAITIFY_PROVIDE_PROMISED_TYPE)
  // Provide the boost future and promise implementation
  // when no custom type is used.
  #define BOOST_THREAD_PROVIDES_FUTURE
  #define BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
  #define BOOST_THREAD_PROVIDES_FUTURE_WHEN_ALL_WHEN_ANY
  #include <boost/thread/future.hpp>
#endif // AWAITIFY_PROVIDE_FUTURE_TYPE || AWAITIFY_PROVIDE_PROMISED_TYPE

#ifndef AWAITIFY_PROVIDE_EXECUTOR_TYPE
  #include <boost/asio/io_service.hpp>
#endif // AWAITIFY_PROVIDE_EXECUTOR_TYPE

// Define AWAITIFY_NO_KEYWORD_MACRO to prevent the creation
// of the keyword `await` macro.
// Declares `await( ... )` instead.
#ifndef AWAITIFY_NO_KEYWORD_MACRO
  #define await _awaiter_impl() <<
#else
  #define await( EXPR ) _awaitify_impl_( EXPR )
#endif // AWAITIFY_NO_KEYWORD_MACRO

namespace awf {
// Provide your own future type through
// defining AWAITIFY_PROVIDE_FUTURE_TYPE.
// The interface of the given type needs to match
// the one from boost::future.
#ifndef AWAITIFY_PROVIDE_FUTURE_TYPE
  /// \brief Future type from boost
  template<typename T>
  using future = boost::future<T>;
#else
  /// \brief Future provided from AWAITIFY_PROVIDE_PROMISE_TYPE
  template<typename T>
  using future = AWAITIFY_PROVIDE_FUTURE_TYPE <T>;
#endif // AWAITIFY_PROVIDE_FUTURE_TYPE

// Provide your own promise type through
// defining AWAITIFY_PROVIDE_PROMISE_TYPE.
// The interface of the given type needs to match
// the one from boost::promise.
#ifndef AWAITIFY_PROVIDE_PROMISE_TYPE
  /// \brief Promise type from boost
  template<typename T>
  using promise = boost::promise<T>;
#else
  /// \brief Promise provided from AWAITIFY_PROVIDE_PROMISE_TYPE
  template<typename T>
  using promise = AWAITIFY_PROVIDE_PROMISE_TYPE <T>;
#endif // AWAITIFY_PROVIDE_PROMISE_TYPE

// Provide your own executor type through
// defining AWAITIFY_PROVIDE_EXECUTOR_TYPE.
// The interface of the given type needs to match
// the one from boost::asio::io_service.
#ifndef AWAITIFY_PROVIDE_EXECUTOR_TYPE
  /// \brief Executor type from boost
  using executor = boost::asio::io_service;
#else
  /// \brief Executor provided from AWAITIFY_PROVIDE_EXECUTOR_TYPE
  using executor = AWAITIFY_PROVIDE_EXECUTOR_TYPE;
#endif // AWAITIFY_PROVIDE_EXECUTOR_TYPE

#ifndef AWAITIFY_NO_SYSTEM_SCHEDULER
  /// \brief System scheduler
  executor& system_scheduler();
#endif // AWAITIFY_NO_SYSTEM_SCHEDULER

  template<typename T>
  class specific_execution_context;

  class execution_context
    : public std::enable_shared_from_this<execution_context>
  {
    using coro_t = boost::coroutines2::coroutine<void>;

    boost::optional<coro_t::pull_type> pull_;
    coro_t::push_type* push_;

  public:
    execution_context() { }
    execution_context(execution_context const&) = delete;
    execution_context(execution_context&&) = delete;
    execution_context& operator= (execution_context const&) = delete;
    execution_context& operator= (execution_context&&) = delete;

    template<typename Result, typename Task>
    void set_task(Task&& task)
    {
      weak_enter();

      pull_ = coro_t::pull_type(
        [ task = std::forward<Task>(task),
          me = shared_from_this() ]
        (boost::coroutines2::coroutine<void>::push_type& push) mutable
      {
        me->push_ = &push;
        me->invoke(std::is_same<decltype(task()), void>{},
          std::move(task),
          &static_cast<specific_execution_context<Result>*>(
            me.get())->promise_);
      });
      weak_leave();
    }
    void resume();
    void weak_enter();
    void weak_leave();
    void suspend();

  private:
    template<typename Task, typename Promise>
    void invoke(std::true_type, Task&& task, Promise* promise)
    {
      std::forward<Task>(task)();
      promise->set_value();
    }
    template<typename Task, typename Promise>
    void invoke(std::false_type, Task&& task, Promise* promise)
    {
      promise->set_value(std::forward<Task>(task)());
    }
  };

  template<typename T>
  class specific_execution_context
    : public execution_context
  {
    friend class execution_context;

    promise<T> promise_;

  public:
    specific_execution_context() { }

    auto get_future() { return promise_.get_future(); }
  };

  using shared_execution_context = std::shared_ptr<execution_context>;

  shared_execution_context* current_execution_context();

  template<typename T>
  T _awaitify_impl_ (future<T>&& future_)
  {
      assert(future_.valid() &&
             "The given future is invalid!");

      // Return the future immediately if it's ready
      if (future_.is_ready())
        return future_.get();

      // Suspend the context
      auto f = future_.then(
        [coroutine = *current_execution_context()](future<T> future_)
      {
        system_scheduler().post([coroutine]
        {
          assert(coroutine &&
                 "Execution context is invalid!");
          printf("resuming\n");
          coroutine->resume();
        });
        return future_.get();
      });
      assert((*current_execution_context()) &&
             "Await isn't dispatched in a coroutine!" &&
             "Use `asyncify` to create an awaitable context!");
      (*current_execution_context())->suspend();
      return f.get();
  }

  struct _awaiter_impl
  {
    template<typename T>
    T operator<< (future<T>&& future) const
    {
      return _awaitify_impl_(std::move(future));
    }
  };

  template<typename T>
  auto awaitify(T&& task)
  {
    using Result = std::decay_t<decltype(task())>;

    auto context = std::make_shared<
      specific_execution_context<Result>>();

    auto future = context->get_future();
    system_scheduler().post([c = std::move(context),
                             t = std::forward<T>(task)] () mutable
    {
      c->set_task<Result>(std::forward<T>(t));
    });
    return future;
  }
} // namespace awf

// Declare AWAITIFY_HEADER_ONLY to make this library header only.
// The usage as header-only library isn't recommended since
// it could lead to duplicated static instances!
#ifdef AWAITIFY_HEADER_ONLY
  #include "awaitify.cpp"
#endif // AWAITIFY_HEADER_ONLY

#endif // INCLUDED_AWAITIFY_HPP
