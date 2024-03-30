// Portions of this file have been adapted from the nam-dll project:
/*
 * Copyright (c) 2023 NAM Team contributors
 *
 * nam-dll is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * nam-dll is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with nam-dll.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Logger.h"
#include "SC4VersionDetection.h"
#include "version.h"
#include "cIGZCOM.h"
#include "cRZCOMDllDirector.h"
#include "cIGZMessage2.h"
#include "cIGZMessage2Standard.h"
#include "cRZMessage2COMDirector.h"
#include "cIGZMessageServer2.h"
#include "GZServPtrs.h"
#include "cIGZCheatCodeManager.h"
#include "cISC4App.h"
#include "cISC43DRender.h"
#include "cIGZWin.h"
#include "cISC4View3DWin.h"
#include <Windows.h>
#include "wil/resource.h"
#include "wil/win32_helpers.h"
#include <numbers>
#include <vector>
#include <string>
#include "StringViewUtil.h"
/* #include <iostream> */


#ifdef __clang__
#define NAKED_FUN __attribute__((naked))
#else
#define NAKED_FUN __declspec(naked)
#endif


static constexpr uint32_t kGZWin_WinSC4App = 0x6104489a;
static constexpr uint32_t kGZWin_SC4View3DWin = 0x9a47b417;
static constexpr uint32_t kGZIID_cISC4View3DWin = 0xFA47B3F9;

static constexpr uint32_t kMessageCheatIssued = 0x230E27AC;
static constexpr uint32_t kSC4MessagePostCityInit = 0x26D31EC1;
static constexpr uint32_t kSC4MessagePreCityShutdown = 0x26D31EC2;

static constexpr uint32_t kPitchAngleCheatID = 0x4A3A1EF5;
static constexpr uint32_t kYawAngleCheatID = 0x6E88F96F;
static constexpr uint32_t kCameraDllDirectorID = 0x23B41621;

static constexpr std::string_view PluginLogFileName = "memo.3dcamera.log";

static constexpr uint32_t UpdateCameraZoomAndRotationParams_InjectPoint = 0x7cbeaa;
static constexpr uint32_t UpdateCameraZoomAndRotationParams_ContinueJump = 0x7cbeb0;
static constexpr uint32_t cSC4CameraControl_UpdateCameraPosition = 0x7ccf80;

static constexpr uint32_t yawAddress0 = 0x7ccb0a;
static constexpr uint32_t yawAddress1 = 0xabcfc4;
static constexpr uint32_t yawAddress2 = 0xabacb8;
static constexpr uint32_t pitchAddress1 = 0xabcfd8;
static constexpr uint32_t pitchAddress2 = 0xabaccc;

static constexpr int numZooms = 5;
static const float_t pitchRadDefault[numZooms] = {0.52359879f, 0.61086524f, 0.69813168f, 0.78539819f, 0.78539819f};
static constexpr float_t yawRadDefault = -0.39269909;
static float_t sYawRad = yawRadDefault;
static constexpr float_t tol = 0.001;
static bool sInstalledPatches = false;

namespace
{
	float_t deg2rad(float_t deg)
	{
		// to avoid some graphical glitches at 0 and 90 degrees, we slightly offset the angle
		if (-tol < deg && deg < tol) {
			deg = 0 + tol;
		} else if (90 - tol < deg && deg < 90 + tol) {
			deg = 90 - tol;
		}
		return deg * (std::numbers::pi / 180);
	}

	float_t clip(float_t x, float_t xMin, float_t xMax)
	{
		return x < xMin ? xMin : (x > xMax ? xMax : x);
	}

	std::filesystem::path GetDllFolderPath()
	{
		wil::unique_cotaskmem_string modulePath = wil::GetModuleFileNameW(wil::GetModuleInstanceHandle());

		std::filesystem::path temp(modulePath.get());

		return temp.parent_path();
	}

	void OverwriteMemoryFloat(void* address, float_t newValue)
	{
		static_assert(sizeof(float_t) == 4);  // 32 bit == 4 byte
		DWORD oldProtect;
		// Allow the executable memory to be written to.
		THROW_IF_WIN32_BOOL_FALSE(VirtualProtect(
			address,
			sizeof(newValue),
			PAGE_EXECUTE_READWRITE,
			&oldProtect));

		// Patch the memory at the specified address.
		*((float_t*)address) = newValue;
	}

	void InstallHook(uint32_t address, void (*pfnFunc)(void))
	{
		DWORD oldProtect;
		THROW_IF_WIN32_BOOL_FALSE(VirtualProtect((void *)address, 5, PAGE_EXECUTE_READWRITE, &oldProtect));

		*((uint8_t*)address) = 0xE9;
		*((uint32_t*)(address+1)) = ((uint32_t)pfnFunc) - address - 5;
	}

