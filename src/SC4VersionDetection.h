/*
 * This file is part of nam-dll, a DLL Plugin for SimCity 4
 * that improves interoperability with the Network Addon Mod.
 *
 * Copyright (c) 2023 NAM Team contributors
 *
 * nam-dll is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * nam-dll is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with nam-dll.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#include <cstdint>

class SC4VersionDetection
{
public:

	SC4VersionDetection();

	uint16_t GetGameVersion() const;

private:

	const uint16_t gameVersion;
};

