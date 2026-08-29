#include "compiler/compiler_warnings_control.h"

#define CATCH_CONFIG_RUNNER
DISABLE_COMPILER_WARNINGS
#include "3rdparty/catch2/catch.hpp"
RESTORE_COMPILER_WARNINGS

#include <stdio.h>

int main(int argc, char* argv[])
{
#ifdef _MSC_FULL_VER
	(void)::printf("%d\n", _MSC_FULL_VER);
#endif

	const int result = Catch::Session().run(argc, argv);
	return result;
}