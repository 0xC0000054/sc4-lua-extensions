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
#include "cIPackageScriptCompilationCallbackServer.h"
#include "IScriptCompilationCallback.h"
#include "cGZPersistResourceKey.h"
#include <unordered_map>

class PackageScriptCompilationCallbackServer : public cIPackageScriptCompilationCallbackServer, private IScriptCompilationCallback
{
public:
	PackageScriptCompilationCallbackServer();

	// IGZUnknown

	bool QueryInterface(uint32_t riid, void** ppvObj) override;
	uint32_t AddRef() override;
	uint32_t Release() override;

	// cIPackageScriptCompilationCallbackServer

	bool AddAnyScriptCompiledCallback(AnyScriptCompiledCallback callback, void* pContext) override;
	bool RemoveAnyScriptCompiledCallback(AnyScriptCompiledCallback callback) override;

	bool AddTargetScriptCompiledCallback(cGZPersistResourceKey const& targetScriptKey, TargetScriptCompiledCallback callback, void* pContext) override;
	bool RemoveTargetScriptCompiledCallback(TargetScriptCompiledCallback callback) override;

private:
	// IScriptCompilationCallback

	void ScriptCompiled(cGZPersistResourceKey const& key, cISCLua* pLua) const override;

	// Private members

	struct TargetScriptInfo
	{
		void* callbackState;

		TargetScriptInfo(cGZPersistResourceKey const& key, void* context)
			: targetScriptKey(key), callbackState(context)
		{
		}

		bool IsTargetScript(cGZPersistResourceKey const& key) const
		{
			return targetScriptKey.instance == key.instance
				&& targetScriptKey.group == key.group
				&& targetScriptKey.type == key.type;
		}

	private:
		cGZPersistResourceKey targetScriptKey;
	};

	std::unordered_map<AnyScriptCompiledCallback, void*> anyScriptCompiledCallbacks;
	std::unordered_map<TargetScriptCompiledCallback, const TargetScriptInfo> targetScriptCompiledCallbacks;
	uint32_t refCount;
};

