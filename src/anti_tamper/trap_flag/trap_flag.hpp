#pragma once
#include <utils/singleton/singleton.hpp>

class trap_flag : public singleton<trap_flag>
{
public:
	void set();
};