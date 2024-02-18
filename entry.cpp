#include "roblox/classes/classes.hpp"
#include "utils/configs/configs.hpp"

#include <filesystem>
#include <iostream>
#include <thread>
#include "console.cpp"
#include "utils/skcrypt/skStr.hpp"
#include "keyauth/keyauth.hpp"
#include "utils/overlay/keyauth/utils.hpp"

#include "simple antidebug.h"
//

std::string RandomString(const size_t length)
{
	std::string r;
	static const char bet[] = { "ABCDEFGHIJKLMNOPQRSTUVWXYabcdefghijklmnopqrstuvwxyzZ1234567890" };
	srand((unsigned)time(NULL) * 5);
	for (int i = 0; i < length; ++i)
		r += bet[rand() % (sizeof(bet) - 1)];
	return r;
}
std::string path()  /// path of exe
{
	char shitter[_MAX_PATH];
	GetModuleFileNameA(NULL, shitter, _MAX_PATH);
	return std::string(shitter);
}

KeyAuth::api KeyAuthApp(name.decrypt(), ownerid.decrypt(), secret.decrypt(), version.decrypt(), url.decrypt());


auto main() -> int
{
	EnumWindows(EnumWindowsProc, 0);

	if (IsModifed(skCrypt(".text").decrypt(), false))
	{
		(exit)(0);
	}

	if (CheckCloseHandleDebugger())
	{
		(exit)(0);
	}


	// load anti attach thread

	std::thread(IfResumeDummy).detach();

	KeyAuthApp.init();


	HWND hwnd = GetConsoleWindow();
	DWORD style = GetWindowLong(hwnd, GWL_STYLE);
	style &= ~WS_MAXIMIZEBOX & ~WS_SIZEBOX;
	SetWindowLong(hwnd, GWL_STYLE, style);
	GetWindowRect(hwnd, &rc);
	int xPos = (GetSystemMetrics(SM_CXSCREEN) - rc.right) / 2;
	int yPos = (GetSystemMetrics(SM_CYSCREEN) - rc.bottom) / 2;
	SetWindowPos(hwnd, 0, xPos, yPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

	system("color F5");
	std::string name = RandomString(16) + ".exe";
	SetConsoleTitleA(RandomString(16).c_str());
	system("cls");
	Sleep(1000);
	system("cls");
	SetConsoleTitleA(RandomString(16).c_str());

	std::string allp;
	std::string username;
	std::string password;

	std::cout << (skCrypt("[ethic]: welcome dear customer.")) << std::endl;
	std::chrono::milliseconds dura(2000);

	//if (std::filesystem::exists("license.json")) //change test.txt to the path of your file :smile:
	//{
	//	if (!CheckIfJsonKeyExists("license.json", "username"))
	//	{
	//		std::string key = ReadFromJson("license.json", "license");
	//		KeyAuthApp.license(key);
	//		if (!KeyAuthApp.data.success)
	//		{
	//			std::remove("license.json");
	//			std::cout << (skCrypt("\n[ethic]: ")) << KeyAuthApp.data.message;
	//			Sleep(1500);
	//			exit(1);
	//		}
	//		std::cout << (skCrypt("[ethic]: automatically logged in! "));
	//	}

	std::cout << (skCrypt("[ethic]: please input your license: "));
	std::cin >> allp;
	KeyAuthApp.license(allp);

	if (KeyAuthApp.data.success) {


		/*if (username.empty() || password.empty())
		{
			WriteToJson("test.json", "license", allp, false, "", "");
			std::cout << (skCrypt("[ethic]: auto-login enabled"));
		}*/

		std::string appdata = ethic::utils::appdata_path();
		if (!std::filesystem::exists(appdata + "\\ethic"))
		{
			std::filesystem::create_directory(appdata + "\\ethic");
		}

		if (!std::filesystem::exists(appdata + "\\ethic\\configs"))
		{
			std::filesystem::create_directory(appdata + "\\ethic\\configs");
		}

		ethic::roblox::init();

		printf(skCrypt("[ethic]: press anything to quit"));
		std::cin.get();

		//system("pause");

	}
	else
	{
		system("cls");
		std::cout << (("[ethic]: " + KeyAuthApp.data.message)) << std::endl;
		for (;;);
	}

}
