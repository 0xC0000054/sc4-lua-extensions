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

#include "cGZLuaScriptServerHooks.h"
#include "cIGZLua5.h"
#include "cIGZLua5Thread.h"
#include "cIGZMessageServer2.h"
#include "cRZMessage2Standard.h"
#include "GZServPtrs.h"
#include "Logger.h"
#include "LuaErrorReporting.h"
#include "LuaScriptServerAddNativeFunctionMessage.h"
#include "Patcher.h"
#include "SC4VersionDetection.h"
#include "SCLuaUtil.h"
#include <cstddef>
#include <cstdint>

namespace
{
	struct cGZLuaScriptServer
	{
		void* vtable;							// 0x0
		uint8_t rzSystemService[0x1c];			// 0x4
		uint32_t refCount;						// 0x20
		bool initialized;						// 0x24
		uint32_t onTickRecursionGuard;			// 0x28
		uint32_t scriptRunCounter;				// 0x2c
		int32_t lastCreatedScriptId;			// 0x30
		cIGZLua5* pLua5;						// 0x34
		void* pLua5CommandGenerator;			// 0x38
		intptr_t scriptInfoMap[3];				// 0x3c
		void* pRunningScriptThreadData;			// 0x48
	};

	static_assert(sizeof(cGZLuaScriptServer) == 0x4c);
	static_assert(offsetof(cGZLuaScriptServer, refCount) == 0x20);
	static_assert(offsetof(cGZLuaScriptServer, initialized) == 0x24);
	static_assert(offsetof(cGZLuaScriptServer, onTickRecursionGuard) == 0x28);
	static_assert(offsetof(cGZLuaScriptServer, pLua5) == 0x34);
	static_assert(offsetof(cGZLuaScriptServer, pRunningScriptThreadData) == 0x48);

	typedef bool(__thiscall* pfn_ScriptArgSetup)(cGZLuaScriptServer* thisPtr);

	static const pfn_ScriptArgSetup RealScriptArgSetup = reinterpret_cast<pfn_ScriptArgSetup>(0x008aed10);

	int32_t print(lua_State* pState)
	{
		Logger& logger = Logger::GetInstance();

		cRZAutoRefCount<cIGZLua5Thread> lua = SCLuaUtil::GetIGZLua5ThreadFromFunctionState(pState);

		const int32_t parameterCount = lua->GetTop();

		lua->GetGlobal("tostring");

		for (int32_t i = 1; i <= parameterCount; i++)
		{
			lua->PushValue(-1); // function to be called
			lua->PushValue(i);  // value to print
			lua->Call(1, 1);

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

		// Users may expect the Lua print data to be updated in real time.
		// Without the flush call, the data will only be written to the
		// file when the buffer is full or the application closes.
		logger.Flush();

		return 0;
	}

	void SendLuaScriptServerAddNativeFunctionsMessage(cIGZLua5* pIGZLua5)
	{
		cIGZMessageServer2Ptr messageServer2;

		if (messageServer2)
		{
			cRZMessage2Standard message;
			message.SetType(kSC4MessageLuaScriptServerAddNativeFunction);
			message.SetData1(reinterpret_cast<intptr_t>(pIGZLua5));
			message.SetIGZUnknown(pIGZLua5);

			messageServer2->MessageSend(reinterpret_cast<cIGZMessage2*>(reinterpret_cast<cIGZMessage2Standard*>(&message)));
		}
	}

	bool __fastcall Hooked_ScriptArgSetup(cGZLuaScriptServer* thisPtr, void* edxUnused)
	{
		bool result = RealScriptArgSetup(thisPtr);

		if (result && thisPtr->initialized && thisPtr->pLua5)
		{
			Logger& logger = Logger::GetInstance();

			const auto printFunctionStatus = SCLuaUtil::RegisterLuaFunction(
				thisPtr->pLua5,
				nullptr,
				"print",
				print);

			if (printFunctionStatus == SCLuaUtil::RegisterLuaFunctionStatus::Ok)
			{
				logger.WriteLine(LogLevel::Info, "Added the LuaScriptServer print function.");
			}
			else
			{
				logger.WriteLine(LogLevel::Info, "Failed to add the LuaScriptServer print function.");
			}

			LuaErrorReporting::InstallErrorLoggingCallback(thisPtr->pLua5);
			SendLuaScriptServerAddNativeFunctionsMessage(thisPtr->pLua5);
		}

		return result;
	}
}

bool cGZLuaScriptServerHooks::Install()
{
	bool result = false;

	Logger& logger = Logger::GetInstance();

	const uint16_t gameVersion = SC4VersionDetection::GetGameVersion();

	if (gameVersion == 641)
	{
		constexpr uintptr_t GZLuaScriptServer_Init_Inject = 0x8B09C8;

		try
		{
			Patcher::InstallCallHook(
				GZLuaScriptServer_Init_Inject,
				reinterpret_cast<uintptr_t>(&Hooked_ScriptArgSetup));
			logger.WriteLine(LogLevel::Info, "Installed the cGZLuaScriptServer hooks.");
			result = true;
		}
		catch (const std::exception& e)
		{
			logger.WriteLineFormatted(
				LogLevel::Error,
				"Failed to install the cGZLuaScriptServer hooks: %s",
				e.what());
		}
	}
	else
	{
		logger.WriteLineFormatted(
			LogLevel::Error,
			"Failed to install the cGZLuaScriptServer hooks, unsupported game version: %d",
			gameVersion);
	}

	return result;
}