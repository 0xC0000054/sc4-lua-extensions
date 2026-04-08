/*
 * This file is part of sc4-lua-extensions, a DLL Plugin for SimCity 4
 * that extends the game's Lua system with new functions.
 *
 * Copyright (C) 2025, 2026 Nicholas Hayes
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

#include "LuaScriptServerFunctions.h"
#include "cIGZLua5.h"
#include "cIGZLua5Thread.h"
#include "cRZAutoRefCount.h"
#include "Logger.h"
#include "LuaHelper.h"
#include "SCLuaUtil.h"
#include <string_view>

namespace
{
	int32_t hex2dec(lua_State* pState)
	{
		cRZAutoRefCount<cIGZLua5Thread> lua = SCLuaUtil::GetIGZLua5ThreadFromFunctionState(pState);

		double result = 0;

		const int32_t parameterCount = lua->GetTop();

		if (parameterCount == 1 && lua->IsString(-1))
		{
			const char* const text = lua->ToString(-1);
			const uint32_t textLength = lua->Strlen(-1);

			uint32_t parsedValue = 0;

			if (LuaHelper::TryParseHexStringToUint32(std::string_view(text, textLength), parsedValue))
			{
				result = parsedValue;
			}
		}

		lua->PushNumber(result);
		return 1;
	}

	void RegisterFunction(
		cIGZLua5* pLua,
		const char* tableName,
		const std::string_view& functionName,
		lua_CFunction pFunction)
	{
		Logger& logger = Logger::GetInstance();

		const auto status = SCLuaUtil::RegisterLuaFunction(pLua, tableName, functionName, pFunction);

		if (status == SCLuaUtil::RegisterLuaFunctionStatus::Ok)
		{
			if (tableName)
			{
				logger.WriteLineFormatted(
					LogLevel::Info,
					"Registered the LuaScriptServer %s.%s function",
					tableName,
					std::string(functionName).c_str());
			}
			else
			{
				logger.WriteLineFormatted(
					LogLevel::Info,
					"Registered the LuaScriptServer %s function",
					std::string(functionName).c_str());
			}
		}
		else
		{
			if (tableName)
			{
				logger.WriteLineFormatted(
					LogLevel::Info,
					"Failed to register the LuaScriptServer %s.%s function",
					tableName,
					std::string(functionName).c_str());
			}
			else
			{
				logger.WriteLineFormatted(
					LogLevel::Info,
					"Failed to register the LuaScriptServer %s function",
					std::string(functionName).c_str());
			}
		}
	}
}

void LuaScriptServerFunctions::Register(cIGZLua5* pLua)
{
	RegisterFunction(pLua, nullptr, "hex2dec", hex2dec);
}