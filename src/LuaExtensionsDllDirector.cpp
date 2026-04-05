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

#include "version.h"
#include "FileSystem.h"
#include "Logger.h"
#include "cIGZApp.h"
#include "cIGZCOM.h"
#include "cIGZFrameWork.h"
#include "cIGZMessageServer2.h"
#include "cIGZMessageTarget.h"
#include "cIGZMessageTarget2.h"
#include "cIGZString.h"
#include "cIGZWin.h"
#include "cISC4AdvisorSystem.h"
#include "cISC4App.h"
#include "cISC4City.h"
#include "cISCLua.h"
#include "cRZMessage2COMDirector.h"
#include "cRZMessage2Standard.h"
#include "cRZBaseString.h"
#include "DebugUtil.h"
#include "GlobalPointers.h"
#include "GZServPtrs.h"
#include "LuaErrorReporting.h"
#include "LuaPrintFunction.h"
#include "PackageScriptCompilationCallbackServer.h"
#include "PackageScriptLoadingPatch.h"
#include "SC4VersionDetection.h"

#include "DBPFTableFunctions.h"
#include "GameTableExtensions.h"
#include "SC4GameBudgetTableExtensions.h"
#include "SC4GameCameraTable.h"
#include "SC4GameCityTable.h"
#include "SC4GameLanguageTable.h"
#include "LuaExtensionFunctionTests.h"

#include <array>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>


static constexpr uint32_t kSC4MessagePostCityInit = 0x26D31EC1;
static constexpr uint32_t kSC4MessagePreCityShutdown = 0x26D31EC2;
static constexpr uint32_t kSC4MessagePostRegionInit = 0xCBB5BB45;
static constexpr uint32_t kSC4MessagePreRegionShutdown = 0x8BB5BB46;
static constexpr uint32_t kMessageCheatIssued = 0x230E27AC;

// Only used to run the Lua testing code in debug builds.
static constexpr uint32_t kSC4MessagePostCityInitComplete = 0xEA8AE29A;

static constexpr std::array<uint32_t, 2> RequiredNotifications =
{
	kSC4MessagePostCityInit,
	kSC4MessagePreCityShutdown,
};

static constexpr uint32_t kLuaExtensionsDirectorID = 0x68AC07EE;

static constexpr uint32_t kShowMaxisLuaDebugTextCheatID = 0x519E8A25;
static constexpr std::string_view kShowMaxisLuaDebugTextCheatStr = "ShowMaxisLuaDebugText";

static constexpr uint32_t kHideMaxisLuaDebugTextCheatID = 0x519E8A26;
static constexpr std::string_view kHideMaxisLuaDebugTextCheatStr = "HideMaxisLuaDebugText";

cIGZCheatCodeManager* spCCM = nullptr;
cRZAutoRefCount<cIGZCommandServer> spCommandServer;
cRZAutoRefCount<cIGZPersistResourceManager> spResourceManager;
cISC4BudgetSimulator* spBudgetSim = nullptr;
cISC4City* spCity = nullptr;
cISC4Simulator* spSimulator = nullptr;
cRZAutoRefCount<cISC4View3DWin> spView3D;

namespace
{
	cRZAutoRefCount<cISC4View3DWin> GetView3DWin()
	{
		constexpr uint32_t kGZWin_WinSC4App = 0x6104489a;
		constexpr uint32_t kGZWin_SC4View3DWin = 0x9a47b417;
		constexpr uint32_t kGZIID_cISC4View3DWin = 0xFA47B3F9;

		cRZAutoRefCount<cISC4View3DWin> view3D;

		cISC4AppPtr sc4App;

		cIGZWin* mainWindow = sc4App->GetMainWindow();

		if (mainWindow)
		{
			cIGZWin* pParentWin = mainWindow->GetChildWindowFromID(kGZWin_WinSC4App);

			if (pParentWin)
			{
				if (!pParentWin->GetChildAs(
					kGZWin_SC4View3DWin,
					kGZIID_cISC4View3DWin,
					view3D.AsPPVoid()))
				{
					view3D.Reset();
				}
			}
		}

		return view3D;
	}

#ifdef _DEBUG
	void DebugLogCompiledScriptTGI(cGZPersistResourceKey const& key, void* pContext)
	{
		DebugUtil::PrintLineToDebugOutputFormatted("Script [0x%08X, 0x%08X, 0x%08X] loaded.", key.type, key.group, key.instance);
	}
#endif // _DEBUG
}

class LuaExtensionsDllDirector : public cRZMessage2COMDirector
{
public:
	LuaExtensionsDllDirector()
		: replacedMaxisPrintFunctionPointers(false),
		  installedPackageScriptLoadingPatch(false),
		  addedScriptCompilationCallback(false)
	{
		std::filesystem::path logFilePath = FileSystem::GetPluginLogFilePath();

		Logger& logger = Logger::GetInstance();
		logger.Init(logFilePath, LogLevel::Error);
		logger.WriteLogFileHeader("SC4LuaExtensions v" PLUGIN_VERSION_STR);
	}

