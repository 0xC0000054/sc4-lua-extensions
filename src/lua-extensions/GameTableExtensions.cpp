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

#include "GameTableExtensions.h"
#include "cRZAutoRefCount.h"
#include "cRZBaseString.h"
#include "GlobalPointers.h"
#include "LuaFunctionRegistration.h"
#include "LuaHelper.h"
#include "SafeInt.hpp"
#include "SCLuaUtil.h"
#include <algorithm>
#include <array>

namespace
{
	enum class ExecuteCheatStatus
	{
		Ok = 0,
		UnknownCheatCode,
		CheatsDisabled
	};

	ExecuteCheatStatus ExecuteCheat(const cIGZString& cheatName)
	{
		ExecuteCheatStatus result = ExecuteCheatStatus::CheatsDisabled;

		if (spCCM)
		{
			uint32_t cheatID = 0;

			if (spCCM->DoesCheatCodeMatch(cheatName, cheatID))
			{
				spCCM->SendCheatNotifications(cheatName, cheatID);
				result = ExecuteCheatStatus::Ok;
			}
			else
			{
				if (spCCM->GetCheatCodesEnabled())
				{
					result = ExecuteCheatStatus::UnknownCheatCode;
				}
			}
		}

		return result;
	}

	bool ExecuteCommand(const cIGZString& command, cRZAutoRefCount<cIGZCommandParameterSet>& outputParameters)
	{
		bool result = false;

		if (spCommandServer)
		{
			uint32_t commandID = 0;
			cIGZCommandParameterSet* pInputParameters = nullptr;

			if (spCommandServer->ConvertStringToCommand(
				command.ToChar(),
				command.Strlen(),
				commandID,
				pInputParameters))
			{
				if (spCommandServer->CreateCommandParameterSet(outputParameters.AsPPObj()))
				{
					spCommandServer->ExecuteCommand(commandID, pInputParameters, outputParameters);
					result = true;
				}

				if (pInputParameters)
				{
					pInputParameters->Release();
				}
			}
		}

		return result;
	}

	int32_t execute_cheat(lua_State* pState)
	{
		cRZAutoRefCount<cISCLua> lua = SCLuaUtil::GetISCLuaFromFunctionState(pState);

		const int32_t parameterCount = lua->GetTop();

		if (parameterCount == 1 && lua->IsString(-1))
		{
			cRZBaseString command(lua->ToString(-1));

			if (command.Strlen() > 0)
			{
				const ExecuteCheatStatus status = ExecuteCheat(command);

				if (status == ExecuteCheatStatus::Ok)
				{
					lua->PushBoolean(true);
				}
				else if (status == ExecuteCheatStatus::UnknownCheatCode)
				{
					cRZAutoRefCount<cIGZCommandParameterSet> outputParameters;

					if (ExecuteCommand(command, outputParameters))
					{
						LuaHelper::SetResultFromIGZCommandParameterSet(lua, outputParameters);
					}
					else
					{
						lua->PushNil();
					}
				}
				else
				{
					lua->PushNil();
				}
			}
			else
			{
				lua->PushNil();
			}
		}
		else
		{
			lua->PushNil();
		}

		return 1;
	}

	int32_t pause(lua_State* pState)
	{
		if (spSimulator)
		{
			if (!spSimulator->IsPaused())
			{
				spSimulator->Pause();
			}
		}

		return 0;
	}

	int32_t resume(lua_State* pState)
	{
		if (spSimulator)
		{
			if (spSimulator->IsPaused())
			{
				spSimulator->Resume();
			}
		}

		return 0;
	}
}

void GameTableExtensions::Register(cISCLua* pLua)
{
	constexpr std::array<LuaFunctionRegistration::LuaFunctionInfo, 3> Functions =
	{
		LuaFunctionRegistration::LuaFunctionInfo("execute_cheat", execute_cheat),
		LuaFunctionRegistration::LuaFunctionInfo("pause", pause),
		LuaFunctionRegistration::LuaFunctionInfo("resume", resume),
	};

	LuaFunctionRegistration::RegisterFunctions(
		pLua,
		"game",
		Functions.data(),
		Functions.size());
}
