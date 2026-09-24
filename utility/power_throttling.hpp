#pragma once

#ifdef _WIN32
#include <Windows.h>
#endif

// Exempts the process from Windows power throttling, which moves an unfocused process to efficiency cores
// Call once at the start of main(); does nothing outside Windows
inline void disablePowerThrottling() noexcept
{
#ifdef _WIN32
	PROCESS_POWER_THROTTLING_STATE throttling{};
	throttling.Version = PROCESS_POWER_THROTTLING_CURRENT_VERSION;
	throttling.ControlMask = PROCESS_POWER_THROTTLING_EXECUTION_SPEED;
	throttling.StateMask = 0; // control bit set with state bit clear means explicitly never throttled
	::SetProcessInformation(::GetCurrentProcess(), ProcessPowerThrottling, &throttling, sizeof(throttling));
#endif
}
