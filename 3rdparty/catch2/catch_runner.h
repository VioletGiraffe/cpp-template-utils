#pragma once

// Not upstream Catch2, but kept beside the header it wraps
// Declares the runner without Catch2 itself: catch_runner.cpp holds the implementation, compiled by catch2.pro or CMakeLists.txt into
// catch2_runner, or into one source by test_main.hpp

#include <functional>
#include <stdint.h>

struct CatchRunOptions {
	// Set: adds the --std-seed option, bound to *randomSeed
	uint32_t* randomSeed = nullptr;
	// Called once the command line is parsed, so the options hold their values
	std::function<void()> beforeRun;
};

// Every runner goes through here, so a hand-written main() cannot forget the process setup
[[nodiscard]] int runCatchSession(int argc, char* argv[], const CatchRunOptions& options = {});
