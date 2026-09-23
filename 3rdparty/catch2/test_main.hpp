#pragma once

// Not upstream Catch2, but kept beside the header it wraps
// Defines main(), so exactly one .cpp per test binary may include it, and that .cpp needs nothing else
// Define NO_TEST_MAIN ahead of the include to suppress that main() and write one around runCatchSession()

#include "compiler/compiler_warnings_control.h"
#include "utility/interactive_diagnostics.hpp"

#define CATCH_CONFIG_RUNNER
DISABLE_COMPILER_WARNINGS
#include "3rdparty/catch2/catch.hpp"
RESTORE_COMPILER_WARNINGS

#include <functional>

// Every runner goes through here, so a hand-written main() cannot forget the diagnostics setup
// session: the one instance, for a main() that adds command line options to it
// beforeRun: called once the command line is parsed, so those options hold their values
[[nodiscard]] inline int runCatchSession(Catch::Session& session, int argc, char* argv[], const std::function<void()>& beforeRun = {})
{
	disableInteractiveDiagnostics();
	if (const int returnCode = session.applyCommandLine(argc, argv); returnCode != 0)
		return returnCode;

	if (beforeRun)
		beforeRun();

	return session.run();
}

[[nodiscard]] inline int runCatchSession(int argc, char* argv[])
{
	Catch::Session session;
	return runCatchSession(session, argc, argv);
}

#ifndef NO_TEST_MAIN
int main(int argc, char* argv[])
{
	return runCatchSession(argc, argv);
}
#endif
