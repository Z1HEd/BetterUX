#include <4dm.h>

using namespace fdm;

gui::Text createWorldTitle;

$hook(void,StateCreateWorld, init, StateManager& s) {
	original(self, s);

	createWorldTitle.setText("Create World");
	createWorldTitle.alignX(gui::ALIGN_CENTER_X);
	createWorldTitle.alignY(gui::ALIGN_TOP);
	createWorldTitle.offsetY(40);
	createWorldTitle.size = 4;
	createWorldTitle.shadow = true;
	createWorldTitle.fancy = true;

	for (auto& element : self->ui.elements) {
		if (0 == strcmp(typeid(*element).name(), "class gui::Button")) continue;
		int x, y;
		element->getPos(&self->ui, &x, &y);
		element->offsetY(y + 50);
	}

	self->ui.addElement(&createWorldTitle);
}