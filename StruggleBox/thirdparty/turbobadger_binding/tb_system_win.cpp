// ================================================================================
// ==      This file is a part of Turbo Badger. (C) 2011-2014, Emil Segerås      ==
// ==                     See tb_core.h for more information.                    ==
// ================================================================================
// Edited to use Drudgerists Timer and Log,
// removed mmsystem.lib dependency 
#include "tb_system.h"
#include "Timer.h"
#include "Log.h"
#ifdef TB_SYSTEM_WINDOWS

#include <Windows.h>
//#include <mmsystem.h>
#include <stdio.h>

#ifdef TB_RUNTIME_DEBUG_INFO

void TBDebugOut(const char *str)
{
	Log::Debug("[TurboBadger] %s", str);
}

#endif // TB_RUNTIME_DEBUG_INFO

namespace tb {

// == TBSystem ========================================

double TBSystem::GetTimeMS()
{
	return Timer::Milliseconds();
}

// This is here since the proper implementations has not yet been done.
// Edit: I don't even know what this should do. - The Drudgerist
void TBSystem::RescheduleTimer(double fire_time)
{
	
}

int TBSystem::GetLongClickDelayMS()
{
	return 500;
}

int TBSystem::GetPanThreshold()
{
	return 5 * GetDPI() / 96;
}

int TBSystem::GetPixelsPerLine()
{
	return 40 * GetDPI() / 96;
}

int TBSystem::GetDPI()
{
	HDC hdc = GetDC(nullptr);
	int DPI_x = GetDeviceCaps(hdc, LOGPIXELSX);
	ReleaseDC(nullptr, hdc);
#if 0 // TEST CODE!
	DPI_x *= 2;
#endif
	return DPI_x;
}

} // namespace tb

#endif // TB_SYSTEM_WINDOWS
