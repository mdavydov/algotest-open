/*  The Sysutils library for Threads/Sockets/PG Database/RPC
    Copyright (C) 2007 Maksim Davydov (http://www.adva-soft.com)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; Version 3

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; If not, see <http://www.gnu.org/licenses/>.
   
   
*/

#ifndef __SYSTEM_UTILS_THREADS_INCLUDED__
#define __SYSTEM_UTILS_THREADS_INCLUDED__

#pragma clang diagnostic error "-Wthread-safety"

#include <thread>
#include <vector>
#include <mutex>
#include <functional>
#include <cassert>
#include "algotest_c.h"

namespace sysutils
{
	void wait_ms(int ms);
    std::string getStackTrace();
    bool isMainThread();
	int getOptimalParallelThreads();
    
    void performOnMainThreadAndWait(std::function<void()> f);
    void performOnMainThreadAsync(std::function<void()> f);
    void setThreadName(const char * utf8name);

	enum { KNumThreadsAuto = 0 };

	template<class Fn>
	void runForThreads(int num_parts, int beg, int end, Fn&& Fx)
	{
		std::vector<std::thread> threads;
        int num_max = getOptimalParallelThreads();
		if (num_parts == KNumThreadsAuto) num_parts = num_max;
        if (num_parts > num_max) num_parts = num_max;
		if (num_parts <= 1)
		{
			Fx(beg, end);
			return;
		}

		for (int i = 0; i < num_parts; ++i)
		{
			int begi = beg + (end - beg) *i / num_parts;
			int endi = beg + (end - beg) *(i + 1) / num_parts;
            if (i!=num_parts-1)
            {
                threads.push_back(std::thread([begi, endi, &Fx](){Fx(begi, endi); }));
            }
            else
            {
                Fx(begi, endi);
            }
		}

		for (auto& thread : threads) thread.join();
	}
}

#ifndef _LIBCPP_ENABLE_THREAD_SAFETY_ANNOTATIONS
#error "No _LIBCPP_ENABLE_THREAD_SAFETY_ANNOTATIONS definition"
#endif

#ifndef _LIBCPP_HAS_THREAD_SAFETY_ANNOTATIONS
#error "No _LIBCPP_HAS_THREAD_SAFETY_ANNOTATIONS definition"
#endif

#define THREAD_ANNOTATION_ATTRIBUTE__(x)   __attribute__((x))

#define GUARDED_BY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(guarded_by(x))

#define REQUIRES(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(requires_capability(__VA_ARGS__))

#define EXCLUDES(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(locks_excluded(__VA_ARGS__))

class _LIBCPP_THREAD_SAFETY_ANNOTATION(capability("mutex")) main_thread_checker
{
public:
    void lock() _LIBCPP_THREAD_SAFETY_ANNOTATION(acquire_capability()) {
#ifdef DEBUG
        ASSERT(sysutils::isMainThread());
#endif
    }
    void unlock() _NOEXCEPT _LIBCPP_THREAD_SAFETY_ANNOTATION(release_capability()) {}
};

class _LIBCPP_THREAD_SAFETY_ANNOTATION(capability("mutex")) background_thread_checker
{
public:
    void lock() _LIBCPP_THREAD_SAFETY_ANNOTATION(acquire_capability()) {
#ifdef DEBUG
        ASSERT(!sysutils::isMainThread());
#endif
    }
    void unlock() _NOEXCEPT _LIBCPP_THREAD_SAFETY_ANNOTATION(release_capability()) {}
};

inline main_thread_checker g_assert_main_thread;
inline background_thread_checker g_assert_background_thread;

#define MAIN_THREAD REQUIRES(g_assert_main_thread) EXCLUDES(g_assert_background_thread)
#define BACKGROUND_THREAD REQUIRES(g_assert_background_thread) EXCLUDES(g_assert_main_thread)
#define ANY_THREAD

#define ASSERT_MAIN_THREAD() SYNC(g_assert_main_thread)
#define ASSERT_BACKGROUND_THREAD() SYNC(g_assert_background_thread)

#define SYNC(a) std::lock_guard<decltype(a)> sync_obj##__LINE__(a)
#define PARALLEL_FOR_N(n_threads, from, to, local_var_name) \
		sysutils::runForThreads( n_threads, (from), (to), [&](int local_var_name##parallel_for_beg, int local_var_name##parallel_for_end) \
			  			  { for (int local_var_name = local_var_name##parallel_for_beg; local_var_name < local_var_name##parallel_for_end; ++local_var_name) \

#define PARALLEL_FOR(from, to, local_var_name)  PARALLEL_FOR_N(sysutils::KNumThreadsAuto, from, to, local_var_name)
#define PARALLEL_FOR1(from, to, local_var_name) PARALLEL_FOR_N(1, from, to, local_var_name)
#define PARALLEL_FOR2(from, to, local_var_name) PARALLEL_FOR_N(2, from, to, local_var_name)
#define PARALLEL_FOR3(from, to, local_var_name) PARALLEL_FOR_N(3, from, to, local_var_name)
#define PARALLEL_FOR4(from, to, local_var_name) PARALLEL_FOR_N(4, from, to, local_var_name)

#define PARALLEL_END });

#endif // __SYSTEM_UTILS_INCLUDED__
