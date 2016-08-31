#pragma once

#include <functional>

namespace detail {
class OnScopeExitExecutor
{
public:
	explicit OnScopeExitExecutor(std::function<void()>&& code) : _code(std::move(code))
	{}

	~OnScopeExitExecutor()
	{
		_code();
	}

private:
	const std::function<void()> _code;
};
}

#define EXEC_ON_SCOPE_EXIT(convertible_to_std_function) detail::OnScopeExitExecutor onScopeExit_##__FUNCTION__##__FILE__##__LINE__(convertible_to_std_function);
