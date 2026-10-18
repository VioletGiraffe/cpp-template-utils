// Not upstream Catch2, but kept beside the header it wraps
// Compiles Catch2's implementation: exactly once per test binary, in catch2_runner or through test_main.hpp

#include "catch_runner.h"

#include "compiler/compiler_warnings_control.h"
#include "utility/interactive_diagnostics.hpp"
#include "utility/power_throttling.hpp"

#define CATCH_CONFIG_RUNNER
DISABLE_COMPILER_WARNINGS
#include "catch.hpp"
RESTORE_COMPILER_WARNINGS

int runCatchSession(int argc, char* argv[], const CatchRunOptions& options)
{
	Catch::Session session;
	return runCatchSession(session, argc, argv, options);
}

int runCatchSession(Catch::Session& session, int argc, char* argv[], const CatchRunOptions& options)
{
	disableInteractiveDiagnostics();
	disablePowerThrottling();

	if (options.randomSeed)
		session.cli(session.cli() | Catch::clara::Opt(*options.randomSeed, "std::random seed")["--std-seed"]("std::random seed"));

	if (const int returnCode = session.applyCommandLine(argc, argv); returnCode != 0)
		return returnCode;

	if (options.beforeRun)
		options.beforeRun();

	return session.run();
}
