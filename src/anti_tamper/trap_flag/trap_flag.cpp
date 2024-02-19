#include "trap_flag.hpp"

void trap_flag::set()
{
#ifdef _M_IX86
    __try
    {
        __asm {
            pushfd;
            or word ptr[esp], 0x100;
            popfd;
            nop;
        }
    }
    __except (1) {}
#else
	return;
#endif
}