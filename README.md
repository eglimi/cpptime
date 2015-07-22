C++ Timeouts
============

A portable, header-only C++11 timer component.

It manages a set of timeouts that, when expired, invoke a callback. It uses
features of C++11 in order to avoid platform specific code.

It supports one-shot and periodic timeouts.

Documentation
-------------

Please see the documentation in [cpptime.h](./cpptime.h) for more detailed information about
the implementation.

Implementation Status
---------------------

This is a new component and not much testing has been done. We already use it
in some of our products but expect that it receives some updates over time.

Possible Features
----------------

While the current implementation serves us well, there are some features that
might potentially be interesting for other use cases. Contact us in case you
are interested.

- [x] Ability to have multiple timer components running.
- [x] Distribute it as a header only library.
- [ ] Optionally avoid locking.
- [ ] API to use client thread instead of creating its own.
- [ ] API to use client mutex instead of its own.

Use Cases and Limitations
-------------------------

Naturally the implementation makes some trade-offs. This makes it useful for
some cases, and less so for others.

- The timer runs completely in user space. This makes it slightly less efficient
than other solutions, such as `timer_create()` or `timerfd_create()`. However,
in many cases, this overhead is acceptable.

- Given a C++11 capable compiler, the code is portable.

- The API to add or remove a timeout is arguably nicer than the platform specific
alternatives. E.g.

~~~
timer.add(seconds(2), [](CppTime::timer_id) { ... });
~~~

- The implementation is small and easy to understand. It is not difficult to
change to make it better suitable for specific cases.

Examples
--------

A one shot timer.

~~~
using namespace std::chrono;
CppTime::Timer t;
t.add(seconds(2), [](CppTime::timer_id) { std::cout << "yes\n"; });
std::this_thread::sleep_for(seconds(3));
~~~

A periodic timer that is first executed after 2 seconds, and after this every
second. The event is removed after 10 seconds.

~~~
using namespace std::chrono;
CppTime::Timer t;
auto id = t.add(seconds(2), [](CppTime::timer_id) { std::cout << "yes\n"; }, seconds(1));
std::this_thread::sleep_for(seconds(10));
t.remove(id);
~~~

See the tests for more examples.


Usage
-----

To use the timer component, Simply copy [cpptime.h](./cpptime.h) into you
project. Everything is contained in this single header file.

Tests can be compiled and executed with the following commands, assuming you
are on a POSIX machine.

~~~
cd tests
clang++-3.6  -std=c++11 -Wall -Wextra -o tests timer_test.cpp -l pthread
./tests
~~~

Contributions
-------------

Contributions, suggestions, and feature requests are welcome. Please use the
Github issue tracker.
