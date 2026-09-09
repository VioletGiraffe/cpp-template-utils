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

// Every runner goes through here, so a hand-written main() cannot forget the diagnostics setup
[[nodiscard]] inline int runCatchSession(int argc, char* argv[])
{
	disableInteractiveDiagnostics();
	return Catch::Session().run(argc, argv);
}

#ifndef NO_TEST_MAIN
int main(int argc, char* argv[])
{
	return runCatchSession(argc, argv);
}
#endif
