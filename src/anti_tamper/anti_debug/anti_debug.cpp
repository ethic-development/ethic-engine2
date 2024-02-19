#include "anti_debug.hpp"

bool anti_debug::context_thread()
{
	int found = false;
	CONTEXT ctx = { 0 };
	void* h_thread = GetCurrentThread();

	ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
	if (GetThreadContext(h_thread, &ctx))
	{
		if ((ctx.Dr0 != 0x00) || (ctx.Dr1 != 0x00) || (ctx.Dr2 != 0x00) || (ctx.Dr3 != 0x00) || (ctx.Dr6 != 0x00) || (ctx.Dr7 != 0x00))
		{
			return true;
		}
	}

	return false;
}

bool anti_debug::cpu_known_vm_vendors()
{
	int cpu_info[4];
	__cpuid(cpu_info, 1);

	if (!(cpu_info[2] & (1 << 31)))
	{
		return false;
	}

	const auto query_vendor_id_magic{ 0x40000000 };
	constexpr int vendor_id_length{ 13 };
	char hyper_vendor_id[vendor_id_length];

	__cpuid(cpu_info, query_vendor_id_magic);
	memcpy(hyper_vendor_id + 0, &cpu_info[1], 4);
	memcpy(hyper_vendor_id + 4, &cpu_info[2], 4);
	memcpy(hyper_vendor_id + 8, &cpu_info[3], 4);
	hyper_vendor_id[12] = '\0';

	static const char* vendors[] = {
	"KVMKVMKVM\0\0\0", // KVM 
	"Microsoft Hv",    // Microsoft Hyper-V or Windows Virtual PC */
	"VMwareVMware",    // VMware 
	"XenVMMXenVMM",    // Xen 
	"prl hyperv  ",    // Parallels
	"VBoxVBoxVBox"     // VirtualBox 
	};

	for (const auto& vendor : vendors)
	{
		if (!memcmp(vendor, hyper_vendor_id, vendor_id_length))
		{
			return true;
		}
	}

	return false;
}

bool anti_debug::is_debugger_present()
{
	return IsDebuggerPresent() ? true : false;
}

bool anti_debug::is_remote_debugger_present()
{
	BOOL status;
	CheckRemoteDebuggerPresent(GetCurrentProcess(), &status);

	return status;
}

bool anti_debug::is_section_modified(const char* section_name, bool fix)
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

	const auto hmodule = GetModuleHandleA(nullptr);
	if (!hmodule)
	{
		return true;
	}

	const auto base_0 = reinterpret_cast<std::uintptr_t>(hmodule);
	if (!base_0)
	{
		return true;
	}

	const auto dos_0 = reinterpret_cast<IMAGE_DOS_HEADER*>(base_0);
	if (dos_0->e_magic != IMAGE_DOS_SIGNATURE)
	{
		return true;
	}

	const auto nt_0 = reinterpret_cast<IMAGE_NT_HEADERS*>(base_0 + dos_0->e_lfanew);
	if (nt_0->Signature != IMAGE_NT_SIGNATURE)
	{
		return true;
	}

	auto section_0 = IMAGE_FIRST_SECTION(nt_0);

	const auto [base_1, file_handle] = map_file(hmodule);
	if (!base_1 || !file_handle || file_handle == INVALID_HANDLE_VALUE)
	{
		return true;
	}

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

bool anti_debug::check_close_handle_debugger()
{
	return !CloseHandle(GetCurrentProcess()) ? true : false;
}

DWORD dummy_thread(LPVOID lParam)
{
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(0));
	}
}

void anti_debug::if_resume_dummy()
{
	auto dummy = CreateThread(NULL, NULL, dummy_thread, NULL, NULL, NULL);
	while (true)
	{
		if (ResumeThread(dummy) >= 1)
		{
			exit(0);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

BOOL __stdcall enum_windows_proc(HWND hwnd, LPARAM lParam)
{
	const auto process_id = GetCurrentProcessId();
	char wt[256];
	GetWindowTextA(hwnd, wt, sizeof(wt));

	std::string window_title = wt;
	const char* blacklisted_strings[15] = {
		"BrocessHacker",
		"reversehacker",
		"FileGrab",
		"[Elevated]",
		"dexzunpacker",
		"x64dbg",
		"Detect It Easy",
		"ntdll.dll",
		"by master131",
		"dbg",
		"debugger",
		"filewatcher",
		"hacker"
	};

	for (const char* string : blacklisted_strings)
	{
		if (window_title.find(string) != std::string::npos)
		{
			DWORD process_id = 0;
			GetWindowThreadProcessId(hwnd, &process_id);

			auto process = OpenProcess(PROCESS_TERMINATE, FALSE, process_id);
			if (process != nullptr)
			{
				TerminateProcess(process, 0);
				CloseHandle(process);
			}

			exit(0);
		}
	}

	return TRUE;
}

void anti_debug::initiate()
{
	EnumWindows(enum_windows_proc, 0);
	if (this->is_section_modified(skCrypt(".text").decrypt(), false))
	{
		exit(0);
	}
	
	this->context_thread();
	this->cpu_known_vm_vendors();

	if (this->check_close_handle_debugger())
	{
		MessageBoxA(0, "ERROR CODE: 255", "Ethic", 0);
		exit(0);
	}

	if (this->cpu_known_vm_vendors())
	{
		MessageBoxA(0, "ERROR CODE: 407", "Ethic", 0);
		exit(0);
	}

	if (this->check_debugger_presence)
	{
		while (true)
		{
			if (is_debugger_present() or is_remote_debugger_present())
			{
				exit(0);
			}

			DebugBreak();
			std::this_thread::sleep_for(std::chrono::seconds(20));
		}
	}
}