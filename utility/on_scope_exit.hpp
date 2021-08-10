#pragma once

#include "macro_utils.h"

#include <utility>

namespace detail {

template <typename Functor>
class OnScopeExitExecutor
{
public:
	explicit OnScopeExitExecutor(Functor&& code) noexcept : _code(std::forward<Functor>(code))
	{}

	~OnScopeExitExecutor() noexcept
	{
		_code();
	}

	OnScopeExitExecutor& operator=(const OnScopeExitExecutor&) = delete;

private:
	const Functor _code;
};

}

#define EXEC_ON_SCOPE_EXIT detail::OnScopeExitExecutor CONCAT_EXPANDED_ARGUMENTS_2(onScopeExit_, __LINE__)
