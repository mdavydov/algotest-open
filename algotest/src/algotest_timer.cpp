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
#include "algotest_timer.h"
#include "algotest_c.h"
#include "stlutil.h"
#include "system_utils.h"
#include <mutex>

#ifdef ALGOTEST
    #define TIMER_LOG LOGR
#else
    #define TIMER_LOG LOGI
#endif

namespace algotest
{
	using namespace sysutils;

	class AlgoTimer
	{
		std::string m_name;
		const PerformanceCounter& m_pc;
		double m_start_time;
		double m_prev_time;
		long m_num_pixels;
        bool m_active;
        
        struct TUsageData
        {
            const char * m_f_name;
            int m_use_count;
            double m_use_total_time;
        };
        
		std::vector<TUsageData> m_use_counter;
        std::mutex m_use_counter_mutex;


	public:
		AlgoTimer(std::string name, const PerformanceCounter& pc, bool active)
			: m_name(name), m_pc(pc), m_active(active)
		{
		}

		double time() { return m_pc.seconds(); }

		void start(long num_pixels)
		{
            if (!m_active) return;
            
			m_num_pixels = (long)num_pixels;
            {
                SYNC(m_use_counter_mutex);
                m_use_counter.clear();
                m_use_counter.reserve(100);
            }
			TIMER_LOG("[%s] start (%10.3f mp)\n", m_name.c_str(), num_pixels/1024.0/1024.0);
            m_start_time = m_prev_time = m_pc.seconds();
		}

		void notifyTime(const char * action)
		{
            if (!m_active) return;
            
			double t = m_pc.seconds();
            double mpps;
            mpps = m_num_pixels / 1024.0 / 1024.0 / (t - m_prev_time + 0.0000001);
			TIMER_LOG("[%s] %35s %6.3f (%10.3f mpps)\n", m_name.c_str(), action, /*t-m_start_time,*/ t-m_prev_time, mpps);
			m_prev_time = t;
		}

		void notifyUse(const char * function_name, double time)
		{
            if (!m_active) return;
            
            size_t s = m_use_counter.size();
            for( size_t i=0;i<s;++i)
            {
                if (m_use_counter[i].m_f_name == function_name)
                {
                    m_use_counter[i].m_use_count += 1;
                    m_use_counter[i].m_use_total_time += time;
                    return;
                }
            }
            
            SYNC(m_use_counter_mutex);
            
            // search if some blocks were added
            size_t s2 = m_use_counter.size();
            for( size_t i=s;i<s2;++i)
            {
                if (m_use_counter[i].m_f_name == function_name)
                {
                    m_use_counter[i].m_use_count += 1;
                    m_use_counter[i].m_use_total_time += time;
                    return;
                }
            }
            
            ASSERT(m_use_counter.size()<99);
            m_use_counter.push_back(TUsageData{function_name, 1, time});
		}

		void finish()
		{
            if (!m_active) return;
            
			double t = m_pc.seconds();
            double mpps;
            mpps = m_num_pixels / 1024.0 / 1024.0 / (t - m_start_time + 0.0000001);
            
            TIMER_LOG("[%s] total %6.3f sec (%.3f mpps)\n", m_name.c_str(), t-m_start_time, mpps);
            
            {
                SYNC(m_use_counter_mutex);
                
                size_t s = m_use_counter.size();
                
                std::sort(m_use_counter.begin(), m_use_counter.end(),
                          [=](const TUsageData & a, const TUsageData & b) -> bool
                          {
                              return a.m_use_total_time > b.m_use_total_time;
                          });
                
                for( size_t i=0;i<s;++i)
                {
                    TIMER_LOG("% 10d % 10f(%0.2f%%) -> %s\n", m_use_counter[i].m_use_count,
                            m_use_counter[i].m_use_total_time,
                            m_use_counter[i].m_use_total_time * 100 / (t-m_start_time),
                            m_use_counter[i].m_f_name);
                }
                m_use_counter.resize(0);
                m_use_counter.reserve(100);
            }
		}
		const std::string& getName() { return m_name; }
	};

	class TimerImpl
	{
		static ref_ptr<PerformanceCounter> s_counter;
		static std::vector< AlgoTimer * > s_algo_stack;
    private:
        
        static void createTimerIfNeeded()
        {
            if (!s_counter) s_counter = createPerformanceCounter();
        }

	public:
        static double getTimeInSeconds()
        {
            createTimerIfNeeded();
            return s_counter->seconds();
        }
		static AlgoTimer * pushTimer(const std::string& algo_name, bool active)
		{
            createTimerIfNeeded();
			s_algo_stack.push_back(new AlgoTimer(algo_name, *s_counter, active));
			return s_algo_stack.back();
		}
        static bool hasTimer()
        {
            return s_algo_stack.size()>0;
        }
		static AlgoTimer * getTimer()
		{
			ASSERT(s_algo_stack.size()>0);
			return s_algo_stack.back();
		}
		static void stopTimer()
		{
			AlgoTimer * t = getTimer();
			t->finish();
			s_algo_stack.pop_back();
			delete t;
		}
	};

	ref_ptr<PerformanceCounter> TimerImpl::s_counter;
	std::vector< AlgoTimer * > TimerImpl::s_algo_stack;

    static std::mutex g_timer_mutex;
    
    double Timer::getTimeInSeconds()
    {
        SYNC(g_timer_mutex);
        
        return TimerImpl::getTimeInSeconds();
    }
	void Timer::start(const std::string& algo_name, long num_pixels, bool active)
	{
        SYNC(g_timer_mutex);
		TimerImpl::pushTimer(algo_name, active)->start(num_pixels);
	}
	void Timer::notifyTime(const char * action)
	{
        SYNC(g_timer_mutex);
        if (!TimerImpl::hasTimer()) return;
		TimerImpl::getTimer()->notifyTime(action);
	}
	void Timer::finish()
	{
        SYNC(g_timer_mutex);
		TimerImpl::stopTimer();
	}

	FunctionProfiler::FunctionProfiler(const char * f_name) : m_name(f_name)
	{
        SYNC(g_timer_mutex);
		m_start_time = TimerImpl::getTimer()->time();
	}
	FunctionProfiler::~FunctionProfiler()
	{
        SYNC(g_timer_mutex);
		double end_time = TimerImpl::getTimer()->time();
		TimerImpl::getTimer()->notifyUse(m_name, end_time-m_start_time);
	}
    
    static sysutils::pAtomic g_time_critical_counter=0;
    
    TimeCriticalStarter::TimeCriticalStarter()
    {
        if (!g_time_critical_counter) g_time_critical_counter = sysutils::atomicAlloc(0);
        sysutils::atomicInc( g_time_critical_counter );
    }
    TimeCriticalStarter::~TimeCriticalStarter()
    {
        sysutils::atomicDecAndZeroTest(g_time_critical_counter);
    }
    bool TimeCriticalStarter::isTimeCriticalInAction()
    {
        if (!g_time_critical_counter) return false;
        return sysutils::atomicRead(g_time_critical_counter)>0;
    }
}
