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

#ifndef __SYSTEM_UTILS_LOGGER_INCLUDED__
#define __SYSTEM_UTILS_LOGGER_INCLUDED__

#include <string>
//#define WITH_CERR_LOGGER

enum ELogSeverity
{
	KLogDebug=0,
	KLogInfo=1,
	KLogNotice=2,
	KLogWarning=3,
	KLogError=4,
	KLogFatal=5
};

class CSystemLogger
{
public:
	virtual ~CSystemLogger() {}
	virtual void SystemDebug(std::string message)=0;
	virtual bool CheckLogSerevity(const char * mod_name, ELogSeverity severity)=0;
	virtual void LogPrint(const char * mod_name, ELogSeverity severity, std::string message)=0;
};

#ifdef WITH_SYSTEM_LOGGER

#include <ostringstream>

extern CSystemLogger * g_Logger;
CSystemLogger * createFileLogger(const char * log_file_name);
void setSystemLogger(CSystemLogger * logger);
#define SU_LOG(module_name, severity, a) if (g_Logger && g_Logger->CheckLogSerevity(module_name,severity)) { std::ostringstream os; os<<a; g_Logger->LogPrint( module_name, severity, os.str() ); }

#elif defined WITH_CERR_LOGGER

#define SU_LOG(module_name, severity, a) std::cerr << module_name << " " << severity << " " << a << std::endl;

#else

#define SU_LOG(module_name, severity, a)

#endif

#endif // __SYSTEM_UTILS_LOGGER_INCLUDED__
