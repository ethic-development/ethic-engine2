#pragma once
#include "../log_reader/log_reader.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <regex>

namespace ethic
{
	namespace utils
	{
		namespace datamodel
		{
			std::uint64_t get_game_address();
			std::uint64_t get_place_id();
		}
	}
}