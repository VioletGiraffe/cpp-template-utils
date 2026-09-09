#pragma once

#ifdef _MSC_VER
#include <crtdbg.h>
#include <initializer_list>
#include <stdlib.h>
#endif

// Sends assertion failures and aborts to stderr instead of a modal dialog
// An unattended run has nobody to dismiss a dialog, so one hangs the job instead of failing it
// Call once at the start of main(); does nothing outside MSVC, where there is no dialog to suppress
inline void disableInteractiveDiagnostics() noexcept
{
#ifdef _MSC_VER
	// Only _CALL_REPORTFAULT is cleared: the abort still writes its message, it just skips the error reporting dialog
	_set_abort_behavior(0, _CALL_REPORTFAULT);

#ifdef _DEBUG
	// One channel carries asserts and the debug STL's own bounds and iterator checks alike
	for (const int reportType : { _CRT_WARN, _CRT_ERROR, _CRT_ASSERT }) {
		_CrtSetReportMode(reportType, _CRTDBG_MODE_FILE);
		_CrtSetReportFile(reportType, _CRTDBG_FILE_STDERR);
	}
#endif
#endif
}
