/*
 * This file is part of sc4-lua-extensions, a DLL Plugin for SimCity 4
 * that extends the game's Lua system with new functions.
 *
 * Copyright (C) 2025 Nicholas Hayes
 *
 * sc4-lua-extensions is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * sc4-lua-extensions is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with sc4-lua-extensions.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "PackageScriptLoadingPatch.h"
#include "cGZPersistResourceKey.h"
#include "cISCLua.h"
#include "Logger.h"
#include "Patcher.h"
#include "SC4VersionDetection.h"
#include <cstdio>

namespace
{
	typedef int32_t(__cdecl* pfn_SCLuaHelp_LoadResourceScript)(
		cISCLua* pLua,
		cGZPersistResourceKey const& resKey,
		char* name);

	static const pfn_SCLuaHelp_LoadResourceScript Real_SCLuaHelp_LoadResourceScript = reinterpret_cast<pfn_SCLuaHelp_LoadResourceScript>(0x5FB7B0);

		int32_t __cdecl Hooked_SCLuaHelp_LoadResourceScript(cISCLua* pLua, cGZPersistResourceKey const& resKey, char* name)
	{
		char resourceName[1024]{};

		// Lua 5.0 will truncate script file names longer than 44 characters in length
		// with an ellipsis, so we have to keep this string shorter than that.
		// The following format string expands to 37 characters.

		std::snprintf(
			resourceName,
			sizeof(resourceName),
			"TGI=0x%08x,0x%08x,0x%08x",
			resKey.type,
			resKey.group,
			resKey.instance);

		return Real_SCLuaHelp_LoadResourceScript(pLua, resKey, resourceName);
	}
}

void PackageScriptLoadingPatch::Install()
{
	Logger& logger = Logger::GetInstance();

	const uint16_t gameVersion = SC4VersionDetection::GetGameVersion();

	if (gameVersion == 641)
	{
		constexpr uintptr_t SCLuaHelp_LoadPackageScripts_Inject = 0x5FBD64;

		try
		{
			Patcher::InstallCallHook(SCLuaHelp_LoadPackageScripts_Inject, reinterpret_cast<uintptr_t>(&Hooked_SCLuaHelp_LoadResourceScript));
			logger.WriteLine(LogLevel::Info, "Installed the Lua package script name patch.");
		}
		catch (const std::exception& e)
		{
			logger.WriteLineFormatted(
				LogLevel::Error,
				"Failed to install the Lua package script name patch: %s",
				e.what());
		}
	}
	else
	{
		logger.WriteLineFormatted(
			LogLevel::Error,
			"Failed to install the Lua package script name patch, unsupported game version: %d",
			gameVersion);
	}
}