	void Call_UpdateCameraPosition(cSC4CameraControl* cameraControl, uint32_t updateMode)
	{
		__asm {
			push dword ptr [updateMode];
			mov ecx, dword ptr [cameraControl];  // (this)
			mov eax, dword ptr [cSC4CameraControl_UpdateCameraPosition];
			call eax;  // (thiscall)
		}
	}

	void NAKED_FUN Hook_UpdateCameraZoomAndRotationParams(void)
	{
		__asm {
			fadd dword ptr [sYawRad];
			push UpdateCameraZoomAndRotationParams_ContinueJump;
			ret;
		}
	}

	void InstallPatches()
	{
		Logger& logger = Logger::GetInstance();
		try
		{
			InstallHook(UpdateCameraZoomAndRotationParams_InjectPoint, Hook_UpdateCameraZoomAndRotationParams);
			logger.WriteLine(
				LogLevel::Info,
				"Installed 3D-Camera patch.");
		}
		catch (const wil::ResultException& e)
		{
			logger.WriteLineFormatted(
				LogLevel::Error,
				"Failed to install the 3D-Camera patch.\n%s",
				e.what());
		}
	}
}

class CameraDllDirector final : public cRZMessage2COMDirector
{
public:

	CameraDllDirector()
	{
		std::filesystem::path dllFolderPath = GetDllFolderPath();

		std::filesystem::path logFilePath = dllFolderPath;
		logFilePath /= PluginLogFileName;

		Logger& logger = Logger::GetInstance();
		logger.Init(logFilePath, LogLevel::Error);
		logger.WriteLogFileHeader("3D Camera DLL " PLUGIN_VERSION_STR);
	}

	uint32_t GetDirectorID() const
	{
		return kCameraDllDirectorID;
	}

	void PostCityInit()
	{
		Logger& logger = Logger::GetInstance();
		cISC4AppPtr pSC4App;
		if (pSC4App)
		{
			cIGZCheatCodeManager* pCheatMgr = pSC4App->GetCheatCodeManager();
			if (pCheatMgr)
			{
				pCheatMgr->AddNotification2(this, 0);
				pCheatMgr->RegisterCheatCode(kPitchAngleCheatID, cRZBaseString("CameraPitch"));
				pCheatMgr->RegisterCheatCode(kYawAngleCheatID, cRZBaseString("CameraYaw"));
			}
			else
			{
				logger.WriteLine(LogLevel::Error, "The cheat manager pointer was null.");
			}
		}
	}

	void PreCityShutdown()
	{
		cISC4AppPtr pSC4App;
		if (pSC4App)
		{
			cIGZCheatCodeManager* pCheatMgr = pSC4App->GetCheatCodeManager();
			if (pCheatMgr)
			{
				pCheatMgr->UnregisterCheatCode(kPitchAngleCheatID);
				pCheatMgr->UnregisterCheatCode(kYawAngleCheatID);
				pCheatMgr->RemoveNotification2(this, 0);
			}
		}
	}

	void ShowUsage()
	{
		Logger& logger = Logger::GetInstance();
		logger.WriteLine(LogLevel::Info, "Usage:\n  CameraYaw [angle] (defaults to 22.5)\n  CameraPitch [angle1] [angle2] .. [angle5] (defaults to previous zoom's pitch angle or game default)");
	}

	// parses a number of floats
	bool ParseCheatString(const std::string_view& cheatString, std::vector<float_t>* values, int maxCount)
	{
		std::vector<std::string_view> arguments;
		arguments.reserve(maxCount + 1);
		StringViewUtil::Split(cheatString, ' ', arguments);

		if (arguments.size() >= 1 && arguments.size() <= maxCount + 1) {
			for (int i = 1; i < arguments.size(); i++) {
				char* end = nullptr;
				float_t arg = std::strtof(arguments[i].data(), &end);
				if (end == arguments[i].data()) {
					ShowUsage();
					return false;
				} else {
					values->push_back(arg);
				}
			}
			return true;
		} else {
			ShowUsage();
			return false;
		}
	}

	void UpdateCamera()
	{
		cISC4AppPtr pSC4App;
		if (pSC4App) {
			cIGZWin* mainWindow = pSC4App->GetMainWindow();
			if (mainWindow) {
				cIGZWin* pParentWin = mainWindow->GetChildWindowFromID(kGZWin_WinSC4App);
				if (pParentWin) {
					cISC4View3DWin* pView3D = nullptr;
					if (pParentWin->GetChildAs(kGZWin_SC4View3DWin, kGZIID_cISC4View3DWin, reinterpret_cast<void**>(&pView3D))) {
						cISC43DRender* renderer = pView3D->GetRenderer();
						if (renderer) {
							cSC4CameraControl* cameraControl = renderer->GetCameraControl();
							if (cameraControl) {
								uint32_t updateMode = 2;
								Call_UpdateCameraPosition(cameraControl, updateMode);
							} else {
								Logger& logger = Logger::GetInstance();
								logger.WriteLine(LogLevel::Error, "Failed to obtain camera. Manually change zoom/rotation instead to update view.");
							}
						}
						pView3D->Release();
					}
				}
			}
		}
	}

