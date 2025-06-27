#include <4dm.h>
#include "Config.h"
#include <fstream>

using namespace fdm;

void updateConfig(const std::string& path, const nlohmann::json& j)
{
	std::ofstream configFileO(path);
	if (configFileO.is_open())
	{
		configFileO << j.dump(4);
		configFileO.close();
	}
}

// Read config
$hook(void, StateIntro, init, StateManager& s)
{
	

	configPath = std::format("{}/config.json", fdm::getModPath(fdm::modID));

	nlohmann::json configJson
	{
		{ "TitleScreenWorldRenderDistance", titleScreenWorldRenderDistance},
		{ "ShiftCraftCount", shiftCraftCount},
		{ "CtrlShiftCraftCount", ctrlShiftCraftCount },
		{ "CtrlCraftCount", ctrlCraftCount},
		{ "PopupsEnabled", popupsEnabled},
		{ "PopupLifeTime", popupLifeTime },
		{ "PopupFadeTime", popupFadeTime},
		{ "PopupMoveSpeed", popupMoveSpeed},
	};

	if (!std::filesystem::exists(configPath))
	{
		updateConfig(configPath, configJson);
	}
	else
	{
		std::ifstream configFileI(configPath);
		if (configFileI.is_open())
		{
			configJson = nlohmann::json::parse(configFileI);
			configFileI.close();
		}
	}

	if (!configJson.contains("TitleScreenWorldRenderDistance"))
	{
		configJson["TitleScreenWorldRenderDistance"] = titleScreenWorldRenderDistance;
		updateConfig(configPath, configJson);
	}
	if (!configJson.contains("ShiftCraftCount"))
	{
		configJson["ShiftCraftCount"] = shiftCraftCount;
		updateConfig(configPath, configJson);
	}
	if (!configJson.contains("CtrlShiftCraftCount"))
	{
		configJson["CtrlShiftCraftCount"] = ctrlShiftCraftCount;
		updateConfig(configPath, configJson);
	}
	if (!configJson.contains("CtrlCraftCount"))
	{
		configJson["CtrlCraftCount"] = ctrlCraftCount;
		updateConfig(configPath, configJson);
	}

	if (!configJson.contains("PopupsEnabled"))
	{
		configJson["PopupsEnabled"] = popupsEnabled;
		updateConfig(configPath, configJson);
	}
	if (!configJson.contains("PopupLifeTime"))
	{
		configJson["PopupLifeTime"] = popupLifeTime;
		updateConfig(configPath, configJson);
	}
	if (!configJson.contains("PopupFadeTime"))
	{
		configJson["PopupFadeTime"] = popupFadeTime;
		updateConfig(configPath, configJson);
	}
	if (!configJson.contains("PopupMoveSpeed"))
	{
		configJson["PopupMoveSpeed"] = popupMoveSpeed;
		updateConfig(configPath, configJson);
	}

	titleScreenWorldRenderDistance = configJson["TitleScreenWorldRenderDistance"];

	shiftCraftCount = configJson["ShiftCraftCount"];
	ctrlShiftCraftCount = configJson["CtrlShiftCraftCount"];
	ctrlCraftCount = configJson["CtrlCraftCount"];

	popupsEnabled = configJson["PopupsEnabled"];
	popupLifeTime = configJson["PopupLifeTime"];
	popupFadeTime = configJson["PopupFadeTime"];
	popupMoveSpeed = configJson["PopupMoveSpeed"];

	original(self, s);

	// initialize opengl stuff
	glewExperimental = true;
	glewInit();
	glfwInit();
}