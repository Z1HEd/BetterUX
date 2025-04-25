//#define DEBUG_CONSOLE // Uncomment this if you want a debug console to start. You can use the Console class to print. You can use Console::inStrings to get input.

#include <4dm.h>
#include <glm/gtc/random.hpp>
#include "auilib/auilib.h"
#include "4DKeyBinds.h"
#include "InventorySorter.h"
#include "StateWorldSettings.h"
#include <fstream>

#include "utils.h"

using namespace fdm;

// Initialize the DLLMain
initDLL

unsigned int ctrlShiftCraftCount = 50;
unsigned int ctrlCraftCount = 10;
unsigned int shiftCraftCount = -1;

static bool initializedSettings = false;

aui::VBoxContainer categoryContainer;

aui::VBoxContainer modLoaderOptionsContainer;
aui::VBoxContainer controlsContainer;
aui::VBoxContainer graphicsContainer;
aui::VBoxContainer gameplayContainer;
aui::VBoxContainer multiplayerContainer;
aui::VBoxContainer audioContainer;
aui::VBoxContainer betterUXContainer;
aui::VBoxContainer otherContainer;

gui::Text controlsText;
gui::Text graphicsText;
gui::Text gameplayText;

gui::Text betterUXText;
gui::Slider zoomingValueSlider{};
gui::CheckBox enableZoomingCheckBox{};

QuadRenderer qr{};
FontRenderer font{};
gui::Interface ui;
gui::Text healthText;
gui::TextInput craftSearchInput;

StateManager* stateManager = nullptr;

std::string configPath;

// enables conventional zooming on button
static bool isZoomingEnabled = true;

// if true locks conventional zooming and assumes other mod is setting currentZoom
// will zoom even if isZoomingEnabled is false
static bool isZoomingLocked = false;

static float currentZoom = 1;
static float zoomedValue = 0.3f;

void updateConfig(const std::string& path, const nlohmann::json& j)
{
	std::ofstream configFileO(path);
	if (configFileO.is_open())
	{
		configFileO << j.dump(4);
		configFileO.close();
	}
}

static std::vector<aui::ImgButton> worldSettingsButtons = {};

inline static gui::Text* getTextElement(gui::Element* element) {
	if (0 == strcmp(typeid(*element).name(), "class gui::Text"))
		return ((gui::Text*)element);
	else if (0 == strcmp(typeid(*element).name(), "class fdm::gui::Text"))
		return ((gui::Text*)element);
	return nullptr;
}

