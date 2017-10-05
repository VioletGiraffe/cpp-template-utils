#pragma once

#include "macro_utils.h"

#include <functional>

namespace detail {

class OnScopeExitExecutor
{
public:
	inline explicit OnScopeExitExecutor(std::function<void()>&& code) : _code(std::move(code))
	{}

	inline ~OnScopeExitExecutor()
	{
		_code();
	}

	OnScopeExitExecutor& operator=(const OnScopeExitExecutor&) = delete;

private:
	const std::function<void()> _code;
};

}

#define EXEC_ON_SCOPE_EXIT detail::OnScopeExitExecutor CONCAT_EXPANDED_ARGUMENTS_2(onScopeExit_, __LINE__)
