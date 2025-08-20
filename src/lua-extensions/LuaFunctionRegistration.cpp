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

#include "LuaFunctionRegistration.h"
#include "Logger.h"

void LuaFunctionRegistration::RegisterFunction(
	cISCLua* pLua,
	const char* tableName,
	const LuaFunctionInfo& info)
{
	Logger& logger = Logger::GetInstance();

	if (pLua && info.IsValid())
	{
		int32_t top = pLua->GetTop();

		if (tableName)
		{
			pLua->GetGlobal(tableName);
		}
		else
		{
			pLua->GetGlobals();
		}

		int32_t functionTableTop = pLua->GetTop();

		if (top != functionTableTop)
		{
			cIGZLua5Thread::LuaType type = pLua->Type(-1);

			if (type == cIGZLua5Thread::LuaTypeTable)
			{
				const char* const kTempFunctionName = "_temp_";

				// We have to register the function with a temporary name and then rename it.
				// Just registering the function with the intended name doesn't work.
				//
				// SC4 uses the same technique when registering the game table functions
				// (e.g. game.trend_slope).

				pLua->Register(info.pFunction, kTempFunctionName);
				pLua->PushString(info.name);
				pLua->GetGlobal(kTempFunctionName);
				pLua->SetTable(-3);
				pLua->Pop(1);

				if (tableName)
				{
					logger.WriteLineFormatted(
						LogLevel::Info,
						"Registered the %s.%s function",
						tableName,
						info.name);
				}
				else
				{
					logger.WriteLineFormatted(
						LogLevel::Info,
						"Registered the %s function",
						info.name);
				}
			}
			else
			{
				if (tableName)
				{
					logger.WriteLineFormatted(
						LogLevel::Info,
						"Failed to register the %s.%s function. The %s object is not a Lua table.",
						tableName,
						info.name,
						tableName);
				}
				else
				{
					logger.WriteLineFormatted(
						LogLevel::Info,
						"Failed to register the %s function.",
						info.name);
				}
			}
		}
		else
		{
			if (tableName)
			{
				logger.WriteLineFormatted(
					LogLevel::Info,
					"Failed to register the %s.%s function. "
					"Is SC4LuaExtensions.dat in the plugins folder?",
					tableName,
					info.name);
			}
			else
			{
				logger.WriteLineFormatted(
					LogLevel::Info,
					"Failed to register the %s function.",
					info.name);
			}
		}

		pLua->SetTop(top);
	}
	else
	{
		if (tableName)
		{
			logger.WriteLineFormatted(
				LogLevel::Info,
				"Failed to register the %s.%s function. "
				"One or more parameters were invalid.",
				tableName,
				info.name);
		}
		else
		{
			logger.WriteLineFormatted(
				LogLevel::Info,
				"Failed to register the %s function. "
				"One or more parameters were invalid.",
				info.name);
		}
	}
}

void LuaFunctionRegistration::RegisterFunctions(
	cISCLua* pLua,
	const char* tableName,
	const LuaFunctionInfo* pFunctions,
	size_t functionCount)
{
	for (size_t i = 0; i < functionCount; i++)
	{
		RegisterFunction(pLua, tableName, pFunctions[i]);
	}
}
