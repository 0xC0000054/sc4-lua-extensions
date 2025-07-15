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

#include "Patcher.h"

#include <Windows.h>
#include "wil/resource.h"
#include "wil/win32_helpers.h"

void Patcher::InstallJump(uintptr_t address, uintptr_t destination)
{
	DWORD oldProtect;
	THROW_IF_WIN32_BOOL_FALSE(VirtualProtect(reinterpret_cast<void*>(address), 5, PAGE_EXECUTE_READWRITE, &oldProtect));

	*((uint8_t*)address) = 0xE9;
	*((uintptr_t*)(address + 1)) = destination - address - 5;
}

void Patcher::InstallCallHook(uintptr_t address, uintptr_t pfnFunc)
{
	DWORD oldProtect;
	THROW_IF_WIN32_BOOL_FALSE(VirtualProtect(reinterpret_cast<void*>(address), 5, PAGE_EXECUTE_READWRITE, &oldProtect));

	*((uint8_t*)address) = 0xE8;
	*((uintptr_t*)(address + 1)) = pfnFunc - address - 5;
}

bool Patcher::OverwriteMemoryUintptr(uintptr_t address, uintptr_t newValue)
{
	bool result = false;

	DWORD oldProtect;
	if (VirtualProtect(reinterpret_cast<void*>(address), sizeof(uintptr_t), PAGE_EXECUTE_READWRITE, &oldProtect))
	{
		*((uintptr_t*)address) = newValue;
		result = true;
	}

	return result;
}
