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

#include "LuaErrorReporting.h"
#include "cIGZLua5.h"
#include "cIGZLua5Thread.h"
#include "Logger.h"

namespace
{
	void LuaErrorCallback(cIGZLua5Thread* pThread, int32_t errorCode, const char* message)
	{
		Logger& logger = Logger::GetInstance();

		// This error log format is based on the one Maxis used in their built-in Lua script error handler.
		// Maxis removed the logging code from retail builds of the game, so the built-in Lua script error
		// handler just builds an error string that is not used.
		//
		// Unfortunately, Lua scripts added by plugins will not include the script name in the error
		// message, SC4 loads all plugins scripts with the script name as an empty string.
		// But the Lua error message may include the name of the function.

		logger.WriteLine(LogLevel::Error, "Lua script error !\n");

		switch (errorCode)
		{
		case 1:
			logger.WriteLine(LogLevel::Error, "Run-time");
			break;
		case 2:
			logger.WriteLine(LogLevel::Error, "File-related");
			break;
		case 3:
			logger.WriteLine(LogLevel::Error, "Syntax");
			break;
		case 4:
			logger.WriteLine(LogLevel::Error, "Allocation");
			break;
		case 5:
		default:
			logger.WriteLine(LogLevel::Error, "Unknown");
			break;
		}

		if (pThread)
		{
			logger.WriteLineFormatted(LogLevel::Error, "Thread ID: %u", pThread->GetID());
		}
		else
		{
			logger.WriteLine(LogLevel::Error, "Thread ID: Unknown");
		}

		logger.WriteLineFormatted(LogLevel::Error, "Error Type: %s", message);
		logger.WriteLine(LogLevel::Error, "\n==========================================================================================\n");
	}
}

void LuaErrorReporting::InstallErrorLoggingCallback(cIGZLua5* pLua)
{
	if (pLua)
	{
		pLua->SetErrorReporter(LuaErrorCallback, true);
	}
}

void LuaErrorReporting::UninstallErrorLoggingCallback(cIGZLua5* pLua)
{
	if (pLua)
	{
		pLua->SetErrorReporter(nullptr, true);
	}
}
