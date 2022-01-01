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

#ifndef algotest_c_h
#define algotest_c_h

#define _USE_MATH_DEFINES
#ifdef __cplusplus
#include <cmath>
#include <cstdlib>
#include <cassert>
#else
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#endif

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#if defined __APPLE__ && defined __OBJC__
#import <Foundation/NSString.h>
#endif

#include <sstream>

#if defined DEBUG || defined _DEBUG || defined SHOW_LOGI
#define WITH_LOGI
#endif

//#include "algotest_types.h"

#ifndef __APPLE__

#ifdef _MSC_VER
#define __printflike(fmtarg, firstvararg)
#endif

#ifndef __printflike
#define __printflike(fmtarg, firstvararg) \
		__attribute__((__format__ (__printf__, fmtarg, firstvararg)))
#endif //__printflike

#ifndef __scanflike
#define __scanflike(fmtarg, firstvararg) \
		__attribute__((__format__ (__scanf__, fmtarg, firstvararg)))
#endif //__scanflike

#endif

#ifdef ANDROID_NDK

namespace std
{
	inline double hypot(double a, double b) { return ::hypot(a,b); }
}

extern "C" void* aligned_alloc(size_t alignment, size_t size) __REMOVED_IN(28);

#endif //ANDROID_NDK

#define ARRSIZE(a) (sizeof(a)/sizeof((a)[0]))
#define DECLARE_NO_COPY(a) a(const a&); a&operator=(const a&);

namespace algotest
{
#if defined __APPLE__ && defined __OBJC__
    void logError(const char * file_name, int line_n, const char * func_name,
                  NSString * format_message, ...) NS_FORMAT_FUNCTION(4,5);
    void logInfo(NSString * format_message, ...) NS_FORMAT_FUNCTION(1,2);
#endif
    
    void logError(const char * file_name, int line_n, const char * func_name,
                  const char * format_message, ...) __printflike(4, 5);
    void logInfo(const char * format_message, ...) __printflike(1, 2);
    
#if defined WITH_LOGI
    void logInfo(const std::ostringstream& os);
#endif
}

#if defined WITH_LOGI
    #if defined __APPLE__ && defined __OBJC__
        #define LOGI(f, ...) algotest::logInfo( @f, ##__VA_ARGS__ )
    #else
        #define LOGI(...) algotest::logInfo(__VA_ARGS__)
    #endif

    #define LOGI_(a) algotest::logInfo( static_cast<const std::ostringstream&>(std::ostringstream() << a) );
#else
	#define LOGI(...)
    #define LOGI_(a)
#endif //DEBUG

// use ASSERT in algotest, because every include of assert.h redefines assert macro
#define ASSERT(x) if(!(x)) abort()

#if defined DISTRIBUTE // hide data from reverse-engineering
    // define assert to be void and force it to be void for subsequent include of assert.h
    #undef assert
    #define assert ASSERT
    #undef NDEBUG
    #define NDEBUG

    // use ASSERT in production to force abort for broken invariants (to get more informative crash reports)
    #define LOGR(...)
    #define LOGE(...)

#else
    #if defined __APPLE__ && defined __OBJC__

        #define LOGR(f, ...) algotest::logInfo( @f, ##__VA_ARGS__ )
        #define LOGE(f, ...) algotest::logError( __FILE__, __LINE__, __func__, @f, ##__VA_ARGS__ )

    #else
        #if defined DEBUG || defined _DEBUG || defined SHOW_LOGI || not defined ANDROID_NDK

            #define LOGR(...) algotest::logInfo(__VA_ARGS__)
            #define LOGE(...) algotest::logError( __FILE__, __LINE__, __func__, __VA_ARGS__)

        #else

            #define LOGR(...) algotest::logInfo(__VA_ARGS__)
            #define LOGE(...)

        #endif

    #endif
#endif // !DISTRIBUTE



#ifdef ALGOTEST
    template<class T> class AlgotestOnly final
    {
        T m_value;
    public:
        AlgotestOnly() = default;
        AlgotestOnly(const AlgotestOnly&) = default;
        AlgotestOnly(AlgotestOnly&&) = default;
        
        template<class... Args> AlgotestOnly(Args&&... args) : m_value(args...) {}
        operator const T&() { return m_value; }
    };
#else
    template<class T> class AlgotestOnly final
    {
    public:
        AlgotestOnly() = default;
        AlgotestOnly(const AlgotestOnly&) = default;
        AlgotestOnly(AlgotestOnly&&) = default;
        
        template<class... Args> AlgotestOnly(Args&&... args) {}
        operator T() { return T(); }
    };
#endif

#endif



