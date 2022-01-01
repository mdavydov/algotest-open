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

#ifndef __SYSTEM_UTILS_STRINGS_INCLUDED__
#define __SYSTEM_UTILS_STRINGS_INCLUDED__

namespace sysutils
{

#ifndef __APPLE__

#ifndef __printflike
#define __printflike(fmtarg, firstvararg) \
		__attribute__((__format__ (__printf__, fmtarg, firstvararg)))
#endif

#endif

	class StringUtils
	{
	public:
        static std::string format(const char * format_message, ...)  __printflike(1, 2);
		static void trimSpaces(std::string& str);
		static void toUpper(std::string& str);
		static std::string replace(const char * original, const char * find_what, const char * replace_with);
		static std::string replace(std::string original, const char * find_what, const char * replace_with);
		static std::string replace(std::string original, const char * find_what, std::string replace_with);
		static std::string paramEncode(std::string p);

		static std::wstring toWide(const std::string& s);
		static std::wstring toWide(const std::wstring& s);
		static std::string toUtf8(const std::string& s);
		static std::string toUtf8(const std::wstring& s);

		static std::wstring toWide(const char * s);
		static std::wstring toWide(const wchar_t * s);
		static std::string toUtf8(const char * s);
		static std::string toUtf8(const wchar_t * s);
	};

}

using sysutils::StringUtils;

#endif //__SYSTEM_UTILS_STRINGS_INCLUDED__
