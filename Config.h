#pragma once

inline unsigned int titleScreenWorldRenderDistance = 5;

inline unsigned int ctrlShiftCraftCount = 50;
inline unsigned int ctrlCraftCount = 10;
inline unsigned int shiftCraftCount = 4096;

inline std::string configPath;

void updateConfig(const std::string& path, const nlohmann::json& j);