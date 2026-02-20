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

#include "SC4GameLanguageTable.h"
#include "cIGZCommandParameterSet.h"
#include "cIGZLanguageUtility.h"
#include "cIGZVariant.h"
#include "cISC4GameDataRegistry.h"
#include "cRZBaseVariant.h"
#include "LuaIGZCommandParameterSetHelper.h"
#include "SafeInt.hpp"
#include <type_traits>

static constexpr uint32_t kCurrentLanguageID = 0;

namespace
{
	cRZBaseString BuildDoubleFormatString(uint32_t precision)
	{
		// Build a format string in the form of '%.<precision>f'.

		cRZBaseString result("%.");

		char numberBuffer[128]{};

		int length = std::snprintf(numberBuffer, sizeof(numberBuffer), "%u", precision);

		result.Append(numberBuffer, static_cast<uint32_t>(length));
		result.Append("f", 1);

		return result;
	}

	cRZBaseString GetCurrencySymbol()
	{
		// The currency symbol used in SimCity 4 is the section sign.
		// https://en.wikipedia.org/wiki/Section_sign
		//
		// We use the escaped UTF-8 representation of that symbol.
		// UTF-8 is the internal string encoding used by SimCity 4.

		return cRZBaseString("\xC2\xA7");
	}

	cRZBaseString FormatNumberString(cIGZLanguageUtility* pLU, double value, uint32_t decimalPlaces = 0)
	{
		cRZBaseString result;

		const cRZBaseString currencySymbol = GetCurrencySymbol();


		if (decimalPlaces >= 1)
		{
			// We add one to the requested number of decimal places to work around a quirk
			// of SC4's formating code.
			// For whatever reason, it appears to be counting the decimal place as part of
			// the precision character count.
			//
			// Using 123.456789 as an example, the following format strings would print:
			// "%.1s" = 123.
			// "%.2s" = 123.4
			// "%.3s" = 123.45
			// ... and so on.

			const cRZBaseString format = BuildDoubleFormatString(decimalPlaces + 1);
			cRZBaseString temp;

			if (pLU->MakeFormattedNumberString(
				value,
				temp,
				format))
			{
				if (temp.Strlen() > 0)
				{
					result = std::move(temp);
				}
			}
		}
		else
		{
			cRZBaseString temp;

			// For numbers with zero decimal places, we do the following:
			// 1. Round the value to the nearest whole number.
			// 2. Print the number with one decimal place of precision.
			// 3. Trim the string to remove the decimal place and anything after it.
			//
			// This workaround is required to correctly print negative whole numbers.
			// The MakeNumberString method will return an empty string for some languages.

			if (pLU->MakeFormattedNumberString(
				round(value),
				temp,
				cRZBaseString("%.1f")))
			{
				if (temp.Strlen() > 0)
				{
					cRZBaseString decimalSeparator;

					if (pLU->GetDecimalSeparator(decimalSeparator))
					{
						int32_t index = temp.RFind(decimalSeparator, UINT_MAX, false);

						if (index > 0)
						{
							temp.Erase(static_cast<uint32_t>(index), UINT_MAX);
							result = std::move(temp);
						}
					}
				}
			}
		}

		return result;
	}
}

SC4GameLanguageTable::SC4GameLanguageTable()
	: SC4GameTableBase("language")
{
}

void SC4GameLanguageTable::PostAppInit(cIGZFrameWork* const pFrameWork)
{
	constexpr uint32_t kGZLanguageManagerSysServiceID = 0x441E5070;
	constexpr uint32_t GZIID_cIGZLanguageManager = 0x0054B7D5;

	pFrameWork->GetSystemService(
		kGZLanguageManagerSysServiceID,
		GZIID_cIGZLanguageManager,
		languageManager.AsPPVoid());
}

void SC4GameLanguageTable::PreAppShutdown()
{
	languageManager.Reset();
}

