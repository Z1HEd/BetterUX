#include <4dm.h>
#include "Config.h"
#include <fstream>

using namespace fdm;

nlohmann::json getConfigJson() {
	return{
		{ "TitleScreenWorldRenderDistance", titleScreenWorldRenderDistance },
		{ "ShiftCraftCount", shiftCraftCount },
		{ "CtrlShiftCraftCount", ctrlShiftCraftCount },
		{ "CtrlCraftCount", ctrlCraftCount },
		{ "PopupsEnabled", popupsEnabled },
		{ "PopupLifeTime", popupLifeTime },
		{ "PopupFadeTime", popupFadeTime },
		{ "PopupMoveSpeed", popupMoveSpeed },
	};
}

void writeConfig(const std::string& path, const nlohmann::json& j)
{
	std::ofstream configFileO(path);
	if (configFileO.is_open())
	{
		configFileO << j.dump(4);
		configFileO.close();
	}
}

void saveConfig() {
	nlohmann::json configJson = getConfigJson();
	writeConfig(configPath, configJson);
}

// Read config
$hook(void, StateIntro, init, StateManager& s)
{
	configPath = std::format("{}/config.json", fdm::getModPath(fdm::modID));
	nlohmann::json configJson;
	
	if (!std::filesystem::exists(configPath))
		saveConfig();

	std::ifstream configFileI(configPath);

	configJson = nlohmann::json::parse(configFileI);
	configFileI.close();

	if (!configJson.contains("TitleScreenWorldRenderDistance"))
	{
		configJson["TitleScreenWorldRenderDistance"] = titleScreenWorldRenderDistance;
		writeConfig(configPath, configJson);
	}
	if (!configJson.contains("ShiftCraftCount"))
	{
		configJson["ShiftCraftCount"] = shiftCraftCount;
		writeConfig(configPath, configJson);
	}
	if (!configJson.contains("CtrlShiftCraftCount"))
	{
		configJson["CtrlShiftCraftCount"] = ctrlShiftCraftCount;
		writeConfig(configPath, configJson);
	}
	if (!configJson.contains("CtrlCraftCount"))
	{
		configJson["CtrlCraftCount"] = ctrlCraftCount;
		writeConfig(configPath, configJson);
	}

	if (!configJson.contains("PopupsEnabled"))
	{
		configJson["PopupsEnabled"] = popupsEnabled;
		writeConfig(configPath, configJson);
	}
	if (!configJson.contains("PopupLifeTime"))
	{
		configJson["PopupLifeTime"] = popupLifeTime;
		writeConfig(configPath, configJson);
	}
	if (!configJson.contains("PopupFadeTime"))
	{
		configJson["PopupFadeTime"] = popupFadeTime;
		writeConfig(configPath, configJson);
	}
	if (!configJson.contains("PopupMoveSpeed"))
	{
		configJson["PopupMoveSpeed"] = popupMoveSpeed;
		writeConfig(configPath, configJson);
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