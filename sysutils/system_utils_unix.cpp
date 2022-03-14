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

#include <cstdio>
#include <cerrno>
#include <string>
#include <pthread.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <ctime>
#include <atomic>
#include <sys/ioctl.h>
#include <sys/stat.h>

#ifdef __ANDROID__
#include <dirent.h>
#include <fnmatch.h>
#else
#include <glob.h>
#endif

#include "system_utils.h"

namespace sysutils
{
	void wait_ms(int ms)
	{
		if (ms >= 1000) sleep(ms / 1000);
		ms %= 1000;
		if (ms > 0) usleep(ms * 1000);
	}

	std::string timeStr(time_t t)
	{
		char buf[128];
		char* time_str = ctime_r(&t, buf);
		if (NULL != time_str)
		{
			char* nl_pos = strrchr(time_str, '\n');
			if (nl_pos) *nl_pos = 0;
		}
		return time_str ? std::string(time_str) : std::string();
	}

	tm localtime_safe(time_t t)
	{
		tm aTm;
		localtime_r(&t, &aTm);
		return aTm;
	}

	tm gmtime_safe(time_t t)
	{
		tm aTm;
		gmtime_r(&t, &aTm);
		return aTm;
	}

	int safe_system(const char* aCommandLine, std::string * command_output, size_t max_output_size)

	{
		int res = -1;
		FILE* f = popen(aCommandLine, "r");
		if (f)
		{
			if (command_output)
			{
				char buf[100];
				size_t n_read;
				do
				{
					memset(buf, 0, sizeof(buf));
					n_read = fread(buf, 1, 90, f);
					*command_output += buf;
				} while (n_read > 0 && command_output->size() < max_output_size);
				if (command_output->size() > max_output_size) *command_output = command_output->substr(0, max_output_size);
			}
			res = pclose(f);
		}

		return (res >> 8);
	}

	pAtomic atomicAlloc(int init_value)
	{
		std::atomic_int * p = new std::atomic_int(init_value);
		return (pAtomic)p;
	}
	void atomicFree(pAtomic p)
	{
		delete (std::atomic_int*)p;
	}
	bool atomicDecAndZeroTest(pAtomic p)
	{
        return --(*(std::atomic_int*)p)==0;
	}
	void atomicInc(pAtomic p)
	{
		++(*(std::atomic_int*)p);

	}
	int atomicRead(pAtomic p)
	{
        return *(std::atomic_int*)p;
	}

	void FileUtils::flockfile(FILE * f)
	{
		::flockfile(f);
	}

	void FileUtils::funlockfile(FILE * f)
	{
		::funlockfile(f);
	}