void SC4GameLanguageTable::RegisterTableFunctions(
	cISC4GameDataRegistry::cEntry* parentNode,
	cISC4GameDataRegistry& registry)
{
	registry.AddChildEntry(
		&sIsLeftHandDrive,
		"is_left_hand_drive",
		sIsLeftHandDrive,
		parentNode,
		cISC4GameDataRegistry::EntryFlagCallFunction,
		this);

	registry.AddChildEntry(
		&sAreFirstAndLastNamesReversed,
		"are_first_and_last_names_reversed",
		sAreFirstAndLastNamesReversed,
		parentNode,
		cISC4GameDataRegistry::EntryFlagCallFunction,
		this);

	registry.AddChildEntry(
		&sGetCurrencySymbol,
		"get_currency_symbol",
		sGetCurrencySymbol,
		parentNode,
		cISC4GameDataRegistry::EntryFlagCallFunction,
		this);

	registry.AddChildEntry(
		&sDoesCurrencySymbolPrecedeAmount,
		"does_currency_symbol_precede_amount",
		sDoesCurrencySymbolPrecedeAmount,
		parentNode,
		cISC4GameDataRegistry::EntryFlagCallFunction,
		this);

	registry.AddChildEntry(
		&sIsSpaceBetweenCurrencySymbolAndAmount,
		"is_space_between_currency_symbol_and_amount",
		sIsSpaceBetweenCurrencySymbolAndAmount,
		parentNode,
		cISC4GameDataRegistry::EntryFlagCallFunction,
		this);

	registry.AddChildEntry(
		&sGetDecimalSeparator,
		"get_decimal_separator",
		sGetDecimalSeparator,
		parentNode,
		cISC4GameDataRegistry::EntryFlagCallFunction,
		this);

	registry.AddChildEntry(
		&sGetThousandSeparator,
		"get_thousands_separator",
		sGetThousandSeparator,
		parentNode,
		cISC4GameDataRegistry::EntryFlagCallFunction,
		this);

	registry.AddChildEntry(
		&sGetDateString,
		"get_date_string",
		sGetDateString,
		parentNode,
		cISC4GameDataRegistry::EntryFlagCallFunction,
		this);

	registry.AddChildEntry(
		&sGetMoneyString,
		"get_money_string",
		sGetMoneyString,
		parentNode,
		cISC4GameDataRegistry::EntryFlagCallFunction,
		this);

	registry.AddChildEntry(
		&sGetNumberString,
		"get_number_string",
		sGetNumberString,
		parentNode,
		cISC4GameDataRegistry::EntryFlagCallFunction,
		this);

	registry.AddChildEntry(
		&sGetFormattedNumberString,
		"get_formatted_number_string",
		sGetFormattedNumberString,
		parentNode,
		cISC4GameDataRegistry::EntryFlagCallFunction,
		this);
}

bool SC4GameLanguageTable::sIsLeftHandDrive(
	void* pKey,
	cIGZCommandParameterSet* pInput,
	cIGZCommandParameterSet* pOutput,
	cISC4GameDataRegistry* pRegistry)
{
	auto entry = pRegistry->GetEntryID(pKey);
	auto pThis = static_cast<SC4GameLanguageTable*>(pRegistry->GetEntryContext(entry));

	bool result = false;

	if (pThis->languageManager)
	{
		auto pLU = pThis->languageManager->GetLanguageUtility(kCurrentLanguageID);
		result = pLU->GetLanguageRoadDrivingSide() == 0;
	}

	LuaIGZCommandParameterSetHelper::SetValue(
		pOutput,
		1,
		result);
	return true;
}

bool SC4GameLanguageTable::sAreFirstAndLastNamesReversed(
	void* pKey,
	cIGZCommandParameterSet* pInput,
	cIGZCommandParameterSet* pOutput,
	cISC4GameDataRegistry* pRegistry)
{
	auto entry = pRegistry->GetEntryID(pKey);
	auto pThis = static_cast<SC4GameLanguageTable*>(pRegistry->GetEntryContext(entry));

	bool result = false;

	if (pThis->languageManager)
	{
		auto pLU = pThis->languageManager->GetLanguageUtility(kCurrentLanguageID);
		result = pLU->AreFirstAndLastNamesReversed();
	}

	LuaIGZCommandParameterSetHelper::SetValue(
		pOutput,
		1,
		result);
	return true;
}

