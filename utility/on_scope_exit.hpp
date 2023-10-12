#pragma once

#include "macro_utils.h"
#include "../lang/utils.hpp"

#include <utility>

namespace detail {

template <typename Functor>
class OnScopeExitExecutor
{
public:
	NON_MOVABLE(OnScopeExitExecutor);

	explicit OnScopeExitExecutor(Functor&& code) noexcept : _code(std::forward<Functor>(code))
	{}

	~OnScopeExitExecutor() noexcept
	{
		_code();
	}

private:
	const Functor _code;
};

} // namespace detail

#define EXEC_ON_SCOPE_EXIT ::detail::OnScopeExitExecutor CONCAT_EXPANDED_ARGUMENTS_2(onScopeExit_, __LINE__)
