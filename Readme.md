![](https://raw.githubusercontent.com/Naios/awaitify/master/doc/preview.png)

**awaitify** offers simple C++14 **stackful await** functionality for **testing purposes (this example isn't ready for production!)**  using boost's asio, future and coroutine2 and is  implemented in less then 400 LOC.

awaitify is simple which makes it as easy to use **await** in C++ like in C# or Python.

## Requirements

- C++14 capable compiler (MSVC 2015+, Clang 3.6+, GCC 4.9+)
- boost > 1.59 (requires `boost::coroutine2`) with following link libraries:
  - boost system
  - boost context
  - boost coroutine
  - boost thread

# Examples

First of all include awaitify:
```c++
#include "awaitify/awaitify.hpp"
```

Start a new awaitable context through using:
```c++
awaitify([]
{
});
```

Wait for a future:
```c++
boost::future<int> sql_query(char const* sql);

awaitify([]
{
  // The value of the future is returned by the await expression
  int i = await sql_query("SELECT count(*) FROM users");
  // awaiting ready futures is possible
  return await boost::make_ready_future(true);
});
```

**BUT: Never use await outside an awaitified expression!**

**AGAIN: This library is only meant for educational/testing purposes, never use it in a productional environment!**

# License

```
Copyright 2015-2016 Denis Blank <denis.blank at outlook dot com>
   Distributed under the Boost Software License, Version 1.0
     (See accompanying file LICENSE_1_0.txt or copy at
           http://www.boost.org/LICENSE_1_0.txt)
```
