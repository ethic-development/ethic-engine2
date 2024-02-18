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

// added include path in project settings - mogus
#include <curl/curl.h>
#include <Wininet.h>
#pragma comment(lib, "wininet.lib")

#pragma comment(lib, "libcurl.lib")
#include <fstream>
#include <thread>

//below gpt to get roblox ver

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
	size_t totalSize = size * nmemb;
	output->append(static_cast<char*>(contents), totalSize);
	return totalSize;
}
/*

std::string GetRobloxVersion() {
	CURL* curl;
	CURLcode res;

	// Initialize libcurl
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();

	std::string response;

	if (curl) {
		// Set the URL
		curl_easy_setopt(curl, CURLOPT_URL, XorStr("https://clientsettingscdn.roblox.com/v2/client-version/WindowsPlayer"));

		// Set the callback function to handle the response
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

		// Perform the request
		res = curl_easy_perform(curl);

		// Check for errors
		if (res != CURLE_OK) {
			fprintf(stderr, XorStr("curl_easy_perform() failed: %s\n"), curl_easy_strerror(res));
		}

		// Cleanup
		curl_easy_cleanup(curl);
	}

	// Cleanup libcurl
	curl_global_cleanup();

	if (res != CURLE_OK) {
		return "";
	}

	// Parse JSON response
	auto json = nlohmann::json::parse(response);
	return json["clientVersionUpload"];
}

bool IsVersionUpToDate() {
	// Hardcoded Roblox version
	std::string githubVersion = XorStr("version-48a28da848b7420d");

	// Get the Roblox version
	std::string robloxVersion = GetRobloxVersion();

	// Compare Roblox versions
	if (robloxVersion != githubVersion) {
		return false;
	}

	return true;
}
*/
// above all gpt

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
	auto render_view = get_render_view();
	auto visualengine =
		read<std::uint64_t>(render_view + 0x10);

	return visualengine;
}


