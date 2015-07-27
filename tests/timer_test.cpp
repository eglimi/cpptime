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
 * \file timer_test.cpp
 *
 * Tests for cpptime component. Compile with
 *
 * ~~~
 * g++ -g -std=c++11 -Wall -Wextra -o tests ../cpptime.cpp timer_test.cpp -l pthread
 * ~~~
 *
 */

#define CATCH_CONFIG_MAIN

// Includes
#include <thread>
#include <chrono>
#include "catch.hpp"
#include "../cpptime.h"

using namespace std::chrono;

TEST_CASE("Test start and stop.")
{
	{
		CppTime::Timer t;
	}
}

TEST_CASE("Tests with two argument add")
{
	CppTime::Timer t;

	SECTION("Test uint64_t timeout argument")
	{
		int i = 0;
		t.add(100000, [&](CppTime::timer_id) { i = 42; });
		std::this_thread::sleep_for(milliseconds(120));
		REQUIRE(i == 42);
	}

	SECTION("Test duration timeout argument")
	{
		int i = 0;
		t.add(milliseconds(100), [&](CppTime::timer_id) { i = 43; });
		std::this_thread::sleep_for(milliseconds(120));
		REQUIRE(i == 43);
	}

	SECTION("Test time_point timeout argument")
	{
		int i = 0;
		t.add(CppTime::clock::now() + milliseconds(100), [&](CppTime::timer_id) { i = 44; });
		std::this_thread::sleep_for(milliseconds(120));
		REQUIRE(i == 44);
	}
}

TEST_CASE("Tests with three argument add")
{
	CppTime::Timer t;

	SECTION("Test uint64_t timeout argument")
	{
		size_t count = 0;
		auto id = t.add(100000, [&](CppTime::timer_id) { ++count; }, 10000);
		std::this_thread::sleep_for(milliseconds(125));
		t.remove(id);
		REQUIRE(count == 3);
	}

	SECTION("Test duration timeout argument")
	{
		size_t count = 0;
		auto id =
		    t.add(milliseconds(100), [&](CppTime::timer_id) { ++count; }, microseconds(10000));
		std::this_thread::sleep_for(milliseconds(135));
		t.remove(id);
		REQUIRE(count == 4);
	}
}

TEST_CASE("Test delete timer in callback")
{
	CppTime::Timer t;

	SECTION("Delete one timer")
	{
		size_t count = 0;
		auto id = t.add(milliseconds(10), [&](CppTime::timer_id id) {
			++count;
			t.remove(id);
		}, milliseconds(10));
		std::this_thread::sleep_for(milliseconds(50));
		REQUIRE(count == 1);
	}

	SECTION("Ensure that the correct timer is freed and reused")
	{
		auto id1 = t.add(milliseconds(40), [](CppTime::timer_id) {});
		auto id2 = t.add(milliseconds(10), [&](CppTime::timer_id id) { t.remove(id); });
		std::this_thread::sleep_for(milliseconds(30));
		auto id3 = t.add(microseconds(100), [](CppTime::timer_id) {});
		auto id4 = t.add(microseconds(100), [](CppTime::timer_id) {});
		REQUIRE(id3 == id2);
		REQUIRE(id4 != id1);
		REQUIRE(id4 != id2);
		std::this_thread::sleep_for(milliseconds(20));
	}

	SECTION("Ensure that the correct timer is freed and reused - different ordering")
	{
		auto id1 = t.add(milliseconds(10), [&](CppTime::timer_id id) { t.remove(id); });
		auto id2 = t.add(milliseconds(40), [](CppTime::timer_id) {});
		std::this_thread::sleep_for(milliseconds(30));
		auto id3 = t.add(microseconds(100), [](CppTime::timer_id) {});
		auto id4 = t.add(microseconds(100), [](CppTime::timer_id) {});
		REQUIRE(id3 == id1);
		REQUIRE(id4 != id1);
		REQUIRE(id4 != id2);
		std::this_thread::sleep_for(milliseconds(20));
	}
}

TEST_CASE("Test two identical timeouts")
{
	int i = 0;
	int j = 0;
	CppTime::Timer t;
	CppTime::timestamp ts = CppTime::clock::now() + milliseconds(40);
	t.add(ts, [&](CppTime::timer_id) { i = 42; });
	t.add(ts, [&](CppTime::timer_id) { j = 43; });
	std::this_thread::sleep_for(milliseconds(50));
	REQUIRE(i == 42);
	REQUIRE(j == 43);
}

TEST_CASE("Test timeouts from the past.")
{
	CppTime::Timer t;

	SECTION("Test negative timeouts")
	{
		int i = 0;
		int j = 0;
		CppTime::timestamp ts1 = CppTime::clock::now() - milliseconds(10);
		CppTime::timestamp ts2 = CppTime::clock::now() - milliseconds(20);
		t.add(ts1, [&](CppTime::timer_id) { i = 42; });
		t.add(ts2, [&](CppTime::timer_id) { j = 43; });
		std::this_thread::sleep_for(microseconds(10));
		REQUIRE(i == 42);
		REQUIRE(j == 43);
	}

	SECTION("Test time overflow when blocking timer thread.")
	{
		int i = 0;
		CppTime::timestamp ts1 = CppTime::clock::now() + milliseconds(10);
		CppTime::timestamp ts2 = CppTime::clock::now() + milliseconds(20);
		t.add(ts1, [&](CppTime::timer_id) { std::this_thread::sleep_for(milliseconds(20)); });
		t.add(ts2, [&](CppTime::timer_id) { i = 42; });
		std::this_thread::sleep_for(milliseconds(50));
		REQUIRE(i == 42);
	}
}

TEST_CASE("Test order of multiple timeouts")
{
	int i = 0;
	CppTime::Timer t;
	t.add(10000, [&](CppTime::timer_id) { i = 42; });
	t.add(20000, [&](CppTime::timer_id) { i = 43; });
	t.add(30000, [&](CppTime::timer_id) { i = 44; });
	t.add(40000, [&](CppTime::timer_id) { i = 45; });
	std::this_thread::sleep_for(milliseconds(50));
	REQUIRE(i == 45);
}

TEST_CASE("Test with multiple timers")
{
	int i = 0;
	CppTime::Timer t1;
	CppTime::Timer t2;

	SECTION("Update the same value at different times with different timers")
	{
		t1.add(milliseconds(20), [&](CppTime::timer_id) { i = 42; });
		t1.add(milliseconds(40), [&](CppTime::timer_id) { i = 43; });
		std::this_thread::sleep_for(milliseconds(30));
		REQUIRE(i == 42);
		std::this_thread::sleep_for(milliseconds(20));
		REQUIRE(i == 43);
	}

	SECTION("Remove one timer without affecting the other")
	{
		auto id1 = t1.add(milliseconds(20), [&](CppTime::timer_id) { i = 42; });
		t1.add(milliseconds(40), [&](CppTime::timer_id) { i = 43; });
		std::this_thread::sleep_for(milliseconds(10));
		t1.remove(id1);
		std::this_thread::sleep_for(milliseconds(20));
		REQUIRE(i == 0);
		std::this_thread::sleep_for(milliseconds(20));
		REQUIRE(i == 43);
	}
}
