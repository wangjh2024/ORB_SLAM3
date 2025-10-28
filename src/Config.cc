/**
* This file is part of ORB-SLAM3
*
* Copyright (C) 2017-2021 Carlos Campos, Richard Elvira, Juan J. Gómez Rodríguez, José M.M. Montiel and Juan D. Tardós, University of Zaragoza.
* Copyright (C) 2014-2016 Raúl Mur-Artal, José M.M. Montiel and Juan D. Tardós, University of Zaragoza.
*
* ORB-SLAM3 is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
* License as published by the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* ORB-SLAM3 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
* the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with ORB-SLAM3.
* If not, see <http://www.gnu.org/licenses/>.
*/


#include "Config.h"

#ifdef _WIN32
#include <cstdint>
#include <io.h>
#include <direct.h>
#include <windows.h>
#define sleep(seconds) Sleep((seconds) * 1000)
#else
#include <unistd.h>
#include <stdint-gcc.h>
#endif

#include <chrono>
#include <thread>

// 跨平台usleep替代
inline void portable_usleep(int microseconds) {
	std::this_thread::sleep_for(std::chrono::microseconds(microseconds));
}



namespace ORB_SLAM3
{

	bool ConfigParser::ParseConfigFile(std::string& strConfigFile)
	{
		return true;
	}

}
