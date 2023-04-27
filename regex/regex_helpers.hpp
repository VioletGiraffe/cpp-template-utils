#pragma once

#include <regex>
#include <string>

namespace regex_helpers {

	template<class CharT, class CharTraits, class Alloc, class RegexTraits, class UnaryFunction>
	[[nodiscard]] std::basic_string<CharT> regex_replace(const std::basic_string<CharT, CharTraits, Alloc>& str, const std::basic_regex<CharT, RegexTraits>& re, UnaryFunction f)
	{
		std::basic_string<CharT, CharTraits, Alloc> result;

		auto endOfLastMatch = str.cbegin();
		using IteratorType = typename std::basic_string<CharT>::const_iterator;
		typename std::match_results<IteratorType>::difference_type positionOfLastMatch = 0;

		auto callback = [&](const std::match_results<IteratorType>& match)
		{
			const auto positionOfThisMatch = match.position(0);
			const auto diff = positionOfThisMatch - positionOfLastMatch;

			auto startOfThisMatch = endOfLastMatch;
			std::advance(startOfThisMatch, diff);

			result.append(endOfLastMatch, startOfThisMatch);
			result.append(f(match));

			auto lengthOfMatch = match.length(0);

			positionOfLastMatch = positionOfThisMatch + lengthOfMatch;

			endOfLastMatch = startOfThisMatch;
			std::advance(endOfLastMatch, lengthOfMatch);
		};

		std::regex_iterator<IteratorType> begin(str.cbegin(), str.cend(), re), end;
		std::for_each(begin, end, callback);

		result.append(endOfLastMatch, str.cend());

		return result;
	}

	template<class CharT, class CharTraits, class Alloc>
	[[nodiscard]] std::basic_string<CharT> replace_match(
		const std::basic_string<CharT, CharTraits, Alloc>& str,
		const std::match_results<typename std::basic_string<CharT, CharTraits, Alloc>::const_iterator>& match,
		const size_t index,
		const std::basic_string<CharT, CharTraits, Alloc>& replaceWith)
	{
		const auto pos0 = match.position(0), posN = match.position(index);
		const auto matchLength0 = match.length(0), matchLengthN = match.length(index);
		return str.substr(pos0, posN - pos0)
			+ replaceWith
			+ str.substr(posN + matchLengthN, pos0 + matchLength0 - posN - matchLengthN);
	}
} // namespace regex_helpers
