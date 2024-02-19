#include "classes.hpp"
#include <string>
#include "../../utils/datamodel/datamodel.hpp"
#include "../../utils/overlay/overlay.hpp"
#include "../driver/driver_impl.hpp"
#include "../globals/globals.hpp"
#include "../aimbot/aimbot.hpp"

#include "../../mapper/kernel_ctx/kernel_ctx.h"
#include "../../mapper/drv_image/drv_image.h"
#include "../../mapper/raw_driver.hpp"
#include "../../mapper/driver_data.hpp"

#include "../../utils/xorstr/xorstr.hpp"
#include "../../utils/json/json.hpp"

#include <curl/curl.h>
#include <Wininet.h>
#pragma comment(lib, "wininet.lib")

#pragma comment(lib, "libcurl.lib")
#include <fstream>
#include <thread>

std::uint64_t get_render_view()
{
	auto latest_log = ethic::utils::log::get_latest_log();

	std::ifstream rbx_log(latest_log);
	std::string rbx_log_line;

	while (true)
	{
		std::getline(rbx_log, rbx_log_line);
		if (rbx_log_line.contains(XorStr("initialize view(")))
		{
			rbx_log_line = rbx_log_line.substr(rbx_log_line.find(XorStr("initialize view(")) + 21);
			rbx_log_line = rbx_log_line.substr(0, rbx_log_line.find(')'));

			std::uint64_t renderview = std::strtoull(rbx_log_line.c_str(), nullptr, 16);
			return renderview;
		}
	}
}

std::uint64_t get_visualengine_address()
{
	return read<std::uint64_t>(get_render_view() + 0x10);
}


std::string replaceAll(std::string subject, const std::string& search, const std::string& replace)
{
	size_t pos = 0;
	while ((pos = subject.find(search, pos)) != std::string::npos)
	{
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	}
	return subject;
}

