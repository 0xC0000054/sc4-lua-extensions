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

#include "PackageScriptCompilationCallbackServer.h"

IScriptCompilationCallback* spScriptCompilationCallback;

PackageScriptCompilationCallbackServer::PackageScriptCompilationCallbackServer() : refCount(0)
{
	spScriptCompilationCallback = this;
}

bool PackageScriptCompilationCallbackServer::QueryInterface(uint32_t riid, void** ppvObj)
{
	if (riid == GZIID_PackageScriptCompilationCallbackServer)
	{
		*ppvObj = static_cast<cIPackageScriptCompilationCallbackServer*>(this);
		AddRef();

		return true;
	}
	else if (riid == GZIID_cIGZUnknown)
	{
		*ppvObj = static_cast<cIGZUnknown*>(this);
		AddRef();

		return true;
	}

	return false;
}

uint32_t PackageScriptCompilationCallbackServer::AddRef()
{
	return ++refCount;
}

uint32_t PackageScriptCompilationCallbackServer::Release()
{
	if (refCount > 0)
	{
		--refCount;
	}

	return refCount;
}

bool PackageScriptCompilationCallbackServer::AddAnyScriptCompiledCallback(AnyScriptCompiledCallback callback, void* state)
{
	bool result = false;

	if (callback)
	{
		result = anyScriptCompiledCallbacks.try_emplace(callback, state).second;
	}

	return result;
}

bool PackageScriptCompilationCallbackServer::RemoveAnyScriptCompiledCallback(AnyScriptCompiledCallback callback)
{
	return anyScriptCompiledCallbacks.erase(callback) == 1;
}

bool PackageScriptCompilationCallbackServer::AddTargetScriptCompiledCallback(
	cGZPersistResourceKey const& targetScriptKey,
	TargetScriptCompiledCallback callback,
	void* state)
{
	bool result = false;

	if (callback)
	{
		result = targetScriptCompiledCallbacks.try_emplace(callback, TargetScriptInfo(targetScriptKey, state)).second;
	}

	return result;
}

bool PackageScriptCompilationCallbackServer::RemoveTargetScriptCompiledCallback(TargetScriptCompiledCallback callback)
{
	return targetScriptCompiledCallbacks.erase(callback) == 1;
}

void PackageScriptCompilationCallbackServer::ScriptCompiled(cGZPersistResourceKey const& key, cISCLua* pLua) const
{
	for (const auto& item : anyScriptCompiledCallbacks)
	{
		item.first(key, item.second);
	}

	for (const auto& item : targetScriptCompiledCallbacks)
	{
		if (item.second.IsTargetScript(key))
		{
			item.first(key, pLua, item.second.callbackState);
		}
	}
}
