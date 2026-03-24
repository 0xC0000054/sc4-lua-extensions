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
#include "cIGZUnknown.h"

class cGZPersistResourceKey;
class cISCLua;

static const uint32_t GZCLSID_PackageScriptCompilationCallbackServer = 0x480FAD3F;
static const uint32_t GZIID_PackageScriptCompilationCallbackServer = 0xADDFAD50;

/**
 * @brief Provides callbacks when the game compiles package advisor/automata scripts.
 */
class cIPackageScriptCompilationCallbackServer : public cIGZUnknown
{
public:
	/**
	 * @brief A callback that will be called when any package script is successfully compiled.
	 * This is intended for debug logging.
	 *
	 * @param key The script resource key.
	 * @param pContext The context that was set when registering the callback.
	 */
	typedef void (*AnyScriptCompiledCallback)(cGZPersistResourceKey const& key, void* pContext);

	/**
	 * @brief A callback that will be called when the specified package script is successfully compiled.
	 *
	 * This is intended to allow DLLs that add Lua functions to hook up a script's native functions after
	 * the script was compiled, instead of waiting until PostCityInit when all scripts have been compiled.
	 * This is necessary for dependent scripts to use those functions when initializing their global
	 * variables.
	 *
	 * Note that the base game initializes the native functions in its game and sc4game tables after all
	 * scripts have been compiled.
	 *
	 * The callback should be registered in PostAppInit or PreCityInit, with error reporting or suitable
	 * fallback code in PostCityInit if the server is unavailable.
	 *
	 * @param key The script resource key.
	 * @param pLua The Lua instance that compiled the script.
	 * @param pContext The context that was set when registering the callback.
	 */
	typedef void (*TargetScriptCompiledCallback)(cGZPersistResourceKey const& key, cISCLua* pLua, void* pContext);

	/**
	 * @brief Adds an AnyScriptCompiledCallback function.
	 * @param callback The callback function pointer.
	 * @param pContext Context that will be passed to the callback.
	 * @return True if successful; otherwise, false.
	 */
	virtual bool AddAnyScriptCompiledCallback(AnyScriptCompiledCallback callback, void* pContext) = 0;

	/**
	 * @brief Removes an AnyScriptCompiledCallback function.
	 * @param callback The callback function pointer.
	 * @return True if successful; otherwise, false.
	 */
	virtual bool RemoveAnyScriptCompiledCallback(AnyScriptCompiledCallback callback) = 0;

	/**
	 * @brief Adds an TargetScriptCompiledCallback function.
	 * @param targetScriptKey The resource key of the script that the callback is for.
	 * @param callback The callback function pointer.
	 * @param pContext Context that will be passed to the callback.
	 * @return True if successful; otherwise, false.
	 */
	virtual bool AddTargetScriptCompiledCallback(cGZPersistResourceKey const& targetScriptKey, TargetScriptCompiledCallback callback, void* pContext) = 0;

	/**
	 * @brief Removes an TargetScriptCompiledCallback function.
	 * @param callback The callback function pointer.
	 * @return True if successful; otherwise, false.
	 */
	virtual bool RemoveTargetScriptCompiledCallback(TargetScriptCompiledCallback callback) = 0;
};