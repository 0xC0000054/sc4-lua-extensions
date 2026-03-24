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

class cGZPersistResourceKey;
class cISCLua;

class IScriptCompilationCallback
{
public:
	/**
	 * @brief The callback that is executed when a script was successfully compiled.
	 * @param key The resource key that the script was loaded from.
	 * @param pLua The Lua instance that compiled the script.
	 */
	virtual void ScriptCompiled(cGZPersistResourceKey const& key, cISCLua* pLua) const = 0;
};

extern IScriptCompilationCallback* spScriptCompilationCallback;
