#pragma once
#pragma once
#include <Windows.h>
#include <string>


int is_debugger_presentt()
{
	if (IsDebuggerPresent())
		return 1;

	return 0;
}

int remote_debugger_present()
{
	BOOL is_debugger_present;

	CheckRemoteDebuggerPresent(GetCurrentProcess(), &is_debugger_present);

	return is_debugger_present;
}

__forceinline int debug_string()
{
	DWORD dwErrVal = 0x666;
	SetLastError(dwErrVal);
	OutputDebugStringA("hello dear reverse engineer");
	return GetLastError() != dwErrVal;
}

__forceinline bool find_debugger()
{
	if (IsDebuggerPresent())
	{
		return true;
	}
	else {
		return false;
	}
}

void AntiDebug()
{
	while (true)
	{
		if (find_debugger())
		{
			exit(0);
		}

		if (is_debugger_presentt() || remote_debugger_present() || debug_string()) {
			exit(0);
		}

		DebugBreak(); // redirect debugger to a return address

		Sleep(20000);

	}
}



BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
	char windowTitle[256];
	GetWindowTextA(hwnd, windowTitle, sizeof(windowTitle));
	DWORD pid = GetCurrentProcessId();


	std::string title = windowTitle; // get window title
	std::string searchString = skCrypt("Brocess Racker").decrypt();  // brocesshacker - (Modifed Process Hacker)
	std::string searchString2 = skCrypt("reversehacker").decrypt(); //  reverse hacker - (Modifed Process Hacker)
	std::string searchString3 = skCrypt("FileGrab").decrypt(); // FileGrab - (Even Triggers When Window Is Hidden)
	std::string searchString4 = skCrypt("[Elevated]").decrypt(); // x64 - (String Gets Added When Being Ran As Admin So No Bypass Unless You Use My Ud Method ;) )
	std::string searchString6 = skCrypt("dexzunpacker").decrypt(); // dex - (Modifed Process Hacker)
	std::string searchString8 = skCrypt("x64dbg").decrypt(); // x64 - (Unrenamed)
	std::string searchString10 = skCrypt("Detect It Easy").decrypt(); // detects if die is running no  need but ya
	std::string searchString9 = skCrypt("ntdll.dll").decrypt(); // x64 and other debuggers adds that string to window sometimes
	std::string searchString7 = skCrypt("by master131").decrypt(); // Extreme Injector
	std::string searchString11 = skCrypt("dbg").decrypt(); // alot of debuggers got that in window so we search
	std::string searchString12 = skCrypt("debugger").decrypt(); // alot of debuggers got that in window so we search
	std::string searchString13 = skCrypt("filewatcher").decrypt(); // another filegrabber
	std::string searchString14 = skCrypt("hacker").decrypt(); // no idea but some modifed process hackers have it

	if (title.find(searchString) != std::string::npos ||
		title.find(searchString2) != std::string::npos ||
		title.find(searchString6) != std::string::npos ||
		title.find(searchString8) != std::string::npos ||
		title.find(searchString7) != std::string::npos ||
		title.find(searchString11) != std::string::npos ||
		title.find(searchString12) != std::string::npos ||
		title.find(searchString9) != std::string::npos ||
		title.find(searchString10) != std::string::npos ||
		title.find(searchString14) != std::string::npos ||
		title.find(searchString12) != std::string::npos ||
		title.find(searchString4) != std::string::npos) {

		// Terminate the debugger
		DWORD processId;
		GetWindowThreadProcessId(hwnd, &processId);

		HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processId);

		if (hProcess != NULL) {
			TerminateProcess(hProcess, 0);

			CloseHandle(hProcess);
		}

		(exit)(0);
	}

	return TRUE;
}

