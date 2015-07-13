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
 * \file cpptime.cpp
 *
 * See the description in the header file for for more information.
 *
 */

// Includes
#include <thread>
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <stack>
#include <set>
#include <queue>
#include <unordered_map>
#include "cpptime.h"

namespace
{
using scoped_m = std::unique_lock<std::mutex>;

// Use to terminate the timer thread.
bool done = false;

// Locking & Co
std::mutex m;
std::condition_variable cond;
std::thread worker;

// The event structure that holds the information about a timer.
struct Event {
	CppTime::timer_id id;
	CppTime::timestamp start;
	CppTime::duration period;
	CppTime::handler_t handler;
	bool valid;
	Event()
	    : id(0),
	      start(CppTime::duration::zero()),
	      period(CppTime::duration::zero()),
	      handler(nullptr),
	      valid(false)
	{
	}
	template <typename Func>
	Event(CppTime::timer_id id, CppTime::timestamp start, CppTime::duration period, Func &&handler)
	    : id(id), start(start), period(period), handler(std::forward<Func>(handler)), valid(true)
	{
	}
	Event(Event &&r) noexcept : id(r.id),
	                            start(r.start),
	                            period(r.period),
	                            handler(std::move(r.handler)),
	                            valid(r.valid)
	{
	}
	Event &operator=(Event &&ev) noexcept
	{
		if(this != &ev) {
			id = ev.id;
			start = ev.start;
			period = ev.period;
			handler = std::move(ev.handler);
			valid = ev.valid;
		}
		return *this;
	}
	Event(const Event &r) = delete;
	Event &operator=(const Event &r) = delete;
};

// A time event structure that holds the next timeout and a reference to its
// Event struct.
struct Time_event {
	CppTime::timestamp next;
	CppTime::timer_id ref;
};

inline bool operator<(const Time_event &l, const Time_event &r)
{
	return r.next < l.next;
}

// The vector that holds all active events.
std::vector<Event> events;
// Sorted queue that has the next timeout at its top.
std::priority_queue<Time_event> time_events;

// A list of ids to be re-used. If possible, ids are used from this pool.
std::stack<CppTime::timer_id> free_ids;

// The thread main entry points. This is an endless loop until the `done` flag
// is set to false.
// TODO cleanup code to make it more readable (less if-else blocks).
void run()
{
	scoped_m lock(m);

	while(!done) {

		if(time_events.empty()) {
			// Wait for work
			cond.wait(lock);
		} else {
			auto te = time_events.top();
			if(CppTime::clock::now() >= te.next) {

				// Remove time event
				time_events.pop();
				Event &ev = events[te.ref];

				if(!ev.valid) {
					// Return the id if the event is no longer valid.
					free_ids.push(te.ref);
				} else {

					// Invoke the handler
					lock.unlock();
					ev.handler(te.ref);
					lock.lock();

					if(!ev.valid) {
						// The callback removed the event.
						free_ids.push(te.ref);
					} else {
						if(ev.period.count() > 0) {
							te.next += ev.period;
							time_events.push(te);
						} else {
							ev.valid = false;
							free_ids.push(te.ref);
						}
					}
				}
			} else {
				cond.wait_until(lock, te.next);
			}
		}
	}
}

} // end anonymous namespace

namespace CppTime
{

void start(size_t expected)
{
	scoped_m lock(m);
	done = false;
	if(expected > 0) {
		events.resize(expected);
	}
	worker = std::thread(&run);
}

void stop()
{
	scoped_m lock(m);
	done = true;
	events.clear();
	while(!time_events.empty()) {
		time_events.pop();
	}
	while(!free_ids.empty()) {
		free_ids.pop();
	}
	cond.notify_all();
	lock.unlock();
	worker.join();
}

timer_id add(const timestamp &when, handler_t &&handler, const duration &period)
{
	scoped_m lock(m);
	timer_id id = 0;
	// Add a new event. Prefer an existing and free id. If none is available, add
	// a new one.
	if(free_ids.empty()) {
		id = events.size();
		Event e(id, when, period, std::move(handler));
		events.push_back(std::move(e));
	} else {
		id = free_ids.top();
		free_ids.pop();
		Event e(id, when, period, std::move(handler));
		events[id] = std::move(e);
	}
	time_events.push(Time_event{when, id});
	cond.notify_all();
	return id;
}

bool remove(timer_id id)
{
	scoped_m lock(m);
	if(events.size() < id) {
		return false;
	}
	events[id].valid = false;
	return true;
}

} // end namespace CppTime
