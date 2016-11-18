//////////////////////////////////////////////////////////////////////////
//// XML serialization for settings
//// Created on 2016-10-28 by Shengzhou Luo
//////////////////////////////////////////////////////////////////////////

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

struct Path
{
	std::string volume_filename = "../../data/CT-Knee.mhd";
	std::string transfer_function_filename = "../../transfer_function/CT-Knee_spectrum_6_bump_balance.tfi";
	std::string transfer_function_filename_save = "save_as.tfi";
	std::string selected_region_filename = "../../images/region.png";
	std::string batch_path = "D:/_time_varying_data/vortex_raw/";
	std::string xml;

	Path(const char * xml = "path.xml")
	{
		init(xml);
	}

	void init(const char * xml)
	{
		this->xml = xml;
		if (file_exist(xml))
		{
			load();
		}
		else
		{
			save();
		}
	}

	void load()
	{
		std::ifstream is(xml);
		cereal::XMLInputArchive archive(is);
		archive(volume_filename, transfer_function_filename, transfer_function_filename_save, selected_region_filename, batch_path);
	}

	void save()
	{
		std::ofstream os(xml);
		cereal::XMLOutputArchive archive(os);
		archive(CEREAL_NVP(volume_filename), CEREAL_NVP(transfer_function_filename), CEREAL_NVP(transfer_function_filename_save), CEREAL_NVP(selected_region_filename), CEREAL_NVP(batch_path));
	}
};

struct DynamicVortex
{
	std::string mhd_prefix = "D:/_time_varying_data/vortex_raw/vorts";
	std::string mhd_suffix = ".mhd";
	std::string tfi_prefix = "D:/document/work/time-varying-visualization/~plot/vorts";
	std::string tfi_suffix = "_optimized_parallelsearch.tfi";
	int min_index = 1;
	int max_index = 99;

	DynamicVortex(const char * xml = "DynamicVortex.xml")
	{
		init(xml);
	}

	void init(const char * xml)
	{
		if (file_exist(xml))
		{
			std::ifstream is(xml);
			cereal::XMLInputArchive archive(is);
			archive(CEREAL_NVP(mhd_prefix), CEREAL_NVP(mhd_suffix), CEREAL_NVP(tfi_prefix), CEREAL_NVP(tfi_suffix), CEREAL_NVP(min_index), CEREAL_NVP(max_index));
		}
		else
		{
			std::ofstream os(xml);
			cereal::XMLOutputArchive archive(os);
			archive(CEREAL_NVP(mhd_prefix), CEREAL_NVP(mhd_suffix), CEREAL_NVP(tfi_prefix), CEREAL_NVP(tfi_suffix), CEREAL_NVP(min_index), CEREAL_NVP(max_index));
		}
	}

	int index_min()
	{
		return min_index;
	}

	int index_max()
	{
		return max_index;
	}

	std::string transferfunction(int index)
	{
		std::stringstream ss;
		ss << tfi_prefix << std::to_string(index) << tfi_suffix;
		return ss.str();
	}

	std::string volume(int index)
	{
		std::stringstream ss;
		ss << mhd_prefix << std::to_string(index) << mhd_suffix;
		return ss.str();
	}
};

struct StaticVortex
{
	std::string mhd_prefix = "D:/_time_varying_data/vortex_raw/vorts";
	std::string mhd_suffix = ".mhd";
	std::string tfi_prefix = "D:/document/work/time-varying-visualization/feature_transfer_function/vortex_naive_proportional_optimized_linesearch.tfi";
	std::string tfi_suffix = "";
	int min_index = 1;
	int max_index = 99;

	StaticVortex(const char * xml = "StaticVortex.xml")
	{
		init(xml);
	}

	void init(const char * xml)
	{
		if (file_exist(xml))
		{
			std::ifstream is(xml);
			cereal::XMLInputArchive archive(is);
			archive(CEREAL_NVP(mhd_prefix), CEREAL_NVP(mhd_suffix), CEREAL_NVP(tfi_prefix), CEREAL_NVP(tfi_suffix), CEREAL_NVP(min_index), CEREAL_NVP(max_index));
		}
		else
		{
			std::ofstream os(xml);
			cereal::XMLOutputArchive archive(os);
			archive(CEREAL_NVP(mhd_prefix), CEREAL_NVP(mhd_suffix), CEREAL_NVP(tfi_prefix), CEREAL_NVP(tfi_suffix), CEREAL_NVP(min_index), CEREAL_NVP(max_index));
		}
	}

	int index_min()
	{
		return min_index;
	}

	int index_max()
	{
		return max_index;
	}

	std::string transferfunction(int index)
	{
		return tfi_prefix;
	}

	std::string volume(int index)
	{
		std::stringstream ss;
		ss << mhd_prefix << std::to_string(index) << mhd_suffix;
		return ss.str();
	}
};

struct NaiveVortex
{
	std::string mhd_prefix = "D:/_time_varying_data/vortex_raw/vorts";
	std::string mhd_suffix = ".mhd";
	std::string tfi_prefix = "D:/document/work/time-varying-visualization/feature_transfer_function/vortex_naive_proportional.tfi";
	std::string tfi_suffix = "";
	int min_index = 1;
	int max_index = 99;

	NaiveVortex(const char * xml = "NaiveVortex.xml")
	{
		init(xml);
	}

	void init(const char * xml)
	{
		if (file_exist(xml))
		{
			std::ifstream is(xml);
			cereal::XMLInputArchive archive(is);
			archive(CEREAL_NVP(mhd_prefix), CEREAL_NVP(mhd_suffix), CEREAL_NVP(tfi_prefix), CEREAL_NVP(tfi_suffix), CEREAL_NVP(min_index), CEREAL_NVP(max_index));
		}
		else
		{
			std::ofstream os(xml);
			cereal::XMLOutputArchive archive(os);
			archive(CEREAL_NVP(mhd_prefix), CEREAL_NVP(mhd_suffix), CEREAL_NVP(tfi_prefix), CEREAL_NVP(tfi_suffix), CEREAL_NVP(min_index), CEREAL_NVP(max_index));
		}
	}

	int index_min()
	{
		return min_index;
	}

	int index_max()
	{
		return max_index;
	}

	std::string transferfunction(int index)
	{
		return tfi_prefix;
	}

	std::string volume(int index)
	{
		std::stringstream ss;
		ss << mhd_prefix << std::to_string(index) << mhd_suffix;
		return ss.str();
	}
};
