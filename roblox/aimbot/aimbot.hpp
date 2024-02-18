#pragma once

#include "../classes/classes.hpp"

namespace ethic
{
	namespace roblox
	{

		ethic::roblox::instance_t get_closest_player_to_cursor();
		ethic::roblox::vector2_t get_relative_player_to_pos(ethic::roblox::vector2_t closest_player);
		void hook_aimbot();
	}
}