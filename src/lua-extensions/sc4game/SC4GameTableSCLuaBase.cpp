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

#include "SC4GameTableSCLuaBase.h"
#include "DebugUtil.h"
#include "Logger.h"

SC4GameTableSCLuaBase::SC4GameTableSCLuaBase(const char* tableName)
	: tableName(tableName)
{
}

void SC4GameTableSCLuaBase::Register(cISCLua& lua) const
{
	Logger& logger = Logger::GetInstance();

	int32_t savedTop = lua.GetTop();

	lua.GetGlobal("sc4game");

	int32_t sc4gameTableTop = lua.GetTop();

	if (sc4gameTableTop != savedTop && lua.Type(-1) == cIGZLua5Thread::LuaTypeTable)
	{
		// Get the child table with the specified field name.
		lua.PushString(tableName);
		lua.GetTable(sc4gameTableTop);

		if (lua.IsTable(-1))
		{
			std::vector<LuaFunctionRegistration::LuaFunctionInfo> functions = GetTableFunctions();

			for (const auto& info : functions)
			{
				LuaFunctionRegistration::RegisterFunctionRaw(lua, info);

				logger.WriteLineFormatted(
					LogLevel::Info,
					"Registered the sc4game.%s.%s function.",
					tableName,
					info.name);
			}
		}
		else
		{
			logger.WriteLineFormatted(
				LogLevel::Error,
				"The sc4game.%s field is not a table.",
				tableName);
		}

		lua.SetTop(savedTop);
	}
	else
	{
		logger.WriteLine(LogLevel::Error, "The sc4game global is not a table.");
	}
}