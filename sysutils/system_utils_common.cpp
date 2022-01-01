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

#include <string>
#include <thread>
#include "system_utils.h"
#include "ConvertUTF.h"


namespace sysutils
{

	TCommonException::TCommonException(std::string error_text, int error_code)
		: m_error_text(error_text), m_error_code(error_code)
	{
		SU_LOG("Common", KLogError, "EXCEPTION(" << error_code << ") " << error_text << ' ' << (error_code <= 0 ? "" : strerror(error_code)));
	}

	TCommonException::TCommonException(const char * module, ELogSeverity severity, std::string error_text, int error_code)
		: m_error_text(error_text), m_error_code(error_code)
	{
		SU_LOG(module, severity, "EXCEPTION(" << error_code << ") " << error_text << ' ' << (error_code <= 0 ? "" : strerror(error_code)));
	}

	std::string FileUtils::getExtension(std::string file_name)
	{
		size_t n = file_name.find_last_of('.');
		if (n != std::string::npos)
		{
			return file_name.substr(n + 1);
		}
		else
		{
			return std::string();
		}
	}

	bool FileUtils::isFilePresent(std::string file_name)
	{
		FILE * f = fopen(file_name.c_str(), "rb");
		if (f)
		{
			fclose(f);
			return true;
		}
		else
		{
			return false;
		}
	}

	std::string FileUtils::getNameWithoutExtension(std::string file_name)
	{
		size_t n = file_name.find_last_of('.');
		if (n != std::string::npos)
		{
			return file_name.substr(0, n);
		}
		else
		{
			return file_name;
		}
	}


    std::string StringUtils::format(const char * format_message, ...)
    {
        std::string s;
        
        va_list valist;
        va_start(valist, format_message);
        int size = vsnprintf(0, 0, format_message, valist);
        va_end(valist);
        
        va_start(valist, format_message);
        if (size>0)
        {
            s.resize(size+1);
            vsnprintf(&s[0], size+1, format_message, valist);
        }
        va_end(valist);
        
        return s;
    }

	void StringUtils::trimSpaces(std::string& str)
	{
		size_t startpos = str.find_first_not_of(" \t\n\r"); // Find the first character position after excluding leading blank spaces
		size_t endpos = str.find_last_not_of(" \t\n\r"); // Find the first character position from reverse af
		if ((std::string::npos == startpos) || (std::string::npos == endpos))
		{
			str = "";
		}
		else
		{
			str = str.substr(startpos, endpos - startpos + 1);
		}
	}

	void StringUtils::toUpper(std::string& str)
	{
		FORALL(std::string, str, i) *i = toupper(*i);
	}

	std::string StringUtils::toUtf8(const std::wstring& s)
	{
		if (!s.size()) return std::string();

		std::string res(s.size() * 3, 0);

		UTF8 * res_start = (UTF8*)&res[0];
		UTF8 * res_end = res_start + res.size();
		UTF8 * res_conv = res_start;

		const UTF16 * s_start = (UTF16*)&s[0];
		const UTF16 * s_end = s_start + s.size();

		if (conversionOK == ConvertUTF16toUTF8(&s_start, s_end, &res_conv, res_end, strictConversion))
		{
			res.resize(res_conv - res_start);
		}
		else
		{
			res.resize(0);
		}
		return res;
	}

	std::string StringUtils::toUtf8(const std::string& s)
	{
		return s;
	}

	std::wstring StringUtils::toWide(const std::string& s)
	{
		if (!s.size()) return std::wstring();

		std::wstring res(s.size(), 0);

		UTF16 * res_start = (UTF16*)&res[0];
		UTF16 * res_end = res_start + res.size();
		UTF16 * res_conv = res_start;

		const UTF8 * s_start = (UTF8*)&s[0];
		const UTF8 * s_end = s_start + s.size();


		if (conversionOK == ConvertUTF8toUTF16(&s_start, s_end, &res_conv, res_end, strictConversion))
		{
			res.resize(res_conv - res_start);
		}
		else
		{
			res.resize(0);
		}
		return res;
	}

	std::wstring StringUtils::toWide(const std::wstring& s)
	{
		return s;
	}

	std::wstring StringUtils::toWide(const char * s) { return toWide(std::string(s)); }
	std::wstring StringUtils::toWide(const wchar_t * s)  { return std::wstring(s); }
	std::string StringUtils::toUtf8(const char * s) { return std::string(s); }
	std::string StringUtils::toUtf8(const wchar_t * s) { return toUtf8(std::wstring(s)); }


	std::string StringUtils::replace(const char * original, const char * find_what, const char * replace_with)
	{
		size_t orig_len = strlen(original);
		size_t find_len = strlen(find_what);
		std::string res;
		res.reserve(orig_len);
		const char * p = original;
		while (*p)
		{
			if (0 == strncmp(p, find_what, find_len)) { res.append(replace_with); p += find_len; }
			else { res.append(1, *p); ++p; }
		}
		return res;
	}


	std::string StringUtils::replace(std::string original, const char * find_what, const char * replace_with)
	{
		return replace(original.c_str(), find_what, replace_with);
	}

	std::string StringUtils::replace(std::string original, const char * find_what, std::string replace_with)
	{
		return replace(original.c_str(), find_what, replace_with.c_str());
	}

	std::string StringUtils::paramEncode(std::string p)
	{
		p = StringUtils::replace(p, " ", "\\ ");
		p = StringUtils::replace(p, "`", "\\`");
		p = StringUtils::replace(p, "'", "\\'");
		p = StringUtils::replace(p, ";", "\\;");
		p = StringUtils::replace(p, "\"", "\\\"");
		p = StringUtils::replace(p, "(", "\\(");
		p = StringUtils::replace(p, ")", "\\)");
		p = StringUtils::replace(p, "<", "\\<");
		p = StringUtils::replace(p, ">", "\\>");
		p = StringUtils::replace(p, "*", "\\*");
		p = StringUtils::replace(p, "&", "\\&");
		return p;
	}

	int getOptimalParallelThreads()
	{
        int n = std::thread::hardware_concurrency(); // can retucn 0!
        return n>1?n:1;
	}
}
