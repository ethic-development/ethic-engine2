#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ethic
{
	namespace offsets
	{
		constexpr std::uint32_t size = 0x8; /* 8 bytes */
		constexpr std::uint32_t name = 0x48;
		constexpr std::uint32_t children = 0x50;
		constexpr std::uint32_t parent = 0x60;
		constexpr std::uint32_t local_player = 0x240;
		constexpr std::uint32_t model_instance = 0x188;
		constexpr std::uint32_t primitive = 0x148;
		constexpr std::uint32_t position = 0x144;
		constexpr std::uint32_t dimensions = 0x718;
		constexpr std::uint32_t viewmatrix = 0x4B0;
		constexpr std::uint32_t camera_pos = 0xA0;
		constexpr std::uint32_t classname = 0x18;
		constexpr std::uint32_t health = 0x358;
		constexpr std::uint32_t max_health = 0x360;
		constexpr std::uint32_t walkspeed = 0x340;
		constexpr std::uint32_t jumpspeed = 0x370;
		constexpr std::uint32_t team = 0x1D8;
		constexpr std::uint32_t gameid = 0x878;
	}

	namespace roblox
	{
		bool init();

		struct vector2_t final { float x, y; };
		struct vector3_t final { float x, y, z; };
		struct quaternion final { float x, y, z, w; };
		struct matrix4_t final { float data[16]; };

		class instance_t final
		{
		public:
			
			std::uint64_t self;

			std::string name();
			std::string class_name();
			std::vector<ethic::roblox::instance_t> children();
			ethic::roblox::instance_t find_first_child(std::string child);

			/* decided to have everything in one class rather than have multiple, it will be harder to manage (im lazy) */

			ethic::roblox::instance_t get_local_player();
			ethic::roblox::instance_t get_model_instance();
			ethic::roblox::instance_t get_team();
//			ethic::roblox::instance_t get_cframe();
			std::uintptr_t get_gameid();

			ethic::roblox::instance_t get_workspace();
			ethic::roblox::instance_t get_current_camera();

			ethic::roblox::vector2_t get_dimensions();
			ethic::roblox::matrix4_t get_view_matrix();
			ethic::roblox::vector3_t get_camera_pos(); 
			ethic::roblox::vector3_t get_part_pos();
			ethic::roblox::vector3_t get_part_velocity();

			float get_health();
			float get_max_health();

			void set_humanoid_walkspeed(float value);

			float get_ping();
		};

		ethic::roblox::vector2_t world_to_screen(ethic::roblox::vector3_t world);
	}
}