#ifndef CPPTIME_H_
#define CPPTIME_H_

/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Michael Egli
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * \author    Michael Egli
 * \copyright Michael Egli
 * \date      11-Jul-2015
 *
 * \file cpptime.h
 *
 * C++11 timer component
 * =====================
 *
 * A portable, pure C++11 timer component.
 *
 * Overview
 * --------
 *
 * This component can be used to manage a set of timeouts. It is implemented in
 * pure C++11. It is therefore very portable given a compliant compiler.
 *
 * A timeout can be added with one of the `add` functions, and removed with the
 * `remove` function. A timeout can be either one-shot, or periodic. In case a
 * timeout is one-shot, the callback is invoked and the timeout event removed.
 *
 * Removing a timeout is possible even from within the callback.
 *
 * Timeout Units
 * -------------
 *
 * The preferred functions for adding timeouts are those that take a
 * `std::chrono::...` argument. However, for convenience, there is also an API
 * that takes a uint64_t. There, all values are expected to be given in
 * microseconds (us).
 *
 * Data Structure
 * --------------
 *
 * Internally, a std::vector is used to store timeout events. The timer_id
 * returned from the `add` functions are used as index to this vector.
 *
 * In addition, a std::multiset is used that holds all time points when
 * timeouts expire.
 *
 * Using a vector to store timeout events has some implications. It is very
 * fast to remove an event, because the timer_id is the vector's index. On the
 * other hand, this makes it also more complicated to manage the timer_ids. The
 * current solution is to keep track of used ids and re-use them if a new timer
 * is added.
 *
 * Examples
 * --------
 *
 * More examples can be found in the `tests` folder.
 *
 * ~~~
 * CppTime::start();
 * CppTime::add(std::chrono::seconds(1), [](CppTime::timer_id){ std::cout << "got it!"; });
 * std::this_thread::sleep_for(std::chrono::seconds(2));
 * CppTime::stop();
 * ~~~
 */
#include <functional>
#include <chrono>

namespace CppTime
{

// Public definitions
using timer_id = std::size_t;
using handler_t = std::function<void(timer_id)>;
using clock = std::chrono::steady_clock;
using timestamp = std::chrono::time_point<clock>;
using duration = std::chrono::microseconds;

/**
 * Start the timer thread.
 */
void start(size_t expected = 0);

/**
 * Stop the timer thread and free all internal resources.
 */
void stop();

/**
 * Add a new timer.
 *
 * \param when The time at which the handler is invoked.
 * \param handler The callable that is invoked when the timer fires.
 * \param period The periodicity at which the timer fires. Only used for periodic timers.
 */
timer_id add(const timestamp& when, handler_t&& handler, const duration& period = duration::zero());

/**
 * Overloaded `add` function that uses a `std::chrono::duration` instead of a
 * `time_point` for the first timeout.
 */
template <class Rep, class Period>
inline timer_id add(const std::chrono::duration<Rep, Period>& when, handler_t&& handler,
    const duration& period = duration::zero())
{
	return add(clock::now() + std::chrono::duration_cast<std::chrono::microseconds>(when),
	    std::move(handler), period);
}

/**
 * Overloaded `add` function that uses a uint64_t instead of a `time_point` for
 * the first timeout and the period.
 */
inline timer_id add(const uint64_t when, handler_t&& handler, const uint64_t period = 0)
{
	return add(duration(when), std::move(handler), duration(period));
}

/**
 * Removes the timer with the given id.
 */
bool remove(timer_id id);

} // end namespace CppTime

#endif // CPPTIME_H_
