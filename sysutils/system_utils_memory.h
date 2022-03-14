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

#ifndef __SYSTEM_UTILS_MEMORY_INCLUDED__
#define __SYSTEM_UTILS_MEMORY_INCLUDED__

namespace sysutils
{

	class SystemOSUtils
	{
	public:
		static void				getMemInfo(int& total_kbytes, int& free_kbytes);
		static std::string		generateTemporaryFilename();
	};

}

#endif // __SYSTEM_UTILS_MEMORY_INCLUDED__
