
//  Copyright 2015-2016 Denis Blank <denis.blank at outlook dot com>
//     Distributed under the Boost Software License, Version 1.0
//       (See accompanying file LICENSE_1_0.txt or copy at
//             http://www.boost.org/LICENSE_1_0.txt)

#include "awaitify/awaitify.hpp"

namespace awf {
  #ifndef AWAITIFY_NO_SYSTEM_SCHEDULER
    executor& awf::system_scheduler()
    {
      static executor instance;
      return instance;
    }
  #endif // AWAITIFY_NO_SYSTEM_SCHEDULER

  shared_execution_context* awf::current_execution_context()
  {
    static thread_local shared_execution_context instance;
    return &instance;
  }

  void awf::execution_context::weak_enter()
  {
    assert(!(*current_execution_context()) &&
           "Context already in use!");
    (*current_execution_context()) = shared_from_this();
  }

  void awf::execution_context::resume()
  {
    weak_enter();
    (*pull_)();
    weak_leave();
  }

  void execution_context::weak_leave()
  {
    assert((*current_execution_context()) &&
           "No context in use!");
    current_execution_context()->reset();
  }

  void execution_context::suspend()
  {
    assert((*current_execution_context()) &&
            "Invalid context of execution.");

    (*push_)();
  }
}
