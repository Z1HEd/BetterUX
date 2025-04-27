#include <4dm.h>
#include "auilib/auilib.h"

gui::Button saveWorldButton;

static void saveWorldCallback(void* user) {
	for (auto& it : ((WorldSingleplayer*)user)->chunks) {
		it.second->save(((WorldSingleplayer*)user)->chunkLoader.chunksPath);
	}
	((WorldSingleplayer*)user)->cleanupLocal(&StateGame::instanceObj.player);
}

$hook(void, StatePause, init, StateManager& s) {
	original(self, s);
	if (StateGame::instanceObj.world->getType() != World::TYPE_SINGLEPLAYER) return;

	saveWorldButton.alignY(gui::ALIGN_BOTTOM);
	saveWorldButton.alignX(gui::ALIGN_CENTER_X);
	saveWorldButton.offsetX(self->settingsButton.xOffset);
	saveWorldButton.offsetY(self->settingsButton.yOffset - 100);
	saveWorldButton.width = self->settingsButton.width;
	saveWorldButton.setText("Save");
	saveWorldButton.user = StateGame::instanceObj.world.get();
	saveWorldButton.callback = saveWorldCallback;
	saveWorldButton.mouseDown = false;
	saveWorldButton.selected = false;

	self->ui.addElement(&saveWorldButton);
}
