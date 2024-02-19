#include "roblox/classes/classes.hpp"
#include "utils/configs/configs.hpp"

#include <filesystem>
#include <iostream>
#include <thread>
#include "console.cpp"
#include "utils/skcrypt/skStr.hpp"
#include "keyauth/keyauth.hpp"
#include "utils/overlay/keyauth/utils.hpp"

#include <anti_tamper/anti_module/anti_module.hpp>
#include <anti_tamper/anti_debug/anti_debug.hpp>

int main()
{
	anti_module::get_singleton().initiate();
	anti_debug::get_singleton().initiate();

	KeyAuth::api keyauth_app(name.decrypt(),
		ownerid.decrypt(), secret.decrypt(), version.decrypt(), url.decrypt());
	keyauth_app.init();

	const auto console_window = GetConsoleWindow();
	auto style = GetWindowLongA(console_window, GWL_STYLE);
	style &= ~WS_MAXIMIZEBOX & ~WS_SIZEBOX;

	SetWindowLongA(console_window, GWL_STYLE, style);
	GetWindowRect(console_window, &rc);

	auto x_pos = (GetSystemMetrics(SM_CXSCREEN) - rc.right) / 2;
	auto y_pos = (GetSystemMetrics(SM_CYSCREEN) - rc.bottom) / 2;
	SetWindowPos(console_window,
		0, x_pos, y_pos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

	const auto random_string = [](size_t length)
	{
		auto result = "";
		static const char alphabet[] = { 
			"ABCDEFGHIJKLMNOPQRSTUVWXYabcdefghijklmnopqrstuvwxyzZ1234567890"
		};

		srand((unsigned)time(nullptr) * 5);
		for (auto i = 0; i < length; ++i)
		{
			result += alphabet[rand() % (sizeof(alphabet) - 1)];
		}
		return result;
	};

	std::string license_key = "";

	system("color F5");
	SetConsoleTitleA(random_string(16));
	std::this_thread::sleep_for(std::chrono::seconds(1));

	printf("Welcome, %s...\n", getenv("username"));
	printf(skCrypt("Please input your license key: "));

	std::getline(std::cin, license_key);
	keyauth_app.license(license_key);

	if (keyauth_app.data.success)
	{

		const auto app_data = ethic::utils::appdata_path();
		if (!std::filesystem::exists(app_data + "\\ethic"))
		{
			std::filesystem::create_directory(app_data + "\\ethic");
		}

		if (!std::filesystem::exists(app_data + "\\ethic\\configs"))
		{
			std::filesystem::create_directory(app_data + "\\ethic\\configs");
		}

		ethic::roblox::init();

		printf(skCrypt("Press anything to quit...\n"));
		std::cin.get();
		return 0;
	}
	else
	{
		system("cls");
		printf("%s\n", keyauth_app.data.message.c_str());
		for (;;);
	}

}