std::string DownloadString(std::string URL) {
	HINTERNET interwebs = InternetOpenA("Mozilla/5.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, NULL);
	HINTERNET urlFile;
	std::string rtn;
	if (interwebs) {
		urlFile = InternetOpenUrlA(interwebs, URL.c_str(), NULL, NULL, NULL, NULL);
		if (urlFile) {
			char buffer[2000];
			DWORD bytesRead;
			do {
				InternetReadFile(urlFile, buffer, 2000, &bytesRead);
				rtn.append(buffer, bytesRead);
				memset(buffer, 0, 2000);
			} while (bytesRead);
			InternetCloseHandle(interwebs);
			InternetCloseHandle(urlFile);
			std::string p = replaceAll(rtn, "|n", "\r\n");
			return p;
		}
	}
	InternetCloseHandle(interwebs);
	std::string p = replaceAll(rtn, "|n", "\r\n");
	return p;
}


bool ethic::roblox::init()
{
	/*
	if (!IsVersionUpToDate()) {
		MessageBox(NULL, XorStr("ethic is not updated, please wait for an update"), XorStr("discord.gg/ethicrbx"), MB_OK | MB_ICONWARNING);
		return false;
	}
	*/
	std::vector<std::uint8_t> drv_buffer(driverlol, raw_driver + sizeof(raw_driver));

	if (!drv_buffer.size())
	{
		printf(XorStr("Invalid driver buffer size.\n"));
		return false;
	}

	physmeme::drv_image image(drv_buffer);
	if (!physmeme::load_drv())
	{
		printf(XorStr("Failed to load driver.\n"));
		return false;
	}

	physmeme::kernel_ctx kernel_ctx;
	// printf("Driver has been loaded...\n");
	// printf("%s mapped physical page -> 0x%p\n", physmeme::syscall_hook.first, physmeme::psyscall_func.load());
	// printf("%s page offset -> 0x%x\n", physmeme::syscall_hook.first, physmeme::nt_page_offset);
	const auto drv_timestamp = util::get_file_header((void*)raw_driver)->TimeDateStamp;
	if (!kernel_ctx.clear_piddb_cache(physmeme::drv_key, drv_timestamp))
	{
		// This is because the signature might be broken on these versions of Windows.
		printf(XorStr("Couldn't clear PiDDB cache table.\n"));
		return false;
	}
	const auto get_export_name = [&](const char* base, const char* name)
	{
		return reinterpret_cast<std::uintptr_t>(util::get_kernel_export(base, name));
	};

	image.fix_imports(get_export_name);
	image.map();

	const auto pool_base = kernel_ctx.allocate_pool(image.size(), NonPagedPool);
	image.relocate(pool_base);
	kernel_ctx.write_kernel(pool_base, image.data(), image.size());
	auto entry_point = reinterpret_cast<std::uintptr_t>(pool_base) + image.entry_point();

	auto result = kernel_ctx.syscall<DRIVER_INITIALIZE>(
		reinterpret_cast<void*>(entry_point),
		reinterpret_cast<std::uintptr_t>(pool_base),
		image.size());

	printf(XorStr("Driver callback: 0x%p\n\n"), result);
	std::this_thread::sleep_for(std::chrono::milliseconds(1500));

	kernel_ctx.zero_kernel_memory(pool_base, image.header_size());
	if (!physmeme::unload_drv())
	{
		printf(XorStr("Unable to load driver.\n"));
		return false;
	}

	printf(XorStr("Connecting to server...\n"));
	std::this_thread::sleep_for(std::chrono::milliseconds(500));

	system("cls");
	if (!mem::find_driver())
	{
		printf(XorStr("Couldn't load driver.\n"));
		return false;
	}

	mem::process_id = mem::find_process(XorStr("RobloxPlayerBeta.exe"));
	if (!mem::process_id)
	{
		printf(XorStr("Failed to find Roblox process.\n"));
		for (;;);
		return false;
	}

	const auto visualengine = static_cast<ethic::roblox::instance_t>(get_visualengine_address());
	const auto game_ptr = read<std::uint64_t>(get_render_view() + 0xD8);
    auto game = static_cast<ethic::roblox::instance_t>(read<std::uint64_t>(game_ptr + 0x150));

	if (!visualengine.self)
	{
		MessageBoxA(nullptr, XorStr("Failed to get visual engine, please run the cheat as administrator!"), 
			XorStr("Ethic"), MB_OK | MB_ICONERROR);
	}

	if (!game.self)
	{
		MessageBoxA(nullptr, XorStr("failed to get datamodel, please run the cheat as administrator!"),
			XorStr("Ethic"), MB_OK | MB_ICONERROR);
	}

	globals::datamodel = game;

	auto players = game.find_first_child(XorStr("Players"));
	if (!players.self)
	{
		MessageBoxA(nullptr, XorStr("failed to get players, please run the cheat as administrator!"),
			XorStr("Ethic"), MB_OK | MB_ICONERROR);
	}

	globals::players = players;
	globals::visualengine = visualengine;

	std::string teamname = "";

	auto team = players.get_local_player().get_team();
	if (team.self)
	{
		teamname = team.name();
	}
	else
	{
		teamname = XorStr("none");
	}
	auto placeid = static_cast<ethic::roblox::instance_t>(ethic::utils::datamodel::get_place_id());

	if (!placeid.self)
	{
		return false;
	}

	globals::placeid = placeid;

	std::thread(ethic::roblox::hook_aimbot).detach();
	std::thread(ethic::utils::overlay::render).detach();

	printf(XorStr("Hiding console in 3 seconds...\n"));
	std::this_thread::sleep_for(std::chrono::seconds(3));

	ShowWindow(GetConsoleWindow(), SW_HIDE);

	while (true)
	{
		std::this_thread::sleep_for(std::chrono::seconds(10));

		if (FindWindowA(nullptr, XorStr("Roblox")) == nullptr)
		{
			physmeme::unload_drv();
			exit(0);
		}
	}

	return true;
}

ethic::roblox::vector2_t ethic::roblox::world_to_screen(ethic::roblox::vector3_t world)
{
	auto dimensions = globals::visualengine.get_dimensions();
	auto view_matrix = globals::visualengine.get_view_matrix();

	ethic::roblox::quaternion quaternion;
	quaternion.x = (world.x * view_matrix.data[0]) + (world.y * view_matrix.data[1]) + (world.z * view_matrix.data[2]) + view_matrix.data[3];
	quaternion.y = (world.x * view_matrix.data[4]) + (world.y * view_matrix.data[5]) + (world.z * view_matrix.data[6]) + view_matrix.data[7];
	quaternion.z = (world.x * view_matrix.data[8]) + (world.y * view_matrix.data[9]) + (world.z * view_matrix.data[10]) + view_matrix.data[11];
	quaternion.w = (world.x * view_matrix.data[12]) + (world.y * view_matrix.data[13]) + (world.z * view_matrix.data[14]) + view_matrix.data[15];

	if (quaternion.w < 0.1f)
	{
		return{ -1, -1 };
	}

	vector3_t ndc;
	ndc.x = quaternion.x / quaternion.w;
	ndc.y = quaternion.y / quaternion.w;
	ndc.z = quaternion.z / quaternion.w;

	return
	{
		(dimensions.x / 2 * ndc.x) + (ndc.x + dimensions.x / 2),
		-(dimensions.y / 2 * ndc.y) + (ndc.y + dimensions.y / 2)
	};
}

std::string readstring(std::uint64_t address)
{
	std::string string = "";
	char character = 0;
	int char_size = sizeof(character);
	int offset = 0;

	string.reserve(204);

	while (offset < 200)
	{
		character = read<char>(address + offset);

		if (character == 0)
			break;

		offset += char_size;
		string.push_back(character);
	}

	return string;
}

std::string readstring2(std::uint64_t string)
{
	const auto length = read<int>(string + 0x18);

	if (length >= 16u)
	{
		const auto New = read<std::uint64_t>(string);
		return readstring(New);
	}
	else
	{
		const auto name = readstring(string);
		return name;
	}
}

std::string ethic::roblox::instance_t::name()
{
	const auto ptr = read<std::uint64_t>(this->self + ethic::offsets::name);
	return !ptr ? XorStr("???") : readstring2(ptr);
}

std::string ethic::roblox::instance_t::class_name()
{
	const auto ptr = read<std::uint64_t>(this->self + ethic::offsets::classname);
	return !ptr ? XorStr("???_classname") : readstring2(ptr + 0x8);
}

std::vector<ethic::roblox::instance_t> ethic::roblox::instance_t::children()
{
	std::vector<ethic::roblox::instance_t> container;

	if (!this->self)
	{
		return container;
	}

	auto start = read<std::uint64_t>(this->self + ethic::offsets::children);

	if (!start)
	{
		return container;
	}

	auto end = read<std::uint64_t>(start + offsets::size);

	for (auto instances = read<std::uint64_t>(start); instances != end; instances += 16)
	{
		container.emplace_back(read<ethic::roblox::instance_t>(instances));
	}

	return container;
}

ethic::roblox::instance_t ethic::roblox::instance_t::find_first_child(std::string child)
{
	ethic::roblox::instance_t ret;

	for (auto &object : this->children())
	{
		if (object.name() == child)
		{
			ret = static_cast<ethic::roblox::instance_t>(object);
		}
	}

	return ret;
}

void ethic::roblox::instance_t::set_humanoid_walkspeed(float value)
{
	auto humanoid_instance = this->get_model_instance().find_first_child("Humanoid");

	if (humanoid_instance.self)
	{
		write<float>(humanoid_instance.self + ethic::offsets::walkspeed, value);
	}
}

float ethic::roblox::instance_t::get_ping()
{
	float ping = read<float>(this->self + 0x3F8);
	return ping * 2000;
}

ethic::roblox::instance_t ethic::roblox::instance_t::get_local_player()
{
	auto local_player = read<ethic::roblox::instance_t>(this->self + ethic::offsets::local_player);
	return local_player;
}

ethic::roblox::instance_t ethic::roblox::instance_t::get_model_instance()
{
	auto character = read<ethic::roblox::instance_t>(this->self + ethic::offsets::model_instance);
	return character;
}

ethic::roblox::vector2_t ethic::roblox::instance_t::get_dimensions()
{
	auto dimensions = read<ethic::roblox::vector2_t>(this->self + offsets::dimensions);
	return dimensions;
}

ethic::roblox::matrix4_t ethic::roblox::instance_t::get_view_matrix()
{
	auto dimensions = read<ethic::roblox::matrix4_t>(this->self + offsets::viewmatrix);
	return dimensions;
}

ethic::roblox::vector3_t ethic::roblox::instance_t::get_camera_pos()
{
	auto camera_pos = read<ethic::roblox::vector3_t>(this->self + offsets::camera_pos);
	return camera_pos;
}

ethic::roblox::vector3_t ethic::roblox::instance_t::get_part_pos()
{
	vector3_t res{};

	auto primitive = read<std::uint64_t>(this->self + offsets::primitive);

	if (!primitive)
	{
		return res;
	}

	res = read<ethic::roblox::vector3_t>(primitive + offsets::position);
	return res;
}

ethic::roblox::vector3_t ethic::roblox::instance_t::get_part_velocity()
{
	vector3_t res{};

	auto primitive = read<std::uint64_t>(this->self + offsets::primitive);

	if (!primitive)
	{
		return res;
	}

	res = read<ethic::roblox::vector3_t>(primitive + offsets::primitive);
	return res;
}

union convertion
{
	std::uint64_t hex;
	float f;
} conv;

float ethic::roblox::instance_t::get_health()
{
	auto one = read<std::uint64_t>(this->self + offsets::health);
	auto two = read<std::uint64_t>(read<std::uint64_t>(this->self + offsets::health));

	conv.hex = one ^ two;
	return conv.f;
}

float ethic::roblox::instance_t::get_max_health()
{
	auto one = read<std::uint64_t>(this->self + offsets::max_health);
	auto two = read<std::uint64_t>(read<std::uint64_t>(this->self + offsets::max_health));

	conv.hex = one ^ two;
	return conv.f;
}

ethic::roblox::instance_t ethic::roblox::instance_t::get_team()
{
	auto get_team = read<ethic::roblox::instance_t>(this->self + ethic::offsets::team);
	return get_team;
}

std::uintptr_t ethic::roblox::instance_t::get_gameid()
{
	auto gameid = read<std::uintptr_t>(this->self + ethic::offsets::gameid);
	return gameid;
}