inline static int getY(gui::Element* element)
{
	// im comparing typeid name strings instead of using dynamic_cast because typeids of 4dminer and typeids of 4dm.h are different
	if (0 == strcmp(typeid(*element).name(), "class gui::Button"))
		return ((gui::Button*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class gui::CheckBox"))
		return ((gui::CheckBox*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class gui::Image"))
		return ((gui::Image*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class gui::Slider"))
		return ((gui::Slider*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class gui::Text"))
		return ((gui::Text*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class gui::TextInput"))
		return ((gui::TextInput*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class fdm::gui::Button"))
		return ((gui::Button*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class fdm::gui::CheckBox"))
		return ((gui::CheckBox*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class fdm::gui::Image"))
		return ((gui::Image*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class fdm::gui::Slider"))
		return ((gui::Slider*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class fdm::gui::Text"))
		return ((gui::Text*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class fdm::gui::TextInput"))
		return ((gui::TextInput*)element)->yOffset;
	return 0;
}

inline static std::string getText(gui::Element* element)
{
	// im comparing typeid name strings instead of using dynamic_cast because typeids of 4dminer and typeids of 4dm.h are different
	if (0 == strcmp(typeid(*element).name(), "class gui::Button"))
		return ((gui::Button*)element)->text;
	else if (0 == strcmp(typeid(*element).name(), "class gui::CheckBox"))
		return ((gui::CheckBox*)element)->text;
	else if (0 == strcmp(typeid(*element).name(), "class gui::Image"))
		return "Image";
	else if (0 == strcmp(typeid(*element).name(), "class gui::Slider"))
		return ((gui::Slider*)element)->text;
	else if (0 == strcmp(typeid(*element).name(), "class gui::Text"))
		return ((gui::Text*)element)->text;
	else if (0 == strcmp(typeid(*element).name(), "class gui::TextInput"))
		return ((gui::TextInput*)element)->text;
	else if (0 == strcmp(typeid(*element).name(), "class fdm::gui::Button"))
		return ((gui::Button*)element)->text;
	else if (0 == strcmp(typeid(*element).name(), "class fdm::gui::CheckBox"))
		return ((gui::CheckBox*)element)->text;
	else if (0 == strcmp(typeid(*element).name(), "class fdm::gui::Image"))
		return "Image";
	else if (0 == strcmp(typeid(*element).name(), "class fdm::gui::Slider"))
		return ((gui::Slider*)element)->text;
	else if (0 == strcmp(typeid(*element).name(), "class fdm::gui::Text"))
		return ((gui::Text*)element)->text;
	else if (0 == strcmp(typeid(*element).name(), "class fdm::gui::TextInput"))
		return ((gui::TextInput*)element)->text;
	return "Unknown element";
}

bool compareY(gui::Element* a, gui::Element* b) {
	return getY(a) < getY(b);
}

static void setTextHeaderStyle(gui::Text* text, int size)
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

static void putIntoCategory(gui::Element* e) {
	std::string elementText = getText(e);
	// LOADER OPTIONS
	if ((elementText.find("4DModLoader Options") != std::string::npos) ||
		(elementText.find("Auto-Update") != std::string::npos))
	{
		modLoaderOptionsContainer.addElement(e);
	}
	// CONTROLS
	else if ((elementText.find("Controls") != std::string::npos) || (elementText.find("Keybinds") != std::string::npos) ||
		(elementText.find("Scroll Sensitivity:") != std::string::npos) ||
		(elementText.find("Look Sensitivity:") != std::string::npos) ||
		(elementText.find("Invert Look X") != std::string::npos) ||
		(elementText.find("Invert Look Y") != std::string::npos))
	{
		controlsContainer.addElement(e);
	}
	// GRAPHICS
	else if ((elementText.find("Enter Fullscreen") != std::string::npos) || (elementText.find("Exit Fullscreen") != std::string::npos) ||
		(elementText.find("Render Distance:") != std::string::npos) ||
		(elementText.find("FOV:") != std::string::npos) ||
		(elementText.find("Smooth Lighting") != std::string::npos) ||
		(elementText.find("Shadows") != std::string::npos) ||
		(elementText.find("Colored Lights") != std::string::npos))
	{
		graphicsContainer.addElement(e);
	}
	// GAMEPLAY
	else if (elementText.find("Difficulty:") != std::string::npos)
	{
		gameplayContainer.addElement(e);
	}
	// MULTIPLAYER
	else if ((elementText.find("Multiplayer:") != std::string::npos) ||
		(elementText.find("Display Chat") != std::string::npos) ||
		(elementText.find("Display Player Nametags") != std::string::npos) ||
		(elementText.find("Display Custom Player Skins") != std::string::npos))
	{
		multiplayerContainer.addElement(e);
	}
	// AUDIO
	else if ((elementText.find("Audio:") != std::string::npos) ||
		(elementText.find("Global Volume:") != std::string::npos) ||
		(elementText.find("Music Volume:") != std::string::npos) ||
		(elementText.find("Creature Volume:") != std::string::npos) ||
		(elementText.find("Ambience Volume:") != std::string::npos))
	{
		audioContainer.addElement(e);
	}
	else otherContainer.addElement(e);
}

$hook(void, StateIntro, init, StateManager& s)
{
	original(self, s);

	stateManager = &s;

	// initialize opengl stuff
	glewExperimental = true;
	glewInit();
	glfwInit();

	configPath = std::format("{}/config.json", fdm::getModPath(fdm::modID));

	nlohmann::json configJson
	{
		{ "ZoomMultiplier", zoomedValue  },
		{ "EnableZooming", isZoomingEnabled },
		{ "ShiftCraftCount", shiftCraftCount},
		{ "CtrlShiftCraftCount", ctrlShiftCraftCount },
		{ "CtrlCraftCount", ctrlCraftCount}
	};

	if (!std::filesystem::exists(configPath))
	{
		updateConfig(configPath, configJson);
	}
	else
	{
		std::ifstream configFileI(configPath);
		if (configFileI.is_open())
		{
			configJson = nlohmann::json::parse(configFileI);
			configFileI.close();
		}
	}

	if (!configJson.contains("ZoomMultiplier"))
	{
		configJson["ZoomMultiplier"] = zoomedValue;
		updateConfig(configPath, configJson);
	}
	if (!configJson.contains("EnableZooming"))
	{
		configJson["EnableZooming"] = isZoomingEnabled;
		updateConfig(configPath, configJson);
	}

	if (!configJson.contains("ShiftCraftCount"))
	{
		configJson["ShiftCraftCount"] = shiftCraftCount;
		updateConfig(configPath, configJson);
	}
	if (!configJson.contains("CtrlShiftCraftCount"))
	{
		configJson["CtrlShiftCraftCount"] = ctrlShiftCraftCount;
		updateConfig(configPath, configJson);
	}
	if (!configJson.contains("CtrlCraftCount"))
	{
		configJson["CtrlCraftCount"] = ctrlCraftCount;
		updateConfig(configPath, configJson);
	}

	zoomedValue = configJson["ZoomMultiplier"];
	isZoomingEnabled = configJson["EnableZooming"];
	shiftCraftCount = configJson["ShiftCraftCount"];
	ctrlShiftCraftCount = configJson["CtrlShiftCraftCount"];
	ctrlCraftCount = configJson["CtrlCraftCount"];
}

//Add custom settings 
$hook(void, StateSettings, render, StateManager& s)
{
	original(self, s);
	if (initializedSettings)
		return;

	StateSettings::instanceObj.renderDistanceSlider.range = 31;
	StateSettings::instanceObj.renderDistanceSlider.setText(std::format("Render Distance: {}", StateSettings::instanceObj.currentRenderDistance));

	setTextHeaderStyle(&self->settingsTitleText, 1); // main title must be big
	self->settingsTitleText.offsetY(32); // and centered

	std::sort(self->mainContentBox.elements.begin(), self->mainContentBox.elements.end(), compareY);

	categoryContainer.clear();
	categoryContainer.maxColumns = 2;

	modLoaderOptionsContainer.clear();
	controlsContainer.clear();
	graphicsContainer.clear();
	gameplayContainer.clear();
	multiplayerContainer.clear();
	audioContainer.clear();
	betterUXContainer.clear();
	otherContainer.clear();

	categoryContainer.ySpacing = 43;
	categoryContainer.xSpacing = 50;

	modLoaderOptionsContainer.ySpacing = 20;
	controlsContainer.ySpacing = 20;
	graphicsContainer.ySpacing = 20;
	gameplayContainer.ySpacing = 20;
	multiplayerContainer.ySpacing = 20;
	audioContainer.ySpacing = 20;
	betterUXContainer.ySpacing = 20;
	otherContainer.ySpacing = 20;

	setTextHeaderStyle(&controlsText, 2);
	controlsText.setText("Controls");
	controlsContainer.addElement(&controlsText);


	setTextHeaderStyle(&graphicsText, 2);
	graphicsText.setText("Graphics");
	graphicsContainer.addElement(&graphicsText);


	setTextHeaderStyle(&gameplayText, 2);
	gameplayText.setText("Gameplay");
	gameplayContainer.addElement(&gameplayText);

	setTextHeaderStyle(&betterUXText, 2);
	betterUXText.setText("BetterUX");

	enableZoomingCheckBox.alignX(gui::ALIGN_CENTER_X);
	enableZoomingCheckBox.checked = isZoomingEnabled;
	enableZoomingCheckBox.setText("Enable Zooming");
	enableZoomingCheckBox.callback = [](void* user, bool value)
		{
			isZoomingEnabled = value;
			updateConfig(configPath, { { "ZoomMultiplier", zoomedValue },{ "EnableZooming", isZoomingEnabled } });
		};

	zoomingValueSlider.alignX(gui::ALIGN_CENTER_X);
	zoomingValueSlider.range = 17; // 0,1...19
	zoomingValueSlider.value = (1.0f / zoomedValue-1.5f)*2;
	zoomingValueSlider.setText(std::format("Zoom Multiplier: {:.2f}", ((float)zoomingValueSlider.value / 2 + 1.5f)));
	zoomingValueSlider.width = 500;
	zoomingValueSlider.user = &zoomingValueSlider;
	zoomingValueSlider.callback = [](void* user, int value)
		{
			zoomedValue = 1.0f / ((float)value / 2 + 1.5f);
			zoomingValueSlider.setText(std::format("Zoom Multiplier: {:.2f}", ((float)value/2+1.5f)));
			updateConfig(configPath, { { "ZoomMultiplier", zoomedValue },{ "EnableZooming", isZoomingEnabled } });
		};

	betterUXContainer.addElement(&betterUXText);
	betterUXContainer.addElement(&enableZoomingCheckBox);
	betterUXContainer.addElement(&zoomingValueSlider);

	for (auto& e : self->mainContentBox.elements)
	{
		// Hmmm where should i put my favourite button?
		if (e == &self->secretButton)
			// Perish.
			continue;

		if (auto* t = getTextElement(e)) setTextHeaderStyle(t, 2); // all titles must be bigger than rest of the text, but smaller than main title


		putIntoCategory(e);
	}

	self->mainContentBox.clear();
	self->mainContentBox.addElement(&categoryContainer);
	self->mainContentBox.addElement(&self->secretButton); // Fiiiine you may live

	categoryContainer.addElement(&modLoaderOptionsContainer);
	categoryContainer.addElement(&graphicsContainer);
	categoryContainer.addElement(&audioContainer);
	categoryContainer.addElement(&gameplayContainer);
	categoryContainer.addElement(&controlsContainer);
	categoryContainer.addElement(&multiplayerContainer);
	categoryContainer.addElement(&betterUXContainer);

	categoryContainer.addElement(&otherContainer);

	initializedSettings = true;

}

$hook(void, StateSettings, init, StateManager& s)
{
	initializedSettings = false;
	original(self, s);
}

// World settings buttons
void worldSettingsButtonCallback(void* user)
{
	StateWorldSettings::instanceObj.worldFolder= *(std::string*)user;

	StateWorldSettings::instanceObj.updateSelectedWorld();
	stateManager->pushState(&StateWorldSettings::instanceObj);
}

$hook(void, StateSingleplayer, init,StateManager& s) {
	original(self, s);

	setTextHeaderStyle(&self->yourWorlds, 1); // main title must be big
	self->yourWorlds.offsetY(32); // and centered

}

$hook(void, StateSingleplayer, updateWorldListContainer, int wWidth, int wHeight) {
	original(self, wWidth, wHeight);
	worldSettingsButtons.clear();
	for (int i = 0;i < self->worldButtons.size();i++) {
		self->worldButtons[i].width -= 80;
		self->worldButtons[i].xOffset = -30;

		aui::ImgButton& newButton = worldSettingsButtons.emplace_back(ResourceManager::get("assets/SettingsIcon.png", true), self->worldButtons[i].height, self->worldButtons[i].height, 0, 0);
		newButton.offsetX(self->worldButtons[i].xOffset+ self->worldButtons[i].width + newButton.width + 15);
		newButton.offsetY(self->worldButtons[i].yOffset);
		newButton.user = &self->worldPaths[i];
		newButton.callback = worldSettingsButtonCallback;
	}
	for (int i = 0;i < worldSettingsButtons.size();i++) {
		self->worldListContainer.addElement(&worldSettingsButtons[i]);
		worldSettingsButtons[i].deselect();
	}
}

$hook(void, StateSingleplayer, windowResize, StateManager& s, int width, int height) {
	original(self, s, width, height);
	for (int i = 0;i < self->worldButtons.size();i++) {
		self->worldButtons[i].xOffset = -30;
	}
}

$hook(void,StateSingleplayer, updateProjection, const glm::ivec2& size, const glm::ivec2& translate2D) {
	original(self, size, translate2D);

	// create a 2D projection matrix from the specified dimensions and scroll position
	glm::mat4 projection2D = glm::ortho(0.0f, (float)size.x, (float)size.y, 0.0f, -1.0f, 1.0f);
	projection2D = glm::translate(projection2D, { translate2D.x, translate2D.y, 0 });

	const Shader* tex2DShader = ShaderManager::get("tex2DShader");
	tex2DShader->use();
	glUniformMatrix4fv(glGetUniformLocation(tex2DShader->id(), "P"), 1, 0, &projection2D[0][0]);
}

//Initialize stuff when entering world

void viewportCallback(void* user, const glm::ivec4& pos, const glm::ivec2& scroll)
{
	GLFWwindow* window = (GLFWwindow*)user;

	// update the render viewport
	int wWidth, wHeight;
	glfwGetWindowSize(window, &wWidth, &wHeight);
	glViewport(pos.x, wHeight - pos.y - pos.w, pos.z, pos.w);

	// create a 2D projection matrix from the specified dimensions and scroll position
	glm::mat4 projection2D = glm::ortho(0.0f, (float)pos.z, (float)pos.w, 0.0f, -1.0f, 1.0f);
	projection2D = glm::translate(projection2D, { scroll.x, scroll.y, 0 });

	// update all 2D shaders
	const Shader* textShader = ShaderManager::get("textShader");
	textShader->use();
	glUniformMatrix4fv(glGetUniformLocation(textShader->id(), "P"), 1, GL_FALSE, &projection2D[0][0]);

	const Shader* tex2DShader = ShaderManager::get("tex2DShader");
	tex2DShader->use();
	glUniformMatrix4fv(glGetUniformLocation(tex2DShader->id(), "P"), 1, GL_FALSE, &projection2D[0][0]);

	const Shader* quadShader = ShaderManager::get("quadShader");
	quadShader->use();
	glUniformMatrix4fv(glGetUniformLocation(quadShader->id(), "P"), 1, GL_FALSE, &projection2D[0][0]);
}

$hook(void, StateGame, init, StateManager& s)
{
	original(self, s);

	font = { ResourceManager::get("pixelFont.png"), ShaderManager::get("textShader") };

	qr.shader = ShaderManager::get("quadShader");
	qr.init();

	ui.window = s.window;
	ui.viewportCallback = viewportCallback;
	ui.viewportUser = s.window;
	ui.font = &font;
	ui.qr = &qr;

	healthText.text = "100";
	healthText.alignX(gui::ALIGN_LEFT);
	healthText.alignY(gui::ALIGN_BOTTOM);
	healthText.size = 2;
	healthText.shadow = true;
	healthText.offsetY(-26);
	
	craftSearchInput.height = 30;
	craftSearchInput.width = 300;
	craftSearchInput.alignX(gui::ALIGN_RIGHT);
	craftSearchInput.alignY(gui::ALIGN_TOP);
	int x, y;
	self->player.inventoryManager.craftingMenuBox.getPos(&ui, &x, &y);
	craftSearchInput.offsetY(y - craftSearchInput.height);
	craftSearchInput.editable = true;

	ui.addElement(&craftSearchInput);

	gui::Text* craftingText = &self->player.inventoryManager.craftingText;

	craftingText->offsetY(20);
}

// Make auilib work lol

$hook(void, StateGame, windowResize, StateManager& s, GLsizei width, GLsizei height) {
	original(self, s, width, height);
	viewportCallback(s.window, { 0,0, width, height }, { 0,0 });
}

//Render UI

$hook(void, Player, renderHud, GLFWwindow* window) {
	original(self, window);

	// Hp indicator
	static float timeSinceDamage;
	
	timeSinceDamage = glfwGetTime() - self->damageTime;

	
	static glm::vec4 textColor;
	if (self->health > 70) textColor = { .5,1,.5,1 }; //light green if high health
	else if (self->health > 30) textColor = { 1,0.64f,0,1 }; // orange if half health
	else textColor = { 1,0.1f,0.1f,1 }; // red if low health

	healthText.text = std::to_string((int)self->health);

	int width, height;
	healthText.getSize(&ui, &width, &height);

	glDisable(GL_DEPTH_TEST);
	if (timeSinceDamage < Player::DAMAGE_COOLDOWN) {

		glm::vec2 aOffset = glm::diskRand(5.f);
		glm::vec2 bOffset = glm::diskRand(5.f);

		healthText.color = { 1,0,1,1 };
		healthText.offsetX(42 - width / 2 + aOffset.x);
		healthText.offsetY(-26 + aOffset.y);
		healthText.render(&ui);

		healthText.color = { 1,0,1,1 };
		healthText.offsetX(42 - width / 2 + aOffset.x);
		healthText.offsetY(-26 + aOffset.y);
		healthText.render(&ui);

		healthText.offsetY(-26);
	}
	healthText.color = textColor;
	healthText.offsetX(42- width/2);
	healthText.render(&ui);

	if (self->inventoryManager.isOpen())
		ui.render();

	glEnable(GL_DEPTH_TEST);

	CraftingMenu* menu = &self->inventoryManager.craftingMenu;
	gui::ContentBox* craftingMenuBox = &self->inventoryManager.craftingMenuBox;
	int x, y;
	for (int i = 0; i < craftingMenuBox->elements.size();i++) {
		craftingMenuBox->elements[i]->getPos(craftingMenuBox->parent, &x, &y);
	}
}

//Recipe filtering

$hook(void, CraftingMenu, updateAvailableRecipes)
{
	original(self);
	for (auto it = self->craftableRecipes.begin(); it < self->craftableRecipes.end(); )
	{
		stl::string lowerItem = it->result->getName();
		std::transform(lowerItem.begin(), lowerItem.end(), lowerItem.begin(),
			[](uint8_t c) { return std::tolower(c); });

		stl::string lowerInput = craftSearchInput.text;
		std::transform(lowerInput.begin(), lowerInput.end(), lowerInput.begin(),
			[](uint8_t c) { return std::tolower(c); });

		if (lowerItem.find(lowerInput) == std::string::npos)
		{
			it = self->craftableRecipes.erase(it);
			continue;
		}
		it++;
	}
	self->Interface->updateCraftingMenuBox();
}

$hook(void, StateGame, charInput, StateManager& s, uint32_t codepoint)
{
	if (!self->player.inventoryManager.isOpen() || !craftSearchInput.active || !ui.charInput(codepoint))
		return original(self, s, codepoint);
	self->player.inventoryManager.craftingMenu.updateAvailableRecipes();
}
bool shiftHeldDown = false;
bool ctrlHeldDown = false;
$hook(void, StateGame, keyInput, StateManager& s, int key, int scancode, int action, int mods)
{
	// used for multicrafting
	if (action != GLFW_REPEAT)
	{
		switch (key)
		{
		case GLFW_KEY_LEFT_SHIFT:
		{
			shiftHeldDown = action == GLFW_PRESS;
		} break;
		case GLFW_KEY_LEFT_CONTROL:
		{
			ctrlHeldDown = action == GLFW_PRESS;
		} break;
		}
	}

	if (!self->player.inventoryManager.isOpen() || !craftSearchInput.active || key == GLFW_KEY_ESCAPE)
		return original(self, s, key, scancode, action, mods);
	if (ui.keyInput(key, scancode, action, mods))
		self->player.inventoryManager.craftingMenu.updateAvailableRecipes();
}
$hook(void, StateGame, mouseInput, StateManager& s, double xpos, double ypos)
{
	original(self, s, xpos, ypos);
	if (!self->player.inventoryManager.isOpen()) return;
	ui.mouseInput(xpos, ypos);
}
$hook(void, StateGame, mouseButtonInput, StateManager& s, int button, int action, int mods)
{
	if (!self->player.inventoryManager.isOpen() || !ui.mouseButtonInput(button, action, mods))
		original(self, s, button, action, mods);
	//updateFilteredCrafts(&self->player);
}
$hook(void, StateGame, scrollInput, StateManager& s, double xoffset, double yoffset)
{
	original(self, s, xoffset, yoffset);
	if (!self->player.inventoryManager.isOpen()) return;
	ui.scrollInput(xoffset, yoffset);
}

// "render distance" is the only lowercase setting ._.

$hookStatic(void, StateSettings, renderDistanceSliderCallback, void* user, int value)
{
	original(user, value);
	StateSettings::instanceObj.renderDistanceSlider.setText(std::format("Render Distance: {}", value+1));
}

// Fastcrafting
$hook(bool, CraftingMenu, craftRecipe, int recipeIndex) {
	if (StateGame::instanceObj.world->getType() != World::TYPE_SINGLEPLAYER) return original(self, recipeIndex);

	if (!original(self, recipeIndex)) return false;
	unsigned int fastcraftCount = 1;

	if (shiftHeldDown && ctrlHeldDown)
		fastcraftCount = ctrlShiftCraftCount;
	else if (shiftHeldDown)
		fastcraftCount = shiftCraftCount;
	else if (ctrlHeldDown)
		fastcraftCount = ctrlCraftCount;

	for (unsigned int i = 0; i < fastcraftCount - 1; i++) {
		if (!original(self, recipeIndex)) break;
	}

	return true;
}
$hookStatic(bool, InventoryManager, craftingMenuCallback, int recipeIndex, void* user) {

	unsigned int fastcraftCount = 1;

	if (shiftHeldDown && ctrlHeldDown)
		fastcraftCount = ctrlShiftCraftCount;
	else if (shiftHeldDown)
		fastcraftCount = shiftCraftCount;
	else if (ctrlHeldDown)
		fastcraftCount = ctrlCraftCount;

	auto dummyItem = Item::create(CraftingMenu::recipes[recipeIndex]["result"]["name"].get<std::string>(), 1);
	
	fastcraftCount = std::min(fastcraftCount, dummyItem->getStackLimit() /
		CraftingMenu::recipes[recipeIndex]["result"]["count"]);

	for (unsigned int i = 0; i < fastcraftCount-1; i++) {
		original(recipeIndex, user);
	}

	return original(recipeIndex, user);
}

// Inventory sorting
void sortInventory(GLFWwindow* window, int action, int mods) {
	if (action != GLFW_PRESS) return;

	InventoryManager* manager = &StateGame::instanceObj.player.inventoryManager;
	if (!manager->isOpen()) return;
	if (manager->secondary->name=="inventoryAndEquipment")
		InventorySorter::sort(manager, (InventoryGrid*)((InventoryPlayer*)manager->secondary)->hotbar);
	else
		InventorySorter::sort(manager, (InventoryGrid*)manager->secondary);
}

// Zooming

void setZooming(GLFWwindow* window, int action, int mods) {
	if (isZoomingLocked || !isZoomingEnabled) return;
	if (action == GLFW_PRESS) currentZoom = zoomedValue;
	else if (action == GLFW_RELEASE) currentZoom = 1;
}

// Decrease sensitivity when zooming
$hook(void, Player, mouseInput, GLFWwindow* window, World* world, double xpos, double ypos) {
	if (isZoomingLocked || !isZoomingEnabled) return original(self, window, world, xpos, ypos);
	original(self, window, world,self->mouseX+ (xpos- self->mouseX)* currentZoom, self->mouseY + (ypos - self->mouseY) * currentZoom);
}
// FOV changing
$hook(void, StateGame, render, StateManager& s) {
	static double lastTime = glfwGetTime() - 0.01;
	double dt = glfwGetTime() - lastTime;
	lastTime = glfwGetTime();
	if (self->player.inventoryManager.isOpen() 
		|| (!isZoomingEnabled && !isZoomingLocked)) return original(self, s);

	if (std::abs(self->FOV - (StateSettings::instanceObj.currentFOV - 30)*currentZoom) > 0.01) {
		self->FOV = utils::ilerp(self->FOV, (StateSettings::instanceObj.currentFOV + 30) * currentZoom, 0.38f, dt);
		int width, height;
		glfwGetWindowSize(s.window, &width, &height);
		self->updateProjection(glm::max(width, 1), glm::max(height, 1));
	}
	original(self, s);
}


// Keybinds

$hook(bool, Player, keyInput, GLFWwindow* window, World* world, int key, int scancode, int action, int mods)
{
	if (!KeyBinds::isLoaded())
	{
		if (key == GLFW_KEY_R)
			sortInventory(window, action, mods);
		if (key == GLFW_KEY_V && action == GLFW_PRESS)
			setZooming(window, action, mods);
	}
	return original(self, window, world, key, scancode, action, mods);
}
$exec
{
	KeyBinds::addBind("BetterUX", "Sort Inventory", glfw::Keys::R, KeyBindsScope::PLAYER, sortInventory);
	KeyBinds::addBind("BetterUX", "Zoom", glfw::Keys::V, KeyBindsScope::PLAYER, setZooming);
}

// Extern functions

extern "C" _declspec(dllexport) aui::VBoxContainer* getCategoryContainer() { return &categoryContainer; }

extern "C" _declspec(dllexport) bool lockZooming() { 
	if (isZoomingLocked) 
		return false; 
	isZoomingLocked = true; 
	return true;
}

extern "C" _declspec(dllexport) bool unlockZooming() {
	if (!isZoomingLocked)
		return true;
	isZoomingLocked = false;
	return false;
}
