#pragma once

#include "auilib/auilib.h"

namespace BetterUX {
	
	inline aui::VBoxContainer* getCategoryContainer()
	{
		return reinterpret_cast<aui::VBoxContainer * (__stdcall*)(void)>(GetProcAddress(fdm::getModHandle("zihed.betterui"), "getCategoryContainer"))();
	}
}