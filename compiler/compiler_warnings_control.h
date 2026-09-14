#pragma once

#define UNUSED_VARIABLE(x) (void)(x)

#if defined _MSC_VER

#define COMPILER_PRAGMA(text) __pragma(text)
#define STORE_COMPILER_WARNINGS COMPILER_PRAGMA(warning(push))
#define RESTORE_COMPILER_WARNINGS COMPILER_PRAGMA(warning(pop))
#define DISABLE_SPECIFIC_COMPILER_WARNING(warningCode) COMPILER_PRAGMA(warning (disable: warningCode))
#define DISABLE_MSVC_WARNING(warningCode) DISABLE_SPECIFIC_COMPILER_WARNING(warningCode)
#define DISABLE_CLANG_GCC_WARNING(warning)
#define DISABLE_GCC_WARNING(warning)

#elif defined __clang__

#define COMPILER_PRAGMA(text) _Pragma(#text) // Stringifying the text to wrap it into quotes
#define STORE_COMPILER_WARNINGS COMPILER_PRAGMA(clang diagnostic push)
#define RESTORE_COMPILER_WARNINGS COMPILER_PRAGMA(clang diagnostic pop)
#define DISABLE_SPECIFIC_COMPILER_WARNING(warning) COMPILER_PRAGMA(clang diagnostic ignored warning)
#define DISABLE_MSVC_WARNING(warningCode)
#define DISABLE_CLANG_GCC_WARNING(warning) DISABLE_SPECIFIC_COMPILER_WARNING(warning)
#define DISABLE_GCC_WARNING(warning)

#elif defined __GNUC__ || defined __GNUG__

#define COMPILER_PRAGMA(text) _Pragma(#text) // Stringifying the text to wrap it into quotes
#define STORE_COMPILER_WARNINGS COMPILER_PRAGMA(GCC diagnostic push)
#define RESTORE_COMPILER_WARNINGS COMPILER_PRAGMA(GCC diagnostic pop)
#define DISABLE_SPECIFIC_COMPILER_WARNING(warning) COMPILER_PRAGMA(GCC diagnostic ignored warning)
#define DISABLE_MSVC_WARNING(warningCode)
#define DISABLE_CLANG_GCC_WARNING(warning) DISABLE_SPECIFIC_COMPILER_WARNING(warning)
#define DISABLE_GCC_WARNING(warning) DISABLE_SPECIFIC_COMPILER_WARNING(warning)

#else

#define COMPILER_PRAGMA(text)
#define DISABLE_SPECIFIC_COMPILER_WARNING(warning)
#define DISABLE_MSVC_WARNING(warningCode)
#define STORE_COMPILER_WARNINGS
#define RESTORE_COMPILER_WARNINGS
#define DISABLE_CLANG_GCC_WARNING(warning)
#define DISABLE_GCC_WARNING(warning)

#endif


#if defined _MSC_VER

#define DISABLE_COMPILER_WARNINGS COMPILER_PRAGMA(warning(push, 0)) // Set /W0

#elif defined __clang__

	#define DISABLE_COMPILER_WARNINGS \
	STORE_COMPILER_WARNINGS \
	DISABLE_SPECIFIC_COMPILER_WARNING("-Weverything")

#elif defined __GNUC__ || defined __GNUG__

	#ifdef __cplusplus

	#define DISABLE_COMPILER_WARNINGS \
		STORE_COMPILER_WARNINGS \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wall") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wunknown-pragmas") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wunused-parameter") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wsign-promo") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wconversion") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wdeprecated-enum-enum-conversion") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wcast-qual") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wunused-const-variable") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wimplicit-fallthrough") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wclass-memaccess") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wextra") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wpedantic") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wnon-virtual-dtor") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Woverloaded-virtual") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wdouble-promotion") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wfloat-conversion") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wundef") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wformat") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wformat-nonliteral") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wformat-security") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wformat-y2k") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wextra-semi") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wzero-as-null-pointer-constant") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wfloat-equal") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wredundant-decls") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wvla") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wreturn-type") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wuninitialized") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wmaybe-uninitialized") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wdelete-non-virtual-dtor") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Waddress") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wsizeof-pointer-div") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wsizeof-pointer-memaccess") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wshadow") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wcast-align") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wduplicated-cond") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wduplicated-branches") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wlogical-op") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wnull-dereference") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wsuggest-override") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wnoexcept") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wmissing-declarations") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wmismatched-tags") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wreturn-local-addr") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wmemset-transposed-args") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wnonnull-compare") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wmismatched-new-delete") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Winfinite-recursion") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wcatch-value")

	#else // C only
	#define DISABLE_COMPILER_WARNINGS \
		STORE_COMPILER_WARNINGS \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wall") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wunused-parameter") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wimplicit-fallthrough") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wconversion") \
		DISABLE_SPECIFIC_COMPILER_WARNING("-Wunknown-pragmas")
	#endif
#else

#pragma message ("Unknown compiler")

#define DISABLE_COMPILER_WARNINGS

#endif
