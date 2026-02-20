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

#include "SC4GameTableBase.h"
#include "cRZAutoRefCount.h"

SC4GameTableBase::SC4GameTableBase(const char* tableName)
	: refCount(0), tableName(tableName)
{
}

void SC4GameTableBase::Register(cISC4GameDataRegistry& registry)
{
	cRZAutoRefCount<cIGZUnknown> parentNodeKey;
	this->QueryInterface(GZIID_cIGZUnknown, parentNodeKey.AsPPVoid());

	auto parentNode = registry.AddChildEntry(
		parentNodeKey,
		tableName,
		registry.GetRootNodeEntry());

	if (parentNode)
	{
		RegisterTableFunctions(parentNode, registry);
	}
}

bool SC4GameTableBase::QueryInterface(uint32_t riid, void** ppvObj)
{
	if (riid == GZIID_cIGZUnknown)
	{
		*ppvObj = static_cast<cIGZUnknown*>(this);
		AddRef();

		return true;
	}

	return false;
}

uint32_t SC4GameTableBase::AddRef()
{
	return ++refCount;
}

uint32_t SC4GameTableBase::Release()
{
	if (refCount > 0)
	{
		--refCount;
	}

	return refCount;
}
