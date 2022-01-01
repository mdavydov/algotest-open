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

#ifndef __SYSTEM_UTILS_EXCEPTIONS_INCLUDED__
#define __SYSTEM_UTILS_EXCEPTIONS_INCLUDED__

#include "system_utils_logger.h"

namespace sysutils
{
	class TCommonException
	{
	protected:
		std::string m_error_text;
		int m_error_code;
	public:
		TCommonException() : m_error_code(0) {}
		TCommonException(std::string error_text, int error_code = -1);
		TCommonException(const char * module, ELogSeverity severity, std::string error_text, int error_code = -1);
		const std::string& getErrorDescription() const { return m_error_text; }
		int getErrorCode() const { return m_error_code; }
	};

	template<class T>
	class TException : public TCommonException
	{
	public:
		TException() {}
		TException(TCommonException& c) : TCommonException(c) {}
		TException(const char * module, ELogSeverity severity, std::string error_text, int error_code = -1)
			: TCommonException(module, severity, error_text, error_code) {}
		TException(std::string error_text)
			: TCommonException("Common", KLogError, error_text, -1) {}
	};
}

#endif // __SYSTEM_UTILS_EXCEPTIONS_INCLUDED__
