#pragma once

// Not upstream Catch2, but kept beside the header it wraps
// Declares the runner without Catch2 itself: catch_runner.cpp holds the implementation, compiled by catch2.pro or CMakeLists.txt into
// catch2_runner, or into one source by test_main.hpp

#include <functional>
#include <stdint.h>

namespace Catch {
	class Session;
}

struct CatchRunOptions {
	// Set: adds the --std-seed option, bound to *randomSeed
	uint32_t* randomSeed = nullptr;
	// Called once the command line is parsed, so the options hold their values
	std::function<void()> beforeRun;
};

// Every runner goes through one of these, so a hand-written main() cannot forget the process setup
[[nodiscard]] int runCatchSession(int argc, char* argv[], const CatchRunOptions& options = {});
// session must be the binary's only Catch::Session, with any extra command line options already added to its cli()
[[nodiscard]] int runCatchSession(Catch::Session& session, int argc, char* argv[], const CatchRunOptions& options = {});
