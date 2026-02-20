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
#include "SC4GameTableBase.h"

class SC4GameCityTable : public SC4GameTableBase
{
public:
	SC4GameCityTable();

private:
	void RegisterTableFunctions(
		cISC4GameDataRegistry::cEntry* parentNode,
		cISC4GameDataRegistry& registry) override;

	static bool sGetCityName(
		void* pKey,
		cIGZCommandParameterSet* pInput,
		cIGZCommandParameterSet* pOutput,
		cISC4GameDataRegistry* pRegistry);
	static bool sSetCityName(
		void* pKey,
		cIGZCommandParameterSet* pInput,
		cIGZCommandParameterSet* pOutput,
		cISC4GameDataRegistry* pRegistry);

	static bool sGetMayorName(
		void* pKey,
		cIGZCommandParameterSet* pInput,
		cIGZCommandParameterSet* pOutput,
		cISC4GameDataRegistry* pRegistry);
	static bool sSetMayorName(
		void* pKey,
		cIGZCommandParameterSet* pInput,
		cIGZCommandParameterSet* pOutput,
		cISC4GameDataRegistry* pRegistry);

	static bool sPlaceLot(
		void* pKey,
		cIGZCommandParameterSet* pInput,
		cIGZCommandParameterSet* pOutput,
		cISC4GameDataRegistry* pRegistry);
};