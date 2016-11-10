#pragma once

#include <sstream>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <cereal/archives/xml.hpp>
#include <sys/stat.h>

//// http://stackoverflow.com/questions/17818099/how-to-check-if-a-file-exists-before-creating-a-new-file
inline bool file_exist(const char * filename)
{
	struct stat fileInfo;
	return stat(filename, &fileInfo) == 0;
}

//// http://stackoverflow.com/questions/8520560/get-a-file-name-from-a-path
inline std::string base_filename(std::string const & path)
{
	auto filename = path.substr(path.find_last_of("/\\") + 1);
	return filename.substr(0, filename.find_last_of('.'));
}

struct TimeVaryingData
{
	std::string mhd = "D:/_time_varying_data/vortex_raw/vorts";
	std::string mhd2 = ".mhd";
	std::string tfi = "D:/document/work/time-varying-visualization/~plot/vorts";
	std::string tfi2 = "_optimized_parallelsearch.tfi";

	int min_index()
	{
		return 1;
	}

	int max_index()
	{
		return 99;
	}

	std::string transferfunction_filename(int index)
	{
		std::stringstream ss;
		ss << tfi << std::to_string(index) << tfi2;
		return ss.str();
	}

	std::string volume_filename(int index)
	{
		std::stringstream ss;
		ss << mhd << std::to_string(index) << mhd2;
		return ss.str();
	}
};