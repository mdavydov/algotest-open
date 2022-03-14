/*  The Algotest library for cross-platform testing of algorithms
 Copyright (C) 2014-2015 Maksym Davydov, ADVA Soft
 
 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; version 3
 
 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public
 License along with this library; If not, see <http://www.gnu.org/licenses/>.
 
 */


#ifndef algotest_time_included
#define algotest_time_included

#include <string>

namespace algotest
{
	class Timer
	{
	public:
        static double getTimeInSeconds();
        static void notifyTime(const char * action);
        inline static void notifyTime(const std::string& action) { notifyTime(action.c_str()); }
        //
    private:
        // use START_TIMER("Name", size) macro for starting local timer
        static void start(const std::string& algo_name, long num_pixels, bool active = true);
        static void finish();
        friend class TimerStarter;
        friend class SkipTimerStarter;
	};

	class FunctionProfiler
	{
		const char * m_name;
		double m_start_time;
	public:
		FunctionProfiler(const char * f_name);
		~FunctionProfiler();
	};
    
    class TimerStarter
    {
    public:
        TimerStarter(const std::string& algo_name, long num_pixels=0)
        {
            Timer::start(algo_name, num_pixels, true);
        }
        ~TimerStarter()
        {
            Timer::finish();
        }
    };
    
    class SkipTimerStarter
    {
    public:
        SkipTimerStarter(const std::string& algo_name, long num_pixels=0)
        {
            Timer::start(algo_name, num_pixels, false);
        }
        ~SkipTimerStarter()
        {
            Timer::finish();
        }
    };
    
    class TimeCriticalStarter
    {
    public:
        TimeCriticalStarter();
        ~TimeCriticalStarter();
        static bool isTimeCriticalInAction();
    };

#ifndef DISTRIBUTE
    
#define ALGOTEST_PROFILE() algotest::FunctionProfiler algotest_f_profiler(__func__);
#define ALGOTEST_PROFILE_BLOCK(name) algotest::FunctionProfiler algotest_f_profiler(name);
    
#define TIME_CRITICAL_BLOCK() algotest::TimeCriticalStarter algotest_time_critical##__LINE__;
#define IS_TIME_CRITICAL_IN_ACTION() algotest::TimeCriticalStarter::isTimeCriticalInAction()
    
    
#define START_TIMER(...) algotest::TimerStarter algotest_timer##__LINE__(__VA_ARGS__)
#define SKIP_TIMER(...) algotest::SkipTimerStarter algotest_timer##__LINE__(__VA_ARGS__)
#define NOTIFY_TIME(action) algotest::Timer::notifyTime(action)
    
#else // DISTRIBUTE
    
#define ALGOTEST_PROFILE()
#define ALGOTEST_PROFILE_BLOCK(name)
    
#define TIME_CRITICAL_BLOCK()
#define IS_TIME_CRITICAL_IN_ACTION()
    
#define START_TIMER(name, size_in_pixels)
#define NOTIFY_TIME(action)
    
#endif

    
}

#endif // algotest_time_included
