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

#include <stdio.h>
#include <stdarg.h>
#include "algotest_c.h"
#include "system_utils.h"
#ifdef ANDROID_NDK
#include <android/log.h>
#ifdef LOG_IN_FILE
extern FILE *f_log;
#endif
#endif

#if defined __APPLE__ && defined __OBJC__
#import <Foundation/NSString.h>
#endif

namespace algotest
{
    enum { KMaxLogLength = 4096 };

    enum ELogLevel
    {
        KLogInfo,
        KLogError,
    };

    static bool hasPrefix(const char * format_message, const char * prefix)
    {
        return strncmp(format_message, prefix, strlen(prefix) )==0;
    }
    
    bool isErrorEnabled(const char * format_message)
    {
        if ( hasPrefix(format_message, "TODO:") ) return false;
        if ( hasPrefix(format_message, "ImgProvider:") ) return false;
        
        return true;
    }
    bool isInfoEnabled(const char * format_message)
    {
        if ( hasPrefix(format_message, "ResourceManager:") ) return false;
        if ( hasPrefix(format_message, "TODO:") ) return false;
        if ( hasPrefix(format_message, "TEXTURES:") ) return false;
        if ( hasPrefix(format_message, "SLICES:") ) return false;
        if ( hasPrefix(format_message, "SessionManager:") ) return false;
        if ( hasPrefix(format_message, "UndoData:") ) return false;
        if ( hasPrefix(format_message, "UndoDataStor:") ) return false;
       
        return true;
    }
    
    
    void log(const char * message, ELogLevel level=KLogInfo)
    {
#ifdef ANDROID_NDK

#ifdef LOG_IN_FILE
        printf("%s", message), printf("\n"), fflush(f_log);
#endif
        __android_log_print((level == KLogInfo ? ANDROID_LOG_INFO : ANDROID_LOG_ERROR),"algotest","%s", message);

#else
        fprintf(stderr, "%s\n", message );
#endif
    }
    
    
    
#if defined __APPLE__ && defined __OBJC__
    
    void logError(const char * file_name, int line_n, const char * func_name, NSString * format_message, ...)
    {
        if ( isErrorEnabled(format_message.UTF8String) )
        {
            char buf[KMaxLogLength];
        
            const char * last_slash = strrchr(file_name, '/');
            if (last_slash==0) { last_slash = file_name; } else { ++last_slash; }
            
            int num_written = snprintf(buf, 256, "%s(%d) %s: ", last_slash, line_n, func_name);
            if (num_written>256) num_written = 256;
            
            va_list valist;
            va_start(valist, format_message);
        
            NSString *nsstring = [[NSString alloc] initWithFormat:format_message arguments:valist];
        
            va_end(valist);
        
            snprintf(buf + num_written, KMaxLogLength-1-num_written, "%s", nsstring.UTF8String);
            
#if !__has_feature(objc_arc)
            [nsstring release];
#endif
            
            log( buf, KLogError );
        }
        else
        {
#if defined DEBUG && !defined ALGOTEST
            static bool first_time = true;
            if (first_time) log("SOME ERROR MESSAGES ARE NOT DISPLAYED. CHECK algotest_log.cpp (isErrorEnabled) FOR DETAILS.");
            first_time = false;
#endif
        }
    }

    void logInfo(NSString * format_message, ...)
    {
        if ( isInfoEnabled(format_message.UTF8String) )
        {
            va_list valist;
            va_start(valist, format_message);
            NSString *nsstring = [[NSString alloc] initWithFormat:format_message arguments:valist];
            va_end(valist);

            char buf[KMaxLogLength];
            snprintf(buf, KMaxLogLength-1, "%s", nsstring.UTF8String);
            
#if !__has_feature(objc_arc)
            [nsstring release];
#endif
            log( buf, KLogInfo );
        }
        else
        {
            static bool first_time = true;
            if (first_time) log("SOME INFO MESSAGES ARE NOT DISPLAYED. CHECK algotest_log.cpp(isInfoEnabled) FOR DETAILS.");
            first_time = false;
        }
    }
#endif
    
    void logError(const char * file_name, int line_n, const char * func_name, const char * format_message, ...)
    {
        if ( isErrorEnabled(format_message) )
        {
            va_list valist;
            va_start(valist, format_message);
            
            char buf[KMaxLogLength];
            
            const char * last_slash = strrchr(file_name, '/');
            if (last_slash==0) { last_slash = file_name; } else { ++last_slash; }
            
            int num_written = snprintf(buf, 256, "%s(%d) %s: ", last_slash, line_n, func_name);
            if (num_written>256) num_written = 256;
            
            vsnprintf(buf + num_written, KMaxLogLength-1-num_written, format_message, valist);
            va_end(valist);
            
            log( buf, KLogError );
        }
        else
        {
            static bool first_time = true;
            if (first_time) log("SOME ERROR MESSAGES ARE NOT DISPLAYED. CHECK algotest_log.cpp (isErrorEnabled) FOR DETAILS.");
            first_time = false;
        }
    }

    void logInfo(const char * format_message, ...)
    {
        if ( isInfoEnabled(format_message) )
        {
            va_list valist;
            va_start(valist, format_message);
            char buf[KMaxLogLength];
            vsnprintf(buf, KMaxLogLength-1, format_message, valist);
            va_end(valist);
            
            log( buf, KLogInfo );
        }
        else
        {
            static bool first_time = true;
            if (first_time) log("SOME INFO MESSAGES ARE NOT DISPLAYED. CHECK algotest_log.cpp(isInfoEnabled) FOR DETAILS.");
            first_time = false;
        }
    }
    
#ifdef WITH_LOGI
    void logInfo(const std::ostringstream& os)
    {
        log( os.str().c_str(), KLogInfo );
    }
#endif
}
