#pragma once

inline unsigned int titleScreenWorldRenderDistance = 5;

inline unsigned int ctrlShiftCraftCount = 50;
inline unsigned int ctrlCraftCount = 10;
inline unsigned int shiftCraftCount = 4096;

inline bool popupsEnabled = true;

inline double popupLifeTime = 3; // between 0.5 and 10
inline double popupFadeTime = 1; // between 0.5 and 3
inline double popupMoveSpeed = .2; // between 0.1 and 1

inline std::string configPath;

void updateConfig(const std::string& path, const nlohmann::json& j);