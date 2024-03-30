////////////////////////////////////////////////////////////////////////////
//
// This file is part of sc4-growify, a DLL Plugin for SimCity 4 that adds
// a cheat code to convert plopped buildings to a growable RCI zone type.
//
// Copyright (c) 2024 Nicholas Hayes
//
// This file is licensed under terms of the MIT License.
// See LICENSE.txt for more information.
//
////////////////////////////////////////////////////////////////////////////

#include "StringViewUtil.h"
/* #include "boost/algorithm/string.hpp" */

/* bool StringViewUtil::EqualsIgnoreCase(const std::string_view& lhs, const std::string_view& rhs) */
/* { */
/* 	return lhs.length() == rhs.length() */
/* 		&& boost::iequals(lhs, rhs); */
/* } */

/* bool StringViewUtil::StartsWithIgnoreCase(const std::string_view& lhs, const std::string_view& rhs) */
/* { */
/* 	return lhs.length() >=  rhs.length() */
/* 		&& boost::istarts_with(lhs, rhs); */
/* } */

void StringViewUtil::Split(
	const std::string_view& input,
	std::string_view::value_type delim,
	std::vector<std::string_view>& results)
{
	// The following code is adapted from: https://stackoverflow.com/a/36301144

	const size_t inputLength = input.length();
	bool foundDoubleQuote = false;
	bool foundSingleQuote = false;
	size_t argumentLength = 0;

	for (size_t i = 0; i < inputLength; i++)
	{
		size_t start = i;
		if (input[i] == '\"')
		{
			foundDoubleQuote = true;
		}
		else if (input[i] == '\'')
		{
			foundSingleQuote = true;
		}

		if (foundDoubleQuote)
		{
			i++;
			start++;

			while (i < inputLength && input[i] != '\"')
			{
				i++;
			}

			if (i < inputLength)
			{
				foundDoubleQuote = false;
			}

			argumentLength = i - start;
			i++;
		}
		else if (foundSingleQuote)
		{
			i++;
			start++;

			while (i < inputLength && input[i] != '\'')
			{
				i++;
			}

			if (i < inputLength)
			{
				foundSingleQuote = false;
			}

			argumentLength = i - start;
			i++;
		}
		else
		{
			while (i < inputLength && input[i] != delim)
			{
				i++;
			}
			argumentLength = i - start;
		}

		if (argumentLength > 0)
		{
			results.push_back(input.substr(start, argumentLength));
		}
	}
}

/* std::string_view StringViewUtil::TrimWhiteSpace(const std::string_view& input) */
/* { */
/* 	return boost::trim_copy(input); */
/* } */