bool SC4GameLanguageTable::sGetCurrencySymbol(
	void* pKey,
	cIGZCommandParameterSet* pInput,
	cIGZCommandParameterSet* pOutput,
	cISC4GameDataRegistry* pRegistry)
{
	const cRZBaseString currencySymbol = GetCurrencySymbol();

	LuaIGZCommandParameterSetHelper::SetValue(
		pOutput,
		1,
		currencySymbol);
	return true;
}

bool SC4GameLanguageTable::sDoesCurrencySymbolPrecedeAmount(
	void* pKey,
	cIGZCommandParameterSet* pInput,
	cIGZCommandParameterSet* pOutput,
	cISC4GameDataRegistry* pRegistry)
{
	auto entry = pRegistry->GetEntryID(pKey);
	auto pThis = static_cast<SC4GameLanguageTable*>(pRegistry->GetEntryContext(entry));

	bool result = false;

	if (pThis->languageManager)
	{
		auto pLU = pThis->languageManager->GetLanguageUtility(kCurrentLanguageID);
		result = pLU->DoesCurrencySymbolPrecedeAmount();
	}

	LuaIGZCommandParameterSetHelper::SetValue(
		pOutput,
		1,
		result);
	return true;
}

bool SC4GameLanguageTable::sIsSpaceBetweenCurrencySymbolAndAmount(
	void* pKey,
	cIGZCommandParameterSet* pInput,
	cIGZCommandParameterSet* pOutput,
	cISC4GameDataRegistry* pRegistry)
{
	auto entry = pRegistry->GetEntryID(pKey);
	auto pThis = static_cast<SC4GameLanguageTable*>(pRegistry->GetEntryContext(entry));

	bool result = false;

	if (pThis->languageManager)
	{
		auto pLU = pThis->languageManager->GetLanguageUtility(kCurrentLanguageID);
		result = pLU->IsSpaceBetweenCurrencySymbolAndAmount();
	}

	LuaIGZCommandParameterSetHelper::SetValue(
		pOutput,
		1,
		result);
	return true;
}

bool SC4GameLanguageTable::sGetDecimalSeparator(
	void* pKey,
	cIGZCommandParameterSet* pInput,
	cIGZCommandParameterSet* pOutput,
	cISC4GameDataRegistry* pRegistry)
{
	auto entry = pRegistry->GetEntryID(pKey);
	auto pThis = static_cast<SC4GameLanguageTable*>(pRegistry->GetEntryContext(entry));

	cRZBaseString decimalSeparator;

	if (pThis->languageManager)
	{
		auto pLU = pThis->languageManager->GetLanguageUtility(kCurrentLanguageID);

		if (!pLU->GetDecimalSeparator(decimalSeparator))
		{
			decimalSeparator.FromChar(".", 1);
		}
	}
	else
	{
		decimalSeparator.FromChar(".", 1);
	}

	LuaIGZCommandParameterSetHelper::SetValue(
		pOutput,
		1,
		decimalSeparator);
	return true;
}

bool SC4GameLanguageTable::sGetThousandSeparator(
	void* pKey,
	cIGZCommandParameterSet* pInput,
	cIGZCommandParameterSet* pOutput,
	cISC4GameDataRegistry* pRegistry)
{
	auto entry = pRegistry->GetEntryID(pKey);
	auto pThis = static_cast<SC4GameLanguageTable*>(pRegistry->GetEntryContext(entry));

	cRZBaseString thousandsSeparator;

	if (pThis->languageManager)
	{
		auto pLU = pThis->languageManager->GetLanguageUtility(kCurrentLanguageID);

		if (!pLU->GetThousandSeparator(thousandsSeparator))
		{
			thousandsSeparator.FromChar(",", 1);
		}
	}
	else
	{
		thousandsSeparator.FromChar(",", 1);
	}

	LuaIGZCommandParameterSetHelper::SetValue(
		pOutput,
		1,
		thousandsSeparator);
	return true;
}

