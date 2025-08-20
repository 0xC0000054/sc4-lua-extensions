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

#include "SC4GameBudgetTableExtensions.h"
#include "cISC4DepartmentBudget.h"
#include "GlobalPointers.h"
#include "Logger.h"
#include "LuaFunctionRegistration.h"
#include "LuaHelper.h"
#include "SCLuaUtil.h"

namespace
{
	int32_t get_department_total_expense(lua_State* pState)
	{
		int64_t total = 0;

		cRZAutoRefCount<cISCLua> lua = SCLuaUtil::GetISCLuaFromFunctionState(pState);

		const int32_t parameterCount = lua->GetTop();

		if (parameterCount == 1)
		{
			uint32_t id = 0;

			if (LuaHelper::GetNumber(lua, -1, id))
			{
				const cISC4DepartmentBudget* pDepartment = spBudgetSim->GetDepartmentBudget(id);

				if (pDepartment)
				{
					total = pDepartment->GetTotalExpenses();
				}
			}
		}

		lua->PushNumber(static_cast<double>(total));
		return 1;
	}

	int32_t get_department_total_income(lua_State* pState)
	{
		int64_t total = 0;

		cRZAutoRefCount<cISCLua> lua = SCLuaUtil::GetISCLuaFromFunctionState(pState);

		const int32_t parameterCount = lua->GetTop();

		if (parameterCount == 1)
		{
			uint32_t id = 0;

			if (LuaHelper::GetNumber(lua, -1, id))
			{
				const cISC4DepartmentBudget* pDepartment = spBudgetSim->GetDepartmentBudget(id);

				if (pDepartment)
				{
					total = pDepartment->GetTotalIncome();
				}
			}
		}

		lua->PushNumber(static_cast<double>(total));
		return 1;
	}
}

SC4GameBudgetTableExtensions::SC4GameBudgetTableExtensions()
	: SC4GameTableSCLuaBase("budget")
{
}

std::vector<LuaFunctionRegistration::LuaFunctionInfo> SC4GameBudgetTableExtensions::GetTableFunctions() const
{
	std::vector<LuaFunctionRegistration::LuaFunctionInfo> functions =
	{
		LuaFunctionRegistration::LuaFunctionInfo("get_department_total_expense", get_department_total_expense),
		LuaFunctionRegistration::LuaFunctionInfo("get_department_total_income", get_department_total_income),
	};

	return functions;
}
