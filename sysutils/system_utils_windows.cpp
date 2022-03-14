/*  The SysUtil library for Threads/Sockets/PG Database/RPC
    Copyright (C) 2007-2012 Maksym Davydov (http://www.adva-soft.com)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; version 2.1

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
   
    If you have any questions contact via ICQ and describe your question is authorization query
*/

#ifdef _WIN32

#include <windows.h>
#include <stdio.h>
#include <io.h>
#include <direct.h>
#include <string.h>
#include <string>
#include <time.h>
#include "system_utils.h"

namespace sysutils
{

	const char KThread[] = "Thread";


	static TMutex timeMutex;

	std::string timeStr(time_t t)
	{
		char buf[128];
		memset(buf, 0, sizeof(buf));

		{
			SYNC(timeMutex);
			char* time_str = ctime(&t);
			strncpy(buf, time_str, 100);
		}

		char* nl_pos = strrchr(buf, '\n');
		if (nl_pos) *nl_pos = 0;

		return std::string(buf);
	}

	tm localtime_safe(time_t t)
	{
		SYNC(timeMutex);
		tm * pTm = localtime(&t);
		return *pTm;
	}

	tm gmtime_safe(time_t t)
	{
		SYNC(timeMutex);
		tm * pTm = gmtime(&t);
		return *pTm;
	}

	void wait_ms(int ms)
	{
		Sleep(ms);
	}

	pAtomic atomicAlloc(int init_value)
	{
		return new long(init_value);
	}
	void atomicFree(pAtomic p)
	{
		delete (long*)p;
	}
	bool atomicDecAndZeroTest(pAtomic p)
	{
		return 0 == InterlockedDecrement((long*)p);
	}
	void atomicInc(pAtomic p)
	{
		InterlockedIncrement((long*)p);
	}
	int atomicRead(pAtomic p)
	{
		return int(*(long*)p);
	}

	bool FileUtils::listFolder(const char * path, const char * pattern, TFileFlags file_flags,
		const char* prefix_path, std::vector<std::string>& push_vect)
	{
		std::string full_pattern = constructPath(path, pattern);

		_finddata_t fd;
		long handle = _findfirst(full_pattern.c_str(), &fd);
		if (-1L == handle) return false;
		do
		{
			if (((fd.attrib&_A_SUBDIR) && (file_flags&KFolder)) ||
				(!(fd.attrib&_A_SUBDIR) && (file_flags&KFile)))
			{
				if (fd.name[0] != '.') push_vect.push_back(constructPath(prefix_path, fd.name));
			}
		} while (-1L != _findnext(handle, &fd));
		_findclose(handle);
		return true;
	}

	bool FileUtils::listFolderWithSubfolders(const char * path, const char * pattern, TFileFlags file_flags, std::vector<std::string>& push_vect)
	{
		std::vector<std::string> subdirs;
		listFolder(path, pattern, file_flags, path, push_vect);
		listFolder(path, "*", KFolder, path, subdirs);
		FORALL(std::vector<std::string>, subdirs, i)
		{
			listFolderWithSubfolders(i->c_str(), pattern, file_flags, push_vect);
		}
		return true;
	}

	std::string FileUtils::constructPath(const char * path, const char * name)
	{
		std::string full_path = path;
		if (full_path.size() >= 1 &&
			full_path[full_path.size() - 1] != '/' &&
			full_path[full_path.size() - 1] != '\\')
		{
			full_path += "/";
		}
		full_path += name;
		return full_path;
	}

	//int FileUtils::truncate(FILE * f, int len)
	//{
	//	return _chsize(f->_file, len);
	//}

	std::string FileUtils::readFully(std::string file_name)
	{
		ref_ptr<FILE> f = fopen(file_name.c_str(), "rb");
		if (!f) throw TCommonException("Can't open file " + file_name);
		if (fseek(f, 0, SEEK_END)) throw TCommonException("Can't seek file " + file_name);
		std::string res;
		res.resize(ftell(f));
		if (fseek(f, 0, SEEK_SET)) throw TCommonException("Can't seek file " + file_name);
		if (fread(&res[0], 1, res.size(), f) != res.size()) throw TCommonException("Can't read full file " + file_name);
		return res;
	}