	bool FileUtils::listFolder(const char * path, const char * pattern, TFileFlags file_flags,
		const char* prefix_path, std::vector<std::string>& push_vect)
	{
#ifdef ANDROID_NDK
		DIR* dirp = opendir(path);
		if ( !dirp )
		{
			SU_LOG("system_utils_unix", KLogError, strerror(errno));
			return false;
		}

		dirent* dp;
		while ( (dp = readdir(dirp)) )
		{
			const char* file_name = dp->d_name;

			if ( fnmatch(pattern, file_name, 0) == 0 && file_name[0] != '.' &&
					((dp->d_type == DT_DIR && (file_flags & KFolder)) || (dp->d_type == DT_REG && (file_flags & KFile))) )
			{
				push_vect.push_back(constructPath(prefix_path, file_name));
			}
		}

		closedir(dirp);

		std::sort(push_vect.begin(), push_vect.end());

		return true;
#else
		std::string full_pattern = constructPath(path, pattern);

        glob_t * globbuf=(glob_t*)malloc(sizeof(glob_t));
		memset(globbuf, 0, sizeof(glob_t));
		//LOGI("%s\n", full_pattern.c_str());
		if (glob(full_pattern.c_str(), GLOB_MARK, 0, globbuf) != 0)
		{
            globfree(globbuf);
            free(globbuf);
			//LOGI("glob error\n");
			return false;
		}

		for (size_t i = 0; i < globbuf->gl_pathc; ++i)
		{
			bool isFolder = false;
			//LOGI("found %s\n", globbuf.gl_pathv[i]);
			char * last_slash = strrchr(globbuf->gl_pathv[i], '/');
			if (last_slash && last_slash[1] == 0)
			{
				// it is a folder
				//LOGI("it is folder!!!\n", globbuf.gl_pathv[i]);
				isFolder = true;
				*last_slash = 0;
			}
			char * file_name = strrchr(globbuf->gl_pathv[i], '/');
			if (!file_name)
			{
				file_name = globbuf->gl_pathv[i];
			}
			else
			{
				file_name += 1;
			}
			//LOGI("file name is %s\n",file_name);
			if (file_name[0] != 0 && file_name[0] != '.' &&
				((isFolder && (file_flags&KFolder)) || (!isFolder && (file_flags&KFile))))
			{
				push_vect.push_back(constructPath(prefix_path, file_name));
			}
		}

		globfree(globbuf);
        free(globbuf);
		return true;
#endif
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
    
    FILE * FileUtils::createFileWithPath(const char * path_name)
    {
        FILE * f = fopen(path_name, "w+b");
        if (f) return f;
        
        // try to construct folder
        size_t prev = 0;
        std::string path_str = path_name;
        for(;;)
        {
            size_t split = path_str.find('/', prev+1);
            if (split == std::string::npos) break;
            prev = split;
            
            path_str[split] = 0;
            int res = mkdir(path_str.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            if (res!=0 && errno!=EEXIST) break;
            path_str[split] = '/';
        }

        return fopen(path_name, "w+b");
    }





	std::string FileUtils::getWorkingFolder()
	{
		char work_folder[256];
		getcwd(work_folder, 256);
		return work_folder;
	}


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

	void mutex_test()
	{
		for (int i = 0; i < 100; ++i)
		{
			pthread_mutex_t m_mutex;
			pthread_mutexattr_t    attr;
			memset(&attr, 0, sizeof(attr));
			memset(&m_mutex, 0, sizeof(m_mutex));
			pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
			int res = pthread_mutex_init(&m_mutex, &attr);
			pthread_mutexattr_destroy(&attr);

			if (res != 0)
			{
				//LOGI("init mutex failed!!!");
			}
			else
			{
				//LOGI("init mutex OK!!!");
				pthread_mutex_destroy(&m_mutex);
			}
		}
	}


	void SystemOSUtils::getMemInfo(int& total_kbytes, int& free_kbytes)
	{
		FILE* fp;
		char buf[1024];
		unsigned long 		used, mfree, shared, buffers, cached, total;

		fp = fopen("/proc/meminfo", "r");
		if (fscanf(fp, "MemTotal: %lu %s\n", &total, buf) != 2)
		{
			fgets(buf, sizeof(buf), fp);    /* skip first line */

			fscanf(fp, "Mem: %lu %lu %lu %lu %lu %lu",
				&total, &used, &mfree, &shared, &buffers, &cached);

			used /= 1024;
			mfree /= 1024;
			shared /= 1024;
			buffers /= 1024;
			cached /= 1024;
			total /= 1024;
			free_kbytes = (int)mfree;
			total_kbytes = (int)total;
		}
		else
		{
			fscanf(fp, "MemFree: %lu %s\n", &mfree, buf);

			total_kbytes = (int)total;
			free_kbytes = (int)mfree;

			if (fscanf(fp, "MemShared: %lu %s\n", &shared, buf) != 2)
				shared = 0;

			fscanf(fp, "Buffers: %lu %s\n", &buffers, buf);
			fscanf(fp, "Cached: %lu %s\n", &cached, buf);
			used = total - mfree;
		}
		fclose(fp);
	}

#ifdef __APPLE__

#include <mach/mach_time.h>
#define ORWL_NANO (+1.0E-9)
#define ORWL_GIGA UINT64_C(1000000000)

	static double orwl_timebase = 0.0;
	static uint64_t orwl_timestart = 0;

	struct timespec orwl_gettime(void) {
		// be more careful in a multithreaded environement
		if (!orwl_timestart) {
			mach_timebase_info_data_t tb = { 0 };
			mach_timebase_info(&tb);
			orwl_timebase = tb.numer;
			orwl_timebase /= tb.denom;
			orwl_timestart = mach_absolute_time();
		}
		struct timespec t;
		double diff = (mach_absolute_time() - orwl_timestart) * orwl_timebase;
		t.tv_sec = diff * ORWL_NANO;
		t.tv_nsec = diff - (t.tv_sec * ORWL_GIGA);
		return t;
	}

#endif

	class PerformanceCounterUnix : public PerformanceCounter
	{
		static double get_elapsed_time(
			const struct timespec * start_time,
			const struct timespec * end_time)
		{
			return double(end_time->tv_sec - start_time->tv_sec) + 1.0e-9 *(end_time->tv_nsec - start_time->tv_nsec);
		}

		struct timespec m_start_time;

	public:
		PerformanceCounterUnix()	//* constructs and restarts
		{
			restart();
		}
		void restart()	//* resets counter
		{
#ifdef __APPLE__
			m_start_time = orwl_gettime();
#else
			[[maybe_unused]] struct timespec start_time;
			clock_gettime(CLOCK_MONOTONIC_RAW, &m_start_time);
#endif       
		}

		double seconds() const	//* returns elapsed time in seconds 
		{
			struct timespec end_time;
#ifdef __APPLE__
			end_time = orwl_gettime();
#else
			clock_gettime(CLOCK_MONOTONIC_RAW, &end_time);
#endif
			return get_elapsed_time(&m_start_time, &end_time);
		}
	};

	ref_ptr<PerformanceCounter> createPerformanceCounter()
	{
		return new PerformanceCounterUnix();
	}
    
#ifndef __APPLE__
    std::string getStackTrace()
    {
        return std::string();
    }
#endif

}
