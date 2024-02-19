#pragma once
#include <Windows.h>
#include <iostream>
#include <thread>
#include <intrin.h>

#include <utils/singleton/singleton.hpp>
#include <utils/skcrypt/skStr.hpp>

class anti_debug : public singleton<anti_debug>
{
private:
	bool check_debugger_presence = false;
public:
	bool context_thread();
	bool cpu_known_vm_vendors();
	bool is_debugger_present();
	bool is_remote_debugger_present();
	bool is_section_modified(const char* section_name, bool fix = false);
	bool check_close_handle_debugger();
	void if_resume_dummy();

	void initiate();
};