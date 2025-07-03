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

#include "LuaGlobalFunctions.h"
#include "LuaFunctionRegistration.h"
#include "Logger.h"
#include "SCLuaUtil.h"

namespace
{
	int32_t print(lua_State* pState)
	{
		Logger& logger = Logger::GetInstance();

		cRZAutoRefCount<cISCLua> lua = SCLuaUtil::GetISCLuaFromFunctionState(pState);

		const int32_t parameterCount = lua->GetTop();

		lua->GetGlobal("tostring");

		for (int32_t i = 1; i <= parameterCount; i++)
		{
			lua->PushValue(-1); // function to be called
			lua->PushValue(i);  // value to print
			lua->RawCall(1, 1);

			const char* result = lua->ToString(-1); // get result

			if (result)
			{
				if (i > 1)
				{
					logger.Write(LogLevel::Info, "\t");
				}

				logger.Write(LogLevel::Info, result);
			}
			else
			{
				lua->Error("`tostring' must return a string to `print'");
			}

			lua->Pop(1); // pop result
		}

		logger.Write(LogLevel::Info, "\n");

		return 0;
	}
}

void LuaGlobalFunctions::Register(cISCLua* pLua)
{
	LuaFunctionRegistration::RegisterFunction(
		pLua,
		nullptr,
		LuaFunctionRegistration::LuaFunctionInfo("print", print));
}