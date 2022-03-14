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

#ifndef __SYSTEM_UTILS_FILES_INCLUDED__
#define __SYSTEM_UTILS_FILES_INCLUDED__

#include <vector>
#include <string>

namespace sysutils
{

	class FileUtils
	{
	public:
		enum TFileFlags
		{
			KFolder = 1,
			KFile = 2
		};
	public:
		static bool listFolder(const char * path, const char * pattern, TFileFlags file_flags,
			const char* prefix_path, std::vector<std::string>& push_vect);

		static bool listFolderWithSubfolders(const char * path, const char * pattern, TFileFlags file_flags, std::vector<std::string>& push_vect);
		static std::string constructPath(const char * path, const char * name);
        static FILE * createFileWithPath(const char * path_name);
		static std::string getExtension(std::string file_name);
		static bool isFilePresent(std::string file_name);
		static std::string getNameWithoutExtension(std::string file_name);
		static std::string readFully(std::string file_name);
		static std::string readFullySafe(std::string file_name);
		static void flockfile(FILE * f);
		static void funlockfile(FILE * f);
		static std::string getWorkingFolder();
		static std::string getApplicationFolder();
		static std::string getFullPath(std::string path);
		static std::string getRelativePath(std::string path);
	};
}

#endif // __SYSTEM_UTILS_FILES_INCLUDED__