	void ProcessCheat(cIGZMessage2Standard* pStandardMsg)
	{
		if (!sInstalledPatches) {
			InstallPatches();
			sInstalledPatches = true;
		}

		uint32_t cheatID = static_cast<uint32_t>(pStandardMsg->GetData1());
		if (cheatID == kPitchAngleCheatID)
		{
			cIGZString* cheatStr = static_cast<cIGZString*>(pStandardMsg->GetVoid2());
			std::vector<float_t> angles;
			if (ParseCheatString(cheatStr->ToChar(), &angles, numZooms)) {
				for (int i = 0; i < numZooms; i++) {
					float_t pitchRad = angles.empty()
						? pitchRadDefault[i]
						: deg2rad(clip(i < angles.size() ? angles[i] : angles.back(), 1.0f, 90.0f-tol));  // for missing angles, re-use last angle
					OverwriteMemoryFloat((void*)(pitchAddress1 + i*4), pitchRad);
					OverwriteMemoryFloat((void*)(pitchAddress2 + i*4), pitchRad);
				}
				UpdateCamera();
			}  // else invalid cheat arguments
		}
		else if (cheatID == kYawAngleCheatID)
		{
			cIGZString* cheatStr = static_cast<cIGZString*>(pStandardMsg->GetVoid2());
			std::vector<float_t> angles;
			if (ParseCheatString(cheatStr->ToChar(), &angles, 1)) {
				sYawRad = angles.size() >= 1 ? -deg2rad(angles[0]) : yawRadDefault;
				OverwriteMemoryFloat((void*)yawAddress0, sYawRad);
				for (int i = 0; i < 5; i++) {
					OverwriteMemoryFloat((void*)(yawAddress1 + i*4), sYawRad);
					OverwriteMemoryFloat((void*)(yawAddress2 + i*4), sYawRad);
				}
				UpdateCamera();
			}  // else invalid cheat arguments
		}
	}

	bool DoMessage(cIGZMessage2* pMsg)
	{
		cIGZMessage2Standard* pStandardMessage = static_cast<cIGZMessage2Standard*>(pMsg);
		uint32_t msgType = pStandardMessage->GetType();

		switch (msgType)
		{
		case kSC4MessagePostCityInit:
			PostCityInit();
			break;
		case kMessageCheatIssued:
			ProcessCheat(pStandardMessage);
			break;
		case kSC4MessagePreCityShutdown:
			PreCityShutdown();
			break;
		}

		return true;
	}

	bool PostAppInit()
	{
		Logger& logger = Logger::GetInstance();
		cIGZMessageServer2Ptr pMsgServ;
		if (pMsgServ)
		{
			std::vector<uint32_t> requiredNotifications;
			requiredNotifications.push_back(kSC4MessagePostCityInit);
			requiredNotifications.push_back(kMessageCheatIssued);
			requiredNotifications.push_back(kSC4MessagePreCityShutdown);

			for (uint32_t messageID : requiredNotifications)
			{
				if (!pMsgServ->AddNotification(this, messageID))
				{
					logger.WriteLine(LogLevel::Error, "Failed to subscribe to the required notifications.");
					return false;
				}
			}
		}
		else
		{
			logger.WriteLine(LogLevel::Error, "Failed to subscribe to the required notifications.");
			return false;
		}

		return true;
	}

	bool OnStart(cIGZCOM* pCOM)
	{
		const uint16_t gameVersion = versionDetection.GetGameVersion();
		if (gameVersion == 641)
		{
			cIGZFrameWork* const pFramework = RZGetFrameWork();
			if (pFramework->GetState() < cIGZFrameWork::kStatePreAppInit) {
				pFramework->AddHook(this);
			} else {
				PreAppInit();
			}
		}
		else
		{
			Logger& logger = Logger::GetInstance();
			logger.WriteLineFormatted(
				LogLevel::Error,
				"Requires game version 641, found game version %d.",
				gameVersion);
		}
		return true;
	}

private:

	const SC4VersionDetection versionDetection;
};

cRZCOMDllDirector* RZGetCOMDllDirector() {
	static CameraDllDirector sDirector;
	return &sDirector;
}