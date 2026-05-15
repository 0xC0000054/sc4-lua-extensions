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

#include "GameTableExtensions.h"
#include "cRZAutoRefCount.h"
#include "cRZBaseString.h"
#include "frozen/unordered_set.h"
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
		// Some cheat codes are blocked from the advisor scripts due to them
		// showing UI or having other behavior that could be abused by malicious
		// scripts.
		static constexpr frozen::unordered_set<uint32_t, 12> BlockedCheatIDs =
		{
			0x4ac096ca, // BuildingPlop - shows UI
			0x4ac096c6, // LotPlop - shows UI
			0x88f3d4bb, // RenderProp - allows messing with the game's render settings
			0x88f3d4bc, // RP - alias for RenderProp
			0x4ab569c9, // PlopAllBuildings
			0x8929e68e, // PlopAllLots
			0x0bfd8064, // toll
			0x6976afa0, // Recorder - animation recorder
			0xcbf31943, // RiskyMoney - chance of earthquakes when used
			0x2b60e45b, // CreateException - developer cheat with CTD potential
			0x8b6d4f8d, // ForceIdle
			0x2ab5c431, // GP
		};

		ExecuteCheatStatus result = ExecuteCheatStatus::CheatsDisabled;

		if (spCCM)
		{
			uint32_t cheatID = 0;

			if (spCCM->DoesCheatCodeMatch(cheatName, cheatID))
			{
				if (!BlockedCheatIDs.contains(cheatID))
				{
					spCCM->SendCheatNotifications(cheatName, cheatID);
					result = ExecuteCheatStatus::Ok;
				}
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
		// Some commands are blocked from the advisor scripts due to them
		// having behavior that could be abused by malicious scripts.
		static constexpr frozen::unordered_set<uint32_t, 28> BlockedCommandIDs =
		{
			// App commands
			0x0bc24f90, // CreateException - crashes the game
			0x6bbc0dad, // CreateGZLog - allows creating a text file at a user controlled location
			0xabac26b5, // ExecuteCheat
			0xabac067c, // ExecuteScript - allows running arbitrary Lua test scripts
			0xcbb17c11, // GameDelay
			0xcbb17c12, // GamePause
			0xabb17c74, // SendMessage - broadcasts on the internal messaging system
			0x6bb9bbe3, // SetScriptAutoYield - only valid for test scripts
			0xcbac32f4, // WaitForMessage2 - only valid for test scripts
			0xebac3957, // CancelWaitForMessage2 - only valid for test scripts
			0x0bb5c013, // GetAppState - only valid for test scripts
			0x6bce8ae6, // SetPopupModalDialogsEnabled - user controlled option
			0x2bb48d41, // Demolish
			0x4ba12cee, // GZWinEnableUserInput
			0x8ba12cf2, // GZWinGenerateInputEvent
			0xcba12cfa, // GZWinClickButton
			0xabb17c0e, // GZWinCreateWindow
			0xebb151d7, // GZWinDestroyWindow
			0x4ba6c540, // GZWinWaitForWindow
			0xabb151c8, // GZWinShowWindow
			0x2ba12da6, // GZWinSelectListboxItem
			0x8ba12d0f, // GZWinMessageBox
			// City commands
			0xcab22c64, // AnimationRecorder
			0xabb5eaeb, // SetCityEstablished
			0x6a935e36, // SaveCity
			0x6a935e37, // SaveCityQuick
			0x6a935e39, // QuitCity
			0x6a935e3c, // QuitGame
		};

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
				if (!BlockedCommandIDs.contains(commandID))
				{
					if (spCommandServer->CreateCommandParameterSet(outputParameters.AsPPObj()))
					{
						spCommandServer->ExecuteCommand(commandID, pInputParameters, outputParameters);
						result = true;
					}
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