	std::string FileUtils::readFullySafe(std::string file_name)
	{
		ref_ptr<FILE> f = fopen(file_name.c_str(), "rb");
		if (!f) return std::string();
		if (fseek(f, 0, SEEK_END)) return std::string();
		std::string res;
		res.resize(ftell(f));
		if (fseek(f, 0, SEEK_SET)) return std::string();
		if (fread(&res[0], 1, res.size(), f) != res.size()) return std::string();
		return res;
	}

	std::string FileUtils::getApplicationFolder()
	{
		WCHAR buf[512];
		GetModuleFileNameW(0, buf, 512);
		std::string s = StringUtils::toUtf8(buf);
		int last_slash = s.find_last_of("\\/");
		if (last_slash != std::string::npos)
		{
			s.resize(last_slash);
		}
		return s;
	}

	std::string FileUtils::getFullPath(std::string path)
	{
		if ((path.length() >= 2 && path[1] == ':') ||
			(path.length() >= 1 && path[0] == '\\') ||
			(path.length() >= 1 && path[0] == '/')) return path;

		TCHAR module_name[MAX_PATH];
		GetModuleFileName(0, module_name, MAX_PATH);
		std::string s_module_name = StringUtils::toUtf8(module_name);
		int last = s_module_name.find_last_of("/\\");
		s_module_name = s_module_name.substr(0, last + 1);
		return s_module_name + path;
	}

	std::string FileUtils::getRelativePath(std::string path)
	{
		TCHAR module_name[MAX_PATH];
		GetModuleFileName(0, module_name, MAX_PATH);
		std::string s_module_name = StringUtils::toUtf8(module_name);
		int last = s_module_name.find_last_of("/\\");
		s_module_name = s_module_name.substr(0, last + 1);
		std::string start_path = path.substr(0, last + 1);
		if (_stricmp(start_path.c_str(), s_module_name.c_str()) == 0)
		{
			return path.substr(last + 1);
		}
		else
		{
			return path;
		}
	}

	void FileUtils::flockfile(FILE * f)
	{
	}
	void FileUtils::funlockfile(FILE * f)
	{

	}

	std::string FileUtils::getWorkingFolder()
	{
		char work_folder[256];
		_getcwd(work_folder, 256);
		return work_folder;
	}




	void SystemOSUtils::getMemInfo(int& total_kbytes, int& free_kbytes)
	{
		MEMORYSTATUS stat;
		GlobalMemoryStatus(&stat);
		total_kbytes = stat.dwTotalPhys / 1024;
		free_kbytes = stat.dwAvailPhys / 1024;
	}

	std::string	SystemOSUtils::generateTemporaryFilename()
	{
		time_t time_now;
		time(&time_now);
		tm tm_now = localtime_safe(time_now);
		char buffer[1024];
		sprintf(buffer, "%d%d%d%d%d%d",
			tm_now.tm_year + 1900,
			tm_now.tm_mon + 1,
			tm_now.tm_mday,
			tm_now.tm_hour,
			tm_now.tm_min,
			tm_now.tm_sec);
		return std::string(buffer);
	}

	class PerformanceCounterWindows : public PerformanceCounter
	{
		__int64 li;		//* perf. counter
		__int64 freq;	//* timer frequency

	public:
		PerformanceCounterWindows()	//* constructs and restarts
		{
			QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&freq));
			restart();
		}
		void restart()	//* resets counter
		{
			QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&li));
		}
		double seconds() const	//* returns elapsed time in seconds 
		{
			__int64 li1;
			QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&li1));
			return double(li1 - li) / double(freq);
		}
	};

	ref_ptr<PerformanceCounter> createPerformanceCounter()
	{
		return new PerformanceCounterWindows();
	}

}

#endif // _WINDOWS_
