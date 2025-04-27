#include <4dm.h>
#include "StateWorldSettings.h"
#include "utils.h"

static std::vector<aui::ImgButton> worldSettingsButtons = {};
static StateManager* manager;

void worldSettingsButtonCallback(void* user)
{
	StateWorldSettings::instanceObj.worldFolder = *(std::string*)user;

	StateWorldSettings::instanceObj.updateSelectedWorld();
	manager->pushState(&StateWorldSettings::instanceObj);
}

$hook(void, StateSingleplayer, init, StateManager& s) {
	original(self, s);
	manager = &s;
	utils::setTextHeaderStyle(&self->yourWorlds, 1); // main title must be big
	self->yourWorlds.offsetY(32); // and centered

}

$hook(void, StateSingleplayer, updateWorldListContainer, int wWidth, int wHeight) {
	original(self, wWidth, wHeight);
	worldSettingsButtons.clear();
	for (int i = 0;i < self->worldButtons.size();i++) {
		self->worldButtons[i].width -= 80;
		self->worldButtons[i].xOffset = -30;

		aui::ImgButton& newButton = worldSettingsButtons.emplace_back(ResourceManager::get("assets/SettingsIcon.png", true), self->worldButtons[i].height, self->worldButtons[i].height, 0, 0);
		newButton.offsetX(self->worldButtons[i].xOffset + self->worldButtons[i].width + newButton.width + 15);
		newButton.offsetY(self->worldButtons[i].yOffset);
		newButton.user = &self->worldPaths[i];
		newButton.callback = worldSettingsButtonCallback;
		newButton.mouseDown = false;
		newButton.selected = false;
	}
	for (int i = 0;i < worldSettingsButtons.size();i++) {
		self->worldListContainer.addElement(&worldSettingsButtons[i]);
	}
}

$hook(void, StateSingleplayer, windowResize, StateManager& s, int width, int height) {
	original(self, s, width, height);
	for (int i = 0;i < self->worldButtons.size();i++) {
		self->worldButtons[i].xOffset = -30;
	}
}

$hook(void, StateSingleplayer, updateProjection, const glm::ivec2& size, const glm::ivec2& translate2D) {
	original(self, size, translate2D);

	// create a 2D projection matrix from the specified dimensions and scroll position
	glm::mat4 projection2D = glm::ortho(0.0f, (float)size.x, (float)size.y, 0.0f, -1.0f, 1.0f);
	projection2D = glm::translate(projection2D, { translate2D.x, translate2D.y, 0 });

	const Shader* tex2DShader = ShaderManager::get("tex2DShader");
	tex2DShader->use();
	glUniformMatrix4fv(glGetUniformLocation(tex2DShader->id(), "P"), 1, 0, &projection2D[0][0]);
}