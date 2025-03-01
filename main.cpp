#define DEBUG_CONSOLE // Uncomment this if you want a debug console to start. You can use the Console class to print. You can use Console::inStrings to get input.

#include <4dm.h>
#include <glm/gtc/random.hpp>
#include <auilib/auilib.h>
#include "4DKeyBinds.h"
#include "InventorySorter.h"

using namespace fdm;

// Initialize the DLLMain
initDLL

static bool initializedSettings = false;

aui::VBoxContainer categoryContainer;

aui::VBoxContainer modLoaderOptionsContainer;
aui::VBoxContainer controlsContainer;
aui::VBoxContainer graphicsContainer;
aui::VBoxContainer gameplayContainer;
aui::VBoxContainer multiplayerContainer;
aui::VBoxContainer audioContainer;
aui::VBoxContainer otherContainer;

gui::Text controlsText;
gui::Text graphicsText;
gui::Text gameplayText;

QuadRenderer qr{};
FontRenderer font{};
gui::Interface ui;
gui::Text healthText;
gui::TextInput craftSearchInput;

$hook(void, StateIntro, init, StateManager& s)
{
	original(self, s);

	// initialize opengl stuff
	glewExperimental = true;
	glewInit();
	glfwInit();
}

$hook(void, StateSettings, init, StateManager& s)
{
	initializedSettings = false;
	original(self, s);
}

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

bool compareY(gui::Element*  a, gui::Element*  b) {
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

//Add custom settings 
$hook(void, StateSettings, render, StateManager& s)
{
	original(self, s);
	if (initializedSettings)
		return;

	StateSettings::instanceObj->renderDistanceSlider.setText(std::format("Render Distance: {}", StateSettings::instanceObj->currentRenderDistance));

	setTextHeaderStyle(&self->settingsTitleText, 1); // main title must be big
	self->settingsTitleText.offsetY(32); // and centered

	std::sort(self->mainContentBox.elements.begin(), self->mainContentBox.elements.end(),compareY);
	
	categoryContainer.clear();
	categoryContainer.maxColumns = 2;

	modLoaderOptionsContainer.clear();
	controlsContainer.clear();
	graphicsContainer.clear();
	gameplayContainer.clear();
	multiplayerContainer.clear();
	audioContainer.clear();
	otherContainer.clear();

	categoryContainer.ySpacing = 43;
	categoryContainer.xSpacing = 50;

	modLoaderOptionsContainer.ySpacing=20;
	controlsContainer.ySpacing = 20;
	graphicsContainer.ySpacing = 20;
	gameplayContainer.ySpacing = 20;
	multiplayerContainer.ySpacing = 20;
	audioContainer.ySpacing = 20;
	otherContainer.ySpacing = 20;

	setTextHeaderStyle(&controlsText, 2);
	controlsText.setText("Controls:");
	controlsContainer.addElement(&controlsText);

	
	setTextHeaderStyle(&graphicsText, 2);
	graphicsText.setText("Graphics:");
	graphicsContainer.addElement(&graphicsText);

	
	setTextHeaderStyle(&gameplayText, 2);
	gameplayText.setText("Gameplay:");
	gameplayContainer.addElement(&gameplayText);

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
	
	categoryContainer.addElement(&otherContainer);

	initializedSettings = true;

}

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

//Initialize stuff when entering world
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
	//craftSearchInput.active = true;
	craftSearchInput.editable = true;

	//ui.addElement(&healthText);
	ui.addElement(&craftSearchInput);
	//self->player.inventoryManager.craftingMenuBox.addElement(&craftSearchInput);

	gui::Text* craftingText = &self->player.inventoryManager.craftingText;

	craftingText->offsetY(20);
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
	if (!self->player.inventoryManager.isOpen() || !ui.charInput(codepoint))
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
	StateSettings::instanceObj->renderDistanceSlider.setText(std::format("Render Distance: {}", value+1));
}

void sortInventory(GLFWwindow* window, int action, int mods) {
	if (action != GLFW_PRESS) return;
	
	InventoryManager* manager = &StateGame::instanceObj->player.inventoryManager;
	if (!manager->isOpen()) return;
	InventorySorter::sort(manager,(InventoryGrid*)manager->secondary);
}

$hook(bool, Player, keyInput, GLFWwindow* window, World* world, int key, int scancode, int action, int mods)
{
	if (!KeyBinds::isLoaded())
	{
		if (key == GLFW_KEY_R)
			sortInventory(window, action, mods);
	}

	return original(self, window, world, key, scancode, action, mods);
}

inline static bool(__thiscall* craftRecipe_o)(CraftingMenu* self, int recipeIndex) = nullptr;
inline static bool __fastcall craftRecipe_h(CraftingMenu* self, int recipeIndex)
{
	if (craftRecipe_o(self, recipeIndex))
	{
		if (shiftHeldDown)
		{
			if (ctrlHeldDown)
			{
				for (int i = 0; i < 50 - 1; i++)
					craftRecipe_o(self, recipeIndex);
				return true;
			}
			while (craftRecipe_o(self, recipeIndex));
			return true;
		}
		if (ctrlHeldDown)
		{
			for (int i = 0; i < 10 - 1; i++)
				craftRecipe_o(self, recipeIndex);
			return true;
		}
	}
	return false;
}

$exec
{
	KeyBinds::addBind("BetterUI", "Sort inventory", glfw::Keys::R, KeyBindsScope::PLAYER, sortInventory);

	Hook(fdm::base + 0x5C120, craftRecipe_h, &craftRecipe_o);
	EnableHook(reinterpret_cast<LPVOID>(fdm::base + 0x5C120));
}

extern "C" _declspec(dllexport) aui::VBoxContainer* getCategoryContainer() { return &categoryContainer; }
