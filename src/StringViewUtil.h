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

#pragma once
#include <string_view>
#include <vector>

namespace StringViewUtil
{
	/* bool EqualsIgnoreCase( */
	/* 	const std::string_view& lhs, */
	/* 	const std::string_view& rhs); */

	/* bool StartsWithIgnoreCase( */
	/* 	const std::string_view& lhs, */
	/* 	const std::string_view& rhs); */

	void Split(
		const std::string_view& input,
		std::string_view::value_type delim,
		std::vector<std::string_view>& results);

	/* std::string_view TrimWhiteSpace(const std::string_view& input); */
}