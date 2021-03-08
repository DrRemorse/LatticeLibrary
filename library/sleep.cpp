#include <library/sleep.hpp>

#ifdef __unix__
	#include <unistd.h>
	#define msleep(x) usleep(1000 * x)
#elif defined _WIN32
	#include <windows.h>
	#define msleep(x) Sleep(x)
#endif

namespace library
{
	void sleepMillis(int millis)
	{
		msleep(millis);
	}
	
	void relinquishCPU()
	{
		msleep(1);
	}
}
