#pragma once
#include <4dm.h>
using namespace fdm;

namespace utils {
	inline static void setTextHeaderStyle(gui::Text* text, int size)
	{
		switch (size) {
		case 1:
			text->size = 4;
			text->fancy = true;
			text->shadow = true;
			break;
		case 2:
			text->size = 3;
			text->fancy = false;
			text->shadow = true;
			break;
		case 3:
		default:
			text->size = 2;
			text->fancy = false;
			text->shadow = false;
			break;
		}
		text->alignX(gui::ALIGN_CENTER_X);
	}
}