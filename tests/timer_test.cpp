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

TEST_CASE("Test start and stop.")
{
	CppTime::start();
	CppTime::stop();
}

TEST_CASE("Tests with two argument add")
{
	CppTime::start();

	SECTION("Test uint64_t timeout argument")
	{
		int i = 0;
		CppTime::add(100000, [&](CppTime::timer_id) { i = 42; });
		std::this_thread::sleep_for(std::chrono::milliseconds(120));
		REQUIRE(i == 42);
	}

	SECTION("Test duration timeout argument")
	{
		int i = 0;
		CppTime::add(std::chrono::milliseconds(100), [&](CppTime::timer_id) { i = 43; });
		std::this_thread::sleep_for(std::chrono::milliseconds(120));
		REQUIRE(i == 43);
	}

	SECTION("Test time_point timeout argument")
	{
		int i = 0;
		CppTime::add(CppTime::clock::now() + std::chrono::milliseconds(100),
		    [&](CppTime::timer_id) { i = 44; });
		std::this_thread::sleep_for(std::chrono::milliseconds(120));
		REQUIRE(i == 44);
	}

	CppTime::stop();
}

TEST_CASE("Tests with three argument add")
{
	CppTime::start();

	SECTION("Test uint64_t timeout argument")
	{
		size_t count = 0;
		auto id = CppTime::add(100000, [&](CppTime::timer_id) { ++count; }, 10000);
		std::this_thread::sleep_for(std::chrono::milliseconds(125));
		CppTime::remove(id);
		REQUIRE(count == 3);
	}

	SECTION("Test duration timeout argument")
	{
		size_t count = 0;
		auto id = CppTime::add(std::chrono::milliseconds(100), [&](CppTime::timer_id) { ++count; },
		    std::chrono::microseconds(10000));
		std::this_thread::sleep_for(std::chrono::milliseconds(135));
		CppTime::remove(id);
		REQUIRE(count == 4);
	}

	CppTime::stop();
}

TEST_CASE("Test delete timer in callback")
{
	CppTime::start();

	size_t count = 0;
	auto id = CppTime::add(std::chrono::milliseconds(10), [&](CppTime::timer_id id) {
		++count;
		CppTime::remove(id);
	}, std::chrono::milliseconds(10));
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	REQUIRE(count == 1);

	CppTime::stop();
}
