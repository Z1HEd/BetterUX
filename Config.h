#pragma once

extern uint32_t titleScreenWorldRenderDistance;

extern uint32_t ctrlShiftCraftCount;
extern uint32_t ctrlCraftCount;
extern uint32_t shiftCraftCount;

extern bool popupsEnabled;

extern double popupLifeTime;
extern double popupFadeTime;
extern double popupMoveSpeed;

extern std::string configPath;

nlohmann::json getConfigJson();

void writeConfig(const std::string& path, const nlohmann::json& j);

void saveConfig();