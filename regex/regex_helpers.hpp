#pragma once

#include <assert.h>
#include <regex>
#include <string>

namespace regex_helpers {

	template<class CharT, class CharTraits, class Alloc, class RegexTraits, class UnaryFunction>
	[[nodiscard]] std::basic_string<CharT, CharTraits, Alloc> regex_replace(const std::basic_string<CharT, CharTraits, Alloc>& str, const std::basic_regex<CharT, RegexTraits>& re, UnaryFunction f)
	{
		using IteratorType = typename std::basic_string<CharT, CharTraits, Alloc>::const_iterator;

		std::basic_string<CharT, CharTraits, Alloc> result;
		auto endOfLastMatch = str.cbegin();

		for (std::regex_iterator<IteratorType, CharT, RegexTraits> it(str.cbegin(), str.cend(), re), end; it != end; ++it)
		{
			const auto& match = *it;
			result.append(endOfLastMatch, match[0].first);
			result.append(f(match));
			endOfLastMatch = match[0].second;
		}

		result.append(endOfLastMatch, str.cend());

		return result;
	}

	// Returns the matched text with one of its capture groups replaced, not the whole subject sequence
	template<class BidirIt, class CharT, class CharTraits, class Alloc>
	[[nodiscard]] std::basic_string<CharT, CharTraits, Alloc> replace_match(
		const std::match_results<BidirIt>& match,
		const size_t index,
		const std::basic_string<CharT, CharTraits, Alloc>& replaceWith)
	{
		assert(match.ready() && index < match.size() && match[index].matched);

		return std::basic_string<CharT, CharTraits, Alloc>(match[0].first, match[index].first)
			+ replaceWith
			+ std::basic_string<CharT, CharTraits, Alloc>(match[index].second, match[0].second);
	}
} // namespace regex_helpers
