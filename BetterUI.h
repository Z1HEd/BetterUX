#pragma once

#include "auilib/auilib.h"

namespace BetterUI {
	

	inline aui::VBoxContainer* getCategoryContainer()
	{
		return reinterpret_cast<aui::VBoxContainer * (__stdcall*)(void)>(GetProcAddress(fdm::getModHandle("zihed.betterui"), "getCategoryContainer"))();
	}
	inline bool lockZooming()
	{
		return reinterpret_cast<bool (__stdcall*)(void)>(GetProcAddress(fdm::getModHandle("zihed.betterui"), "lockZooming"))();
	}
	inline void setZoom(float zoomValue)
	{
		return reinterpret_cast<void(__stdcall*)(void)>(GetProcAddress(fdm::getModHandle("zihed.betterui"), "setZoom"))();
	}
	inline bool unlockZooming()
	{
		return reinterpret_cast<bool(__stdcall*)(void)>(GetProcAddress(fdm::getModHandle("zihed.betterui"), "unlockZooming"))();
	}
}