	uint32_t GetDirectorID() const
	{
		return kLuaExtensionsDirectorID;
	}

	void EnumClassObjects(ClassObjectEnumerationCallback pCallback, void* pContext)
	{
		if (SC4VersionDetection::IsVersion641())
		{
			pCallback(GZCLSID_PackageScriptCompilationCallbackServer, 0, pContext);
		}
	}

	bool GetClassObject(uint32_t rclsid, uint32_t riid, void** ppvObj)
	{
		bool result = false;

		if (rclsid == GZCLSID_PackageScriptCompilationCallbackServer && installedPackageScriptLoadingPatch)
		{
			result = packageScriptCompilationCallbackServer.QueryInterface(riid, ppvObj);
		}

		return result;
	}

	void PostCityInit(cIGZMessage2Standard* pStandardMsg)
	{
		spCity = static_cast<cISC4City*>(pStandardMsg->GetVoid1());

		if (spCity)
		{
			spBudgetSim = spCity->GetBudgetSimulator();
			spSimulator = spCity->GetSimulator();
			spView3D = GetView3DWin();

			if (!addedScriptCompilationCallback)
			{
				RegisterNativeLuaFunctions(spCity->GetAdvisorSystem()->GetScriptingContext());
			}
		}
	}

	void PostCityInitComplete()
	{
		// The testing code has to be run in PostCityInitComplete because cISC4View3DWin
		// crashes with a null pointer error when rotating the camera in PostCityInit.

		if (spCity)
		{
			LuaExtensionFunctionTests::Run(spCity->GetAdvisorSystem()->GetScriptingContext());
		}
	}

	void PreCityShutdown()
	{
		spBudgetSim = nullptr;
		spSimulator = nullptr;
		spCity = nullptr;
		spView3D.Reset();
	}

	void PostRegionInit()
	{
		if (spCCM)
		{
			spCCM->AddNotification2(this, 0);
			spCCM->RegisterCheatCode(
				kShowMaxisLuaDebugTextCheatID,
				cRZBaseString(kShowMaxisLuaDebugTextCheatStr.data(), kShowMaxisLuaDebugTextCheatStr.size()));
			spCCM->RegisterCheatCode(
				kHideMaxisLuaDebugTextCheatID,
				cRZBaseString(kHideMaxisLuaDebugTextCheatStr.data(), kHideMaxisLuaDebugTextCheatStr.size()));
		}
	}

	void PreRegionShutdown()
	{
		if (spCCM)
		{
			spCCM->RemoveNotification2(this, 0);
			spCCM->UnregisterCheatCode(kShowMaxisLuaDebugTextCheatID);
			spCCM->UnregisterCheatCode(kHideMaxisLuaDebugTextCheatID);
		}
	}

	void ProcessCheat(cIGZMessage2Standard* pStandardMsg)
	{
		const uint32_t cheatID = static_cast<uint32_t>(pStandardMsg->GetData1());

		switch (cheatID)
		{
		case kShowMaxisLuaDebugTextCheatID:
			LuaPrintFunction::SetPrintMaxisScriptDebugOutput(true);
			break;
		case kHideMaxisLuaDebugTextCheatID:
			LuaPrintFunction::SetPrintMaxisScriptDebugOutput(false);
			break;
		}
	}

	bool DoMessage(cIGZMessage2* pMessage)
	{
		switch (pMessage->GetType())
		{
		case kSC4MessagePostCityInit:
			PostCityInit(static_cast<cIGZMessage2Standard*>(pMessage));
			break;
#ifdef _DEBUG
		case kSC4MessagePostCityInitComplete:
			PostCityInitComplete();
			break;
#endif // _DEBUG
		case kSC4MessagePreCityShutdown:
			PreCityShutdown();
			break;
		case kSC4MessagePostRegionInit:
			PostRegionInit();
			break;
		case kSC4MessagePreRegionShutdown:
			PreRegionShutdown();
			break;
		case kMessageCheatIssued:
			ProcessCheat(static_cast<cIGZMessage2Standard*>(pMessage));
			break;
		}

		return true;
	}

