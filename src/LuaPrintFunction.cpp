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

#include "LuaPrintFunction.h"
#include "LuaFunctionRegistration.h"
#include "Logger.h"
#include "Patcher.h"
#include "SCLuaUtil.h"
#include "SC4VersionDetection.h"

using namespace std::string_view_literals;

namespace
{
	static bool sbPrintMaxisScriptDebugOutput = false;

	bool IsMaxisScriptDebugOutput(const std::string_view& text)
	{
		// Automata debug text in _system.lua.
		if (text == "Executing _system.lua..."sv
			|| text == "NOTE! All tables supporting inheritance through '_parent' field \n\thave to be created via 'TTT' prefix."sv)
		{
			return true;
		}

		// The overridden dofile function in _adv_sys.lua logs every file it loads.
		// Because it includes the file name, we only check the prefix.
		if (text.starts_with("dofile() -- '"sv))
		{
			return true;
		}

		// The set_task_range_and_print_info function in tutorial_registry.lua logs
		// the GUID and number of steps for each tutorial.
		// We check both the prefix and two possible suffixes.
		if (text.starts_with("Tutorial \""sv))
		{
			if (text.ends_with("steps."sv)
				|| text.ends_with("steps. - Not registerd."sv))
			{
				return true;
			}
		}

		return false;
	}

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
				else
				{
					if (!sbPrintMaxisScriptDebugOutput && IsMaxisScriptDebugOutput(result))
					{
						lua->Pop(1); // pop result
						return 0;
					}
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

	struct LuaExtensionEntry
	{
		const char* functionName;
		void* functionAddress;
		bool isGlobal;
	};

	bool Patch_cLUAExtensionsCollector_print()
	{
		// SC4 stores its built-in Lua extension for the advisor system in a statically initialized vector.
		// Because the vector is initialized before DLL plugins are loaded, we have to find the game's
		// print function in the vector and replace its function pointer with our own.
		//
		// We have to manually walk the vector data to get the correct item.
		// The vector type used in SimCity 4 consists of start, end, and capacity pointers.
		// Only the start and end pointers are required to get the size.

		constexpr uintptr_t VectorStartPtrAddress = 0xB207CC;
		constexpr uintptr_t VectorEndPtrAddress = VectorStartPtrAddress + sizeof(void*);

		LuaExtensionEntry* pData = reinterpret_cast<LuaExtensionEntry*>(*reinterpret_cast<uintptr_t*>(VectorStartPtrAddress));
		LuaExtensionEntry* pDataEnd = reinterpret_cast<LuaExtensionEntry*>(*reinterpret_cast<uintptr_t*>(VectorEndPtrAddress));

		const size_t count = pDataEnd - pData;

		for (size_t i = 0; i < count; i++)
		{
			LuaExtensionEntry& entry = pData[i];

			if (entry.isGlobal)
			{
				if (strncmp(entry.functionName, "print", 5) == 0)
				{
					entry.functionAddress = &print;
					return true;
				}
			}
		}

		return false;
	}
}

bool LuaPrintFunction::PatchMaxisPrintFunctionPointers()
{
	bool result = false;

	Logger& logger = Logger::GetInstance();

	const uint16_t gameVersion = SC4VersionDetection::GetGameVersion();

	if (gameVersion == 641)
	{
		constexpr uintptr_t SC4AutomataScriptSystem_InitScripts_Inject = 0x51C50B;

		try
		{
			if (Patch_cLUAExtensionsCollector_print()
				&& Patcher::OverwriteMemoryUintptr(
					SC4AutomataScriptSystem_InitScripts_Inject,
					reinterpret_cast<uintptr_t>(&print)))
			{
				logger.WriteLine(LogLevel::Info, "Replaced the Maxis Lua print function overrides.");
				result = true;
			}
		}
		catch (const std::exception& e)
		{
			logger.WriteLineFormatted(
				LogLevel::Error,
				"Failed to replace the Maxis Lua print function overrides: %s",
				e.what());
		}
	}
	else
	{
		logger.WriteLineFormatted(
			LogLevel::Error,
			"Failed to replace the Maxis Lua print function overrides, unsupported game version: %d",
			gameVersion);
	}

	return result;
}

void LuaPrintFunction::RegisterFallbackPrintFunction(cISCLua* pLua)
{
	LuaFunctionRegistration::RegisterFunction(
		pLua,
		nullptr,
		LuaFunctionRegistration::LuaFunctionInfo("print", print));
}

void LuaPrintFunction::SetPrintMaxisScriptDebugOutput(bool enabled)
{
	sbPrintMaxisScriptDebugOutput = enabled;
}