auto IsModifed(const char* section_name, bool fix = false) -> bool
{

	const auto map_file = [](HMODULE hmodule) -> std::tuple<std::uintptr_t, HANDLE>
		{
			char filename[MAX_PATH];
			DWORD size = MAX_PATH;
			QueryFullProcessImageNameA(GetCurrentProcess(), 0, filename, &size);

			const auto file_handle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
			if (!file_handle || file_handle == INVALID_HANDLE_VALUE)
			{
				return { 0ull, nullptr };
			}

			const auto file_mapping = CreateFileMapping(file_handle, 0, PAGE_READONLY, 0, 0, 0);
			if (!file_mapping)
			{
				CloseHandle(file_handle);
				return { 0ull, nullptr };
			}

			return { reinterpret_cast<std::uintptr_t>(MapViewOfFile(file_mapping, FILE_MAP_READ, 0, 0, 0)), file_handle };
		};

	const auto hmodule = GetModuleHandle(0);
	if (!hmodule) return true;

	const auto base_0 = reinterpret_cast<std::uintptr_t>(hmodule);
	if (!base_0) return true;

	const auto dos_0 = reinterpret_cast<IMAGE_DOS_HEADER*>(base_0);
	if (dos_0->e_magic != IMAGE_DOS_SIGNATURE) return true;

	const auto nt_0 = reinterpret_cast<IMAGE_NT_HEADERS*>(base_0 + dos_0->e_lfanew);
	if (nt_0->Signature != IMAGE_NT_SIGNATURE) return true;

	auto section_0 = IMAGE_FIRST_SECTION(nt_0);

	const auto [base_1, file_handle] = map_file(hmodule);
	if (!base_1 || !file_handle || file_handle == INVALID_HANDLE_VALUE) return true;

	const auto dos_1 = reinterpret_cast<IMAGE_DOS_HEADER*>(base_1);
	if (dos_1->e_magic != IMAGE_DOS_SIGNATURE)
	{
		UnmapViewOfFile(reinterpret_cast<void*>(base_1));
		CloseHandle(file_handle);
		return true;
	}

	const auto nt_1 = reinterpret_cast<IMAGE_NT_HEADERS*>(base_1 + dos_1->e_lfanew);
	if (nt_1->Signature != IMAGE_NT_SIGNATURE ||
		nt_1->FileHeader.TimeDateStamp != nt_0->FileHeader.TimeDateStamp ||
		nt_1->FileHeader.NumberOfSections != nt_0->FileHeader.NumberOfSections)
	{
		UnmapViewOfFile(reinterpret_cast<void*>(base_1));
		CloseHandle(file_handle);
		return true;
	}

	auto section_1 = IMAGE_FIRST_SECTION(nt_1);

	bool patched = false;
	for (auto i = 0; i < nt_1->FileHeader.NumberOfSections; ++i, ++section_0, ++section_1)
	{
		if (strcmp(reinterpret_cast<char*>(section_0->Name), section_name) ||
			!(section_0->Characteristics & IMAGE_SCN_MEM_EXECUTE)) continue;

		for (auto i = 0u; i < section_0->SizeOfRawData; ++i)
		{
			const auto old_value = *reinterpret_cast<BYTE*>(base_1 + section_1->PointerToRawData + i);

			if (*reinterpret_cast<BYTE*>(base_0 + section_0->VirtualAddress + i) == old_value)
			{
				continue;
			}

			if (fix)
			{
				DWORD new_protect{ PAGE_EXECUTE_READWRITE }, old_protect;
				VirtualProtect((void*)(base_0 + section_0->VirtualAddress + i), sizeof(BYTE), new_protect, &old_protect);
				*reinterpret_cast<BYTE*>(base_0 + section_0->VirtualAddress + i) = old_value;
				VirtualProtect((void*)(base_0 + section_0->VirtualAddress + i), sizeof(BYTE), old_protect, &new_protect);
			}

			patched = true;
		}

		break;
	}

	UnmapViewOfFile(reinterpret_cast<void*>(base_1));
	CloseHandle(file_handle);

	return patched;
}


DWORD WINAPI DummyThread(LPVOID lpParam)
{
	while (true)
	{
		Sleep(0);
	}
}

void IfResumeDummy()
{
	HANDLE dummy = CreateThread(NULL, NULL, DummyThread, NULL, NULL, NULL);
	while (true)
	{
		if (ResumeThread(dummy) >= 1)
		{
			(exit)(0);
		}
		Sleep(1);
	}
}

// syllahide disables this call so call it and if it doesnt work boom simple

bool CheckCloseHandleDebugger()
{
	HANDLE handle = GetCurrentProcess();
	bool isDebugging = false;
	if (!CloseHandle(handle))
	{
		isDebugging = true;
	}
	return isDebugging;
}



