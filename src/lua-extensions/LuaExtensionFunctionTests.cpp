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

#include "LuaExtensionFunctionTests.h"
#include "cISCLua.h"
#include "cISCStringDetokenizer.h"
#include "DebugUtil.h"
#include "GlobalPointers.h"
#include <array>
#include <string_view>

namespace
{
	void RunTestFunction(cIGZLua5Thread* pIGZLua5Thread, const char* functionName)
	{
		int32_t top = pIGZLua5Thread->GetTop();

		pIGZLua5Thread->GetGlobal(functionName);

		int32_t functionTop = pIGZLua5Thread->GetTop();

		if (functionTop != top)
		{
			if (pIGZLua5Thread->IsFunction(-1))
			{
				// The test functions take no parameters and return a string.

				int32_t status = pIGZLua5Thread->CallProtected(0, 1, 0, false);

				if (status == 0)
				{
					if (pIGZLua5Thread->IsString(-1))
					{
						const char* result = pIGZLua5Thread->ToString(-1);

						DebugUtil::PrintLineToDebugOutputFormattedUTF8(
							"%s() returned: %s",
							functionName,
							result ? result : "<null>");
					}
					else
					{
						DebugUtil::PrintLineToDebugOutputFormatted(
							"%s() did not return a string.",
							functionName);
					}
				}
				else
				{
					const char* errorString = pIGZLua5Thread->ToString(-1);

					DebugUtil::PrintLineToDebugOutputFormatted(
						"Error status code %d returned when calling %s(), error text: %s.",
						status,
						functionName,
						errorString ? errorString : "");

					// Pop the error string off the stack.
					pIGZLua5Thread->Pop(1);
				}
			}
			else
			{
				DebugUtil::PrintLineToDebugOutputFormatted(
					"%s is not a function, actual type: %s.",
					functionName,
					pIGZLua5Thread->TypeName(-1));
			}
		}
		else
		{
			DebugUtil::PrintLineToDebugOutputFormatted(
				"%s() does not exist.",
				functionName);
		}

		pIGZLua5Thread->SetTop(top);
	}
}


void LuaExtensionFunctionTests::Run(cISCLua* pLua)
{
	constexpr std::array<const char*, 29> LuaTestFunctions =
	{
		"null45_lua_extensions_test_dbpf_get_cohort_property_value",
		"null45_lua_extensions_test_dbpf_get_exemplar_property_value",
		"null45_lua_extensions_test_dbpf_resource_exists",
		"null45_lua_extensions_test_execute_cheat",
		"null45_lua_extensions_test_execute_command",
		"null45_lua_extensions_test_pause",
		"null45_lua_extensions_test_resume",
		"null45_lua_extensions_test_print",
		"null45_lua_extensions_test_get_budget_department_total_expense",
		"null45_lua_extensions_test_get_budget_department_total_income",
		"null45_lua_extensions_test_is_left_hand_drive",
		"null45_lua_extensions_test_are_first_and_last_names_reversed",
		"null45_lua_extensions_test_get_currency_symbol",
		"null45_lua_extensions_test_does_currency_symbol_precede_amount",
		"null45_lua_extensions_test_is_space_between_currency_symbol_and_amount",
		"null45_lua_extensions_test_get_decimal_separator",
		"null45_lua_extensions_test_get_thousands_separator",
		"null45_lua_extensions_test_get_date_string",
		"null45_lua_extensions_test_get_money_string",
		"null45_lua_extensions_test_get_formatted_number_string",
		"null45_lua_extensions_test_get_number_string",
		"null45_lua_extensions_test_get_set_city_name",
		"null45_lua_extensions_test_get_set_mayor_name",
		"null45_lua_extensions_test_place_lot",
		"null45_lua_extensions_test_get_set_camera_current_cell",
		"null45_lua_extensions_test_get_set_camera_current_position",
		"null45_lua_extensions_test_camera_rotation",
		"null45_lua_extensions_test_camera_take_snapshot_1",
		"null45_lua_extensions_test_camera_take_snapshot_2",
	};

	cIGZLua5Thread* pIGZLua5Thread = pLua->AsIGZLua5()->AsIGZLua5Thread();

	for (const auto& item : LuaTestFunctions)
	{
		RunTestFunction(pIGZLua5Thread, item);
	}
}