	bool PostAppInit()
	{
		cIGZMessageServer2Ptr pMS2;

		if (pMS2)
		{
			for (const uint32_t& messageID : RequiredNotifications)
			{
				pMS2->AddNotification(this, messageID);
			}

			if (replacedMaxisPrintFunctionPointers)
			{
				// Subscribe to notifications for the region-view cheat codes
				// that control weather the Maxis debug text is included in
				// the log's Lua print output.
				pMS2->AddNotification(this, kSC4MessagePostRegionInit);
				pMS2->AddNotification(this, kSC4MessagePreRegionShutdown);
			}

#ifdef _DEBUG
			pMS2->AddNotification(this, kSC4MessagePostCityInitComplete);
#endif // _DEBUG

		}

		cIGZApp* const pApp = mpFrameWork->Application();

		if (pApp)
		{
			cRZAutoRefCount<cISC4App> sc4App;

			if (pApp->QueryInterface(GZIID_cISC4App, sc4App.AsPPVoid()))
			{
				spCCM = sc4App->GetCheatCodeManager();

				// The city advisor/automata scripting system uses the
				// cISCLua instance owned by cISC4App.
				// We take advantage of that and add error logging to help
				// mod authors that write Lua scripts.

				cISCLua* pLua = sc4App->GetLuaInterpreter();

				if (pLua)
				{
					LuaErrorReporting::InstallErrorLoggingCallback(pLua->AsIGZLua5());
				}
			}
		}

		if (installedPackageScriptLoadingPatch)
		{
#ifdef _DEBUG
			packageScriptCompilationCallbackServer.AddAnyScriptCompiledCallback(DebugLogCompiledScriptTGI, nullptr);
#endif // _DEBUG

			// Request a callback when the script that defines the Lua prototypes for our native
			// functions is compiled.
			// This is required to allow dependent Lua scripts to use our native functions to
			// initialize their global variables, which is performed when the script is compiled.
			// For example, some scripts use dbpf.get_exemplar_property_value to initialize
			// global variables from exemplar properties.
			//
			// If the callback could not be registered, the native functions will be initialized in PostCityInit.
			addedScriptCompilationCallback = packageScriptCompilationCallbackServer.AddTargetScriptCompiledCallback(
				cGZPersistResourceKey(0xCA63E2A3, 0x4A5E8EF6, 0xAE36ED6A),
				RegisterNativeLuaFunctionsStatic,
				this);
		}

		cIGZCommandServerPtr commandServer;
		spCommandServer = commandServer;
		cIGZPersistResourceManagerPtr resourceManager;
		spResourceManager = resourceManager;

		sc4gameLanguageTable.PostAppInit(mpFrameWork);

		return true;
	}

	bool PreAppShutdown()
	{
		if (mpFrameWork)
		{
			cIGZApp* const pApp = mpFrameWork->Application();

			if (pApp)
			{
				cRZAutoRefCount<cISC4App> sc4App;

				if (pApp->QueryInterface(GZIID_cISC4App, sc4App.AsPPVoid()))
				{
					// The city advisor/automata scripting system uses the
					// cISCLua instance owned by cISC4App.
					// We take advantage of that and add error logging to help
					// mod authors that write Lua scripts.

					cISCLua* pLua = sc4App->GetLuaInterpreter();

					if (pLua)
					{
						LuaErrorReporting::UninstallErrorLoggingCallback(pLua->AsIGZLua5());
					}
				}
			}
		}

		spCCM = nullptr;
		spCommandServer.Reset();
		spResourceManager.Reset();
		sc4gameLanguageTable.PreAppShutdown();

		return true;
	}

	bool OnStart(cIGZCOM * pCOM)
	{
		replacedMaxisPrintFunctionPointers = LuaPrintFunction::PatchMaxisPrintFunctionPointers();
		installedPackageScriptLoadingPatch = PackageScriptLoadingPatch::Install();

		const cIGZFrameWork::FrameworkState state = mpFrameWork->GetState();

		if (state < cIGZFrameWork::kStatePreAppInit)
		{
			mpFrameWork->AddHook(this);
		}
		else
		{
			PreAppInit();
		}
		return true;
	}

	void RegisterNativeLuaFunctions(cISCLua* pLua)
	{
		if (pLua)
		{
			if (!replacedMaxisPrintFunctionPointers)
			{
				LuaPrintFunction::RegisterFallbackPrintFunction(pLua);
			}

			DBPFTableFunctions::Register(pLua);
			GameTableExtensions::Register(pLua);
			sc4gameBudgetTable.Register(*pLua);
		}

		// The GameDataRegistry class that is used to register the sc4game methods appears
		// to have been intended as a way for C++ classes to add lua functions, it allows
		// a pointer to the class to be stored as user data for the function.

		cISC4GameDataRegistryPtr registry;

		if (registry)
		{
			sc4gameCameraTable.Register(*registry);
			sc4gameCityTable.Register(*registry);
			sc4gameLanguageTable.Register(*registry);
		}
	}

	static void RegisterNativeLuaFunctionsStatic(cGZPersistResourceKey const&, cISCLua* pLua, void* pContext)
	{
		static_cast<LuaExtensionsDllDirector*>(pContext)->RegisterNativeLuaFunctions(pLua);
	}

	SC4GameBudgetTableExtensions sc4gameBudgetTable;
	SC4GameCameraTable sc4gameCameraTable;
	SC4GameCityTable sc4gameCityTable;
	SC4GameLanguageTable sc4gameLanguageTable;
	PackageScriptCompilationCallbackServer packageScriptCompilationCallbackServer;
	bool replacedMaxisPrintFunctionPointers;
	bool installedPackageScriptLoadingPatch;
	bool addedScriptCompilationCallback;
};

cRZCOMDllDirector* RZGetCOMDllDirector() {
	static LuaExtensionsDllDirector sDirector;
	return &sDirector;
}