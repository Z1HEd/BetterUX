#pragma once

inline unsigned int titleScreenWorldRenderDistance = 5;

inline unsigned int ctrlShiftCraftCount = 50;
inline unsigned int ctrlCraftCount = 10;
inline unsigned int shiftCraftCount = 4096;

inline bool popupsEnabled = true;

inline double popupLifeTime = 3;
inline double popupFadeTime = 1;
inline double popupMoveSpeed = .015;

inline std::string configPath;

nlohmann::json getConfigJson();

void writeConfig(const std::string& path, const nlohmann::json& j);

void saveConfig();