std::string replaceAll(std::string subject, const std::string& search,
	const std::string& replace) {
	size_t pos = 0;
	while ((pos = subject.find(search, pos)) != std::string::npos) {
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
		std::perror(XorStr("[ethic]: invalid driver buffer size\n"));
		return -1;
	}

	physmeme::drv_image image(drv_buffer);
	if (!physmeme::load_drv())
	{
		std::perror(XorStr("[ethic]: failed to load driver\n"));
		return -1;
	}

	physmeme::kernel_ctx kernel_ctx;
	//std::printf("[+] driver has been loaded...\n");
	//std::printf("[+] %s mapped physical page -> 0x%p\n", physmeme::syscall_hook.first, physmeme::psyscall_func.load());
	//std::printf("[+] %s page offset -> 0x%x\n", physmeme::syscall_hook.first, physmeme::nt_page_offset);
	const auto drv_timestamp = util::get_file_header((void*)raw_driver)->TimeDateStamp;
	if (!kernel_ctx.clear_piddb_cache(physmeme::drv_key, drv_timestamp))
	{
		// this is because the signature might be broken on these versions of windows.
		perror(XorStr("[ethic]: couldn't clear PiDDB cache table\n"));
		return -1;
	}
	const auto _get_export_name = [&](const char* base, const char* name)
		{
			return reinterpret_cast<std::uintptr_t>(util::get_kernel_export(base, name));
		};

	image.fix_imports(_get_export_name);
	image.map();

	const auto pool_base = kernel_ctx.allocate_pool(image.size(), NonPagedPool);
	image.relocate(pool_base);
	kernel_ctx.write_kernel(pool_base, image.data(), image.size());
	auto entry_point = reinterpret_cast<std::uintptr_t>(pool_base) + image.entry_point();

	auto result = kernel_ctx.syscall<DRIVER_INITIALIZE>
		(
			reinterpret_cast<void*>(entry_point),
			reinterpret_cast<std::uintptr_t>(pool_base),
			image.size()
		);
	std::printf(XorStr("[ethic]: driver callback: 0x%p\n\n"), result);
	Sleep(1500);

	kernel_ctx.zero_kernel_memory(pool_base, image.header_size());
	if (!physmeme::unload_drv())
	{
		std::perror(XorStr("[ethic]: unable to load driver\n"));
		return -1;
	}

	printf(XorStr("[ethic]: connecting to server\n"));

	// NOTE: This loader check is messy as hell + not working properly.

	//std::string LoaderVersion = DownloadString("https://pastebin.com/raw/PdnnqEX9"); //loader version
	//if (LoaderVersion == "1.0.1") // 
	//{
	//	//loader version ok
	//}
	//else if (LoaderVersion == "0") //makes the loader offline

	//{
	//	std::cout << XorStr("\n    Failure: INFO: Loader is offline!");
	//	int msgboxIDa = MessageBoxA(0, "Loader is offline!", "Failure", MB_ICONWARNING | MB_OK);
	//	switch (msgboxIDa)
	//	{
	//	case IDOK:
	//	{
	//		exit(0);
	//	}
	//	}
	//}
	//else
	//{
	//	std::cout << XorStr("\n    Failure: INFO: Loader is outdated!");
	//	int msgboxID2 = MessageBoxA(0, "A new Version of the Loader is avaible. \n\n\n Do you want to get redirected to the new Version?", "Update", MB_ICONWARNING | MB_YESNO);
	//	switch (msgboxID2)
	//	{
	//	case IDYES:
	//	{
	//		system("cls");
	//		exit(0);
	//	}

	//	case IDNO:
	//	{
	//		exit(0);
	//	}
	//	}
	//}

	Sleep(500);
	system("cls");
	if (!mem::find_driver())
	{
		printf(XorStr("[ethic]: couldn't load driver\n"));
		return false;
	}

	mem::process_id = mem::find_process(XorStr("RobloxPlayerBeta.exe"));
	if (!mem::process_id)
	{
		printf(XorStr("[ethic]: roblox not found\n"));
		for (;;);
		return false;
	}

	//virtualaddy = mem::find_image();
	//ethic::utils::log::debug_log(virtualaddy, XorStr("image base"));

	auto visualengine = static_cast<ethic::roblox::instance_t>(get_visualengine_address());

	//std::cout << std::hex << get_render_view() << std::endl;
	// fix thanks to draculeaa for telling me
	auto game_ptr = read<std::uint64_t>(get_render_view() + 0xD8);
    auto game = static_cast<ethic::roblox::instance_t>(read<std::uint64_t>(game_ptr + 0x150));

	if (!visualengine.self)
	{
		MessageBox(NULL, XorStr("failed to get visual engine, please run the cheat as administrator!"), XorStr("ethic"), MB_OK | MB_ICONERROR);
	}

	//auto game =
		//static_cast<ethic::roblox::instance_t>(ethic::utils::datamodel::get_game_address());

	if (!game.self)
	{
		MessageBox(NULL, XorStr("failed to get datamodel, please run the cheat as administrator!"), XorStr("ethic"), MB_OK | MB_ICONERROR);
	}

	globals::datamodel = game;

	auto players =
		game.find_first_child(XorStr("Players"));

	if (!players.self)
	{
		MessageBox(NULL, XorStr("failed to get players, please run the cheat as administrator!"), XorStr("ethic"), MB_OK | MB_ICONERROR);
	}

	globals::players = players;

	

	globals::visualengine = visualengine;

	std::string teamname;

	ethic::roblox::instance_t team = players.get_local_player().get_team();

	if (team.self)
	{
		teamname = team.name();
	}
	else
	{
		teamname = XorStr("none");
	}
	auto placeid =
		static_cast<ethic::roblox::instance_t>(ethic::utils::datamodel::get_place_id());

	if (!placeid.self)
		return false;

	globals::placeid = placeid;


	//std::cout << globals::placeid.self << std::endl;

    //float walkspeed = players.get_local_player().get_model_instance().find_first_child("Humanoid").get_walkspeed();
	//printf("%f\n", walkspeed);

	//printf("%s\n", teamname);

	//ethic::utils::log::debug_log(globals::visualengine.self, XorStr("visualengine"));

	//auto workspace =
	//	game.find_first_child("Workspace");

	//if (!workspace.self)
	//	return false;

	//ethic::utils::log::debug_log(workspace.self, "workspace");

	//float health = players.get_local_player().get_model_instance().find_first_child("Humanoid").get_health();
	//printf("%f\n", health);

	//ethic::roblox::instance_t team = players.get_local_player().get_team();
	//printf("%s\n", team.name());

	std::thread(ethic::roblox::hook_aimbot).detach();
	std::thread(ethic::utils::overlay::render).detach();

	printf(XorStr("[ethic]: hiding console in 3 seconds\n"));
	std::this_thread::sleep_for(std::chrono::seconds(3));

	ShowWindow(GetConsoleWindow(), SW_HIDE);

	/*while (true)
	{
		float ping = players.get_local_player().get_ping();
		printf("ping : %f\n", ping);
	}*/

	while (true)
	{
		std::this_thread::sleep_for(std::chrono::seconds(10));

		if (FindWindowA(NULL, XorStr("Roblox")) == NULL) {
			physmeme::unload_drv();
			exit(0);
		}
	}

	return true;
}

