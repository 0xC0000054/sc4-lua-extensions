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

#pragma once
#include "SC4GameTableBase.h"
#include "cIGZFrameWork.h"
#include "cIGZLanguageManager.h"
#include "cRZAutoRefCount.h"
#include "cRZBaseString.h"

class SC4GameLanguageTable : public SC4GameTableBase
{
public:
	SC4GameLanguageTable();

	void PostAppInit(cIGZFrameWork* const pFrameWork);
	void PreAppShutdown();

private:
	void RegisterTableFunctions(
		cISC4GameDataRegistry::cEntry* parentNode,
		cISC4GameDataRegistry& registry) override;

	static bool sIsLeftHandDrive(
		void* pKey,
		cIGZCommandParameterSet* pInput,
		cIGZCommandParameterSet* pOutput,
		cISC4GameDataRegistry* pRegistry);
	static bool sAreFirstAndLastNamesReversed(
		void* pKey,
		cIGZCommandParameterSet* pInput,
		cIGZCommandParameterSet* pOutput,
		cISC4GameDataRegistry* pRegistry);

	static bool sGetCurrencySymbol(
		void* pKey,
		cIGZCommandParameterSet* pInput,
		cIGZCommandParameterSet* pOutput,
		cISC4GameDataRegistry* pRegistry);
	static bool sDoesCurrencySymbolPrecedeAmount(
		void* pKey,
		cIGZCommandParameterSet* pInput,
		cIGZCommandParameterSet* pOutput,
		cISC4GameDataRegistry* pRegistry);
	static bool sIsSpaceBetweenCurrencySymbolAndAmount(
		void* pKey,
		cIGZCommandParameterSet* pInput,
		cIGZCommandParameterSet* pOutput,
		cISC4GameDataRegistry* pRegistry);

	static bool sGetDecimalSeparator(
		void* pKey,
		cIGZCommandParameterSet* pInput,
		cIGZCommandParameterSet* pOutput,
		cISC4GameDataRegistry* pRegistry);
	static bool sGetThousandSeparator(
		void* pKey,
		cIGZCommandParameterSet* pInput,
		cIGZCommandParameterSet* pOutput,
		cISC4GameDataRegistry* pRegistry);

	static bool sGetDateString(
		void* pKey,
		cIGZCommandParameterSet* pInput,
		cIGZCommandParameterSet* pOutput,
		cISC4GameDataRegistry* pRegistry);

	static bool sGetMoneyString(
		void* pKey,
		cIGZCommandParameterSet* pInput,
		cIGZCommandParameterSet* pOutput,
		cISC4GameDataRegistry* pRegistry);

	static bool sGetNumberString(
		void* pKey,
		cIGZCommandParameterSet* pInput,
		cIGZCommandParameterSet* pOutput,
		cISC4GameDataRegistry* pRegistry);
	static bool sGetFormattedNumberString(
		void* pKey,
		cIGZCommandParameterSet* pInput,
		cIGZCommandParameterSet* pOutput,
		cISC4GameDataRegistry* pRegistry);

	cRZAutoRefCount<cIGZLanguageManager> languageManager;
};