bool SC4GameLanguageTable::sGetDateString(
	void* pKey,
	cIGZCommandParameterSet* pInput,
	cIGZCommandParameterSet* pOutput,
	cISC4GameDataRegistry* pRegistry)
{
	cRZBaseString result("Error");

	auto entry = pRegistry->GetEntryID(pKey);
	auto pThis = static_cast<SC4GameLanguageTable*>(pRegistry->GetEntryContext(entry));

	int32_t month = 0;
	int32_t day = 0;
	int32_t year = 0;

	if (pThis->languageManager
		&& LuaIGZCommandParameterSetHelper::GetNumber(pInput, 0, month)
		&& LuaIGZCommandParameterSetHelper::GetNumber(pInput, 1, day)
		&& LuaIGZCommandParameterSetHelper::GetNumber(pInput, 2, year))
	{
		cRZBaseString formatted;

		auto pLU = pThis->languageManager->GetLanguageUtility(kCurrentLanguageID);

		if (pLU->MakeDateString(month, day, year, formatted, 0))
		{
			result = std::move(formatted);
		}
	}

	LuaIGZCommandParameterSetHelper::SetValue(
		pOutput,
		1,
		result);
	return true;
}

bool SC4GameLanguageTable::sGetMoneyString(
	void* pKey,
	cIGZCommandParameterSet* pInput,
	cIGZCommandParameterSet* pOutput,
	cISC4GameDataRegistry* pRegistry)
{
	cRZBaseString result("Error");

	auto entry = pRegistry->GetEntryID(pKey);
	auto pThis = static_cast<SC4GameLanguageTable*>(pRegistry->GetEntryContext(entry));

	double number = 0.0;

	if (pThis->languageManager
		&& LuaIGZCommandParameterSetHelper::GetNumber(pInput, 0, number))
	{
		auto pLU = pThis->languageManager->GetLanguageUtility(kCurrentLanguageID);

		int64_t numberAsInt64 = 0;
		cRZBaseString temp;

		if (SafeCast(round(number), numberAsInt64))
		{
			const cRZBaseString currencySymbol = GetCurrencySymbol();

			if (pLU->MakeMoneyString(
				numberAsInt64,
				temp,
				&currencySymbol))
			{
				if (temp.Strlen() > 0)
				{
					result = std::move(temp);
				}
			}
		}
	}

	LuaIGZCommandParameterSetHelper::SetValue(
		pOutput,
		1,
		result);
	return true;
}

bool SC4GameLanguageTable::sGetNumberString(
	void* pKey,
	cIGZCommandParameterSet* pInput,
	cIGZCommandParameterSet* pOutput,
	cISC4GameDataRegistry* pRegistry)
{
	cRZBaseString result("Error");

	auto entry = pRegistry->GetEntryID(pKey);
	auto pThis = static_cast<SC4GameLanguageTable*>(pRegistry->GetEntryContext(entry));

	double number = 0.0;

	if (pThis->languageManager
		&& LuaIGZCommandParameterSetHelper::GetNumber(pInput, 0, number))
	{
		auto pLU = pThis->languageManager->GetLanguageUtility(kCurrentLanguageID);

		cRZBaseString temp = FormatNumberString(pLU, number);

		if (temp.Strlen() > 0)
		{
			result = std::move(temp);
		}
	}

	LuaIGZCommandParameterSetHelper::SetValue(
		pOutput,
		1,
		result);
	return true;
}

bool SC4GameLanguageTable::sGetFormattedNumberString(
	void* pKey,
	cIGZCommandParameterSet* pInput,
	cIGZCommandParameterSet* pOutput,
	cISC4GameDataRegistry* pRegistry)
{
	cRZBaseString result("Error");

	auto entry = pRegistry->GetEntryID(pKey);
	auto pThis = static_cast<SC4GameLanguageTable*>(pRegistry->GetEntryContext(entry));

	double number = 0.0;
	uint32_t decimalPlaces = 0;

	if (pThis->languageManager
		&& LuaIGZCommandParameterSetHelper::GetNumber(pInput, 0, number)
		&& LuaIGZCommandParameterSetHelper::GetNumber(pInput, 1, decimalPlaces))
	{
		auto pLU = pThis->languageManager->GetLanguageUtility(kCurrentLanguageID);

		cRZBaseString temp = FormatNumberString(pLU, number, decimalPlaces);

		if (temp.Strlen() > 0)
		{
			result = std::move(temp);
		}
	}

	LuaIGZCommandParameterSetHelper::SetValue(
		pOutput,
		1,
		result);
	return true;
}