ethic::roblox::vector2_t ethic::roblox::world_to_screen(ethic::roblox::vector3_t world)
{
	auto dimensions = globals::visualengine.get_dimensions();
	auto viewmatrix = globals::visualengine.get_view_matrix();


	ethic::roblox::quaternion quaternion;
	quaternion.x = (world.x * viewmatrix.data[0]) + (world.y * viewmatrix.data[1]) + (world.z * viewmatrix.data[2]) + viewmatrix.data[3];
	quaternion.y = (world.x * viewmatrix.data[4]) + (world.y * viewmatrix.data[5]) + (world.z * viewmatrix.data[6]) + viewmatrix.data[7];
	quaternion.z = (world.x * viewmatrix.data[8]) + (world.y * viewmatrix.data[9]) + (world.z * viewmatrix.data[10]) + viewmatrix.data[11];
	quaternion.w = (world.x * viewmatrix.data[12]) + (world.y * viewmatrix.data[13]) + (world.z * viewmatrix.data[14]) + viewmatrix.data[15];

	if (quaternion.w < 0.1f)
		return{ -1, -1 };

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
	std::string string;
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
		const auto Name = readstring(string);
		return Name;
	}
}

std::string ethic::roblox::instance_t::name()
{
	const auto ptr = read<std::uint64_t>(this->self + ethic::offsets::name);

	if (ptr)
		return readstring2(ptr);

	return XorStr("???");
}

std::string ethic::roblox::instance_t::class_name()
{
	const auto ptr = read<std::uint64_t>(this->self + ethic::offsets::classname);

	if (ptr)
		return readstring2(ptr + 0x8);

	return XorStr("???_classname");
}

std::vector<ethic::roblox::instance_t> ethic::roblox::instance_t::children()
{
	std::vector<ethic::roblox::instance_t> container;

	if (!this->self)
		return container;

	auto start = read<std::uint64_t>(this->self + ethic::offsets::children);

	if (!start)
		return container;

	auto end = read<std::uint64_t>(start + offsets::size);

	for (auto instances = read<std::uint64_t>(start); instances != end; instances += 16)
		container.emplace_back(read<ethic::roblox::instance_t>(instances));

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

	if (humanoid_instance.self) {
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
		return res;

	res = read<ethic::roblox::vector3_t>(primitive + offsets::position);
	return res;
}

ethic::roblox::vector3_t ethic::roblox::instance_t::get_part_velocity()
{
	vector3_t res{};

	auto primitive = read<std::uint64_t>(this->self + offsets::primitive);

	if (!primitive)
		return res;

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
	auto getteam = read<ethic::roblox::instance_t>(this->self + ethic::offsets::team);
	return getteam;
}

std::uintptr_t ethic::roblox::instance_t::get_gameid()
{
	auto gameid = read<std::uintptr_t>(this->self + ethic::offsets::gameid);
	return gameid;
}
