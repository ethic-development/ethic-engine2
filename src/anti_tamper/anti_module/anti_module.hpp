#pragma once
#include <Windows.h>
#include <iostream>
#include <winternl.h>

#include <utils/singleton/singleton.hpp>

class anti_module : public singleton<anti_module>
{
public:
	void initiate();
	void setup_process_mitigations();
};