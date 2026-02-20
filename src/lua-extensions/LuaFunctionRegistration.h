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

#pragma once
#include "cISCLua.h"

namespace LuaFunctionRegistration
{
	struct LuaFunctionInfo
	{
		const char* name;
		lua_CFunction pFunction;

		constexpr LuaFunctionInfo(
			const char* name,
			lua_CFunction function)
			: name(name), pFunction(function)
		{
		}

		bool IsValid() const
		{
			return name != nullptr && *name != '\0' && pFunction;
		}
	};

	// Registers the function into the table at the top of the Lua stack.
	// The caller is responsible for ensuring that the object at the top
	// of the Lua stack is a table, and performing any stack cleanup.
	// This function does not modify the Lua stack position.
	void RegisterFunctionRaw(
		cISCLua& lua,
		const LuaFunctionInfo& info);

	void RegisterFunction(
		cISCLua* pLua,
		const char* tableName,
		const LuaFunctionInfo& info);

	void RegisterFunctions(
		cISCLua* pLua,
		const char* tableName,
		const LuaFunctionInfo* pFunctions,
		size_t functionCount);
}