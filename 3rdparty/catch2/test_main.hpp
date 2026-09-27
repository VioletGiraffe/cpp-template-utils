#pragma once

// Not upstream Catch2, but kept beside the header it wraps
// Compiles the runner into the including source, for a test binary that does not link catch2_runner: one that does must not
// include this, or Catch2's implementation is defined twice
// Defines main(), so exactly one .cpp per test binary may include it, and that .cpp needs nothing else
// Define NO_TEST_MAIN ahead of the include to suppress that main() and write one around runCatchSession()

#include "catch_runner.cpp"

#ifndef NO_TEST_MAIN
int main(int argc, char* argv[])
{
	return runCatchSession(argc, argv);
}
#endif
