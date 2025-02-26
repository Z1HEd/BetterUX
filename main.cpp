#define DEBUG_CONSOLE // Uncomment this if you want a debug console to start. You can use the Console class to print. You can use Console::inStrings to get input.

#include <4dm.h>
#include <glm/gtc/random.hpp>
#include <auilib/auilib.h>
#include "4DKeyBinds.h"

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

	ui = gui::Interface{ s.window };
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
	ui.addElement(&healthText);
}

//Render UI
$hook(void, Player, renderHud, GLFWwindow* window) {
	original(self, window);

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

	glEnable(GL_DEPTH_TEST);
}

//render distance is the only lowercase setting
$hookStatic(void, StateSettings, renderDistanceSliderCallback, void* user, int value)
{
	original(user, value);
	StateSettings::instanceObj->renderDistanceSlider.setText(std::format("Render Distance: {}", value+1));
}

int getItemCategory(Item* item) {
	if (item==nullptr) return 2;
	if (0 == strcmp(typeid(*item).name(), "class ItemTool")) return -1;
	else if (0 == strcmp(typeid(*item).name(), "class ItemMaterial")) return 0;
	else if (0 == strcmp(typeid(*item).name(), "class ItemBlock")) return 1;
	
	return 0; Console::printLine("UNKNOWN CATEGORY"); // should not be the case
}

void swapIndex(InventoryManager * manager,Inventory * inventory, int a, int b) {
	manager->applyTransfer(InventoryManager::ACTION_SWAP, *inventory->getSlot(a), manager->cursor.item, inventory);
	manager->applyTransfer(InventoryManager::ACTION_SWAP, *inventory->getSlot(b), manager->cursor.item, inventory);
	manager->applyTransfer(InventoryManager::ACTION_SWAP, *inventory->getSlot(a), manager->cursor.item, inventory);
}

int swapPredicateSimple(Item* a, Item* b) {
	if (a != nullptr && b != nullptr) {
		return std::strcmp(a->getName().c_str(), b->getName().c_str());
	}
	else if (b != nullptr) return 1;
	else return -1;
}

int swapPredicateCategorical(Item* a, Item* b) {
	int aCat = getItemCategory(a);
	int bCat = getItemCategory(b);
	if (aCat != bCat) {
		if (bCat > aCat) return -1;
		else if (bCat < aCat) return 1;
		else return 0;
	}
	else {
		return swapPredicateSimple(a, b);
	}
}

void bubbleSort(InventoryManager* manager, Inventory* inventory, int(*predicate)(Item*, Item*)) {
	int n = inventory->getSlotCount();
	for (int i = 0; i < n - 1; i++) {
		for (int j = 0; j < n -i- 1; j++) {
			if (predicate(inventory->getSlot(j)->get(), inventory->getSlot(j + 1)->get())==1)
				swapIndex(manager, inventory, j, j + 1);
		}
	}
}

void ChatGPTSort(InventoryManager* manager, Inventory* inventory) {
	constexpr int ROWS = 8;
	constexpr int COLUMNS = 4;
	constexpr int TOTAL_SLOTS = ROWS * COLUMNS;

	// Step 1: Extract all items with their current positions
	std::vector<std::pair<Item*, int>> items;

	for (int i = 0; i < TOTAL_SLOTS; ++i) {
		Item* item = inventory->getSlot(i)->get();
		if (item) {
			items.emplace_back(item, i);
		}
	}

	// Step 2: Sort items first by category, then by name
	auto getCategory = [](Item* item) -> int {
		int category = getItemCategory(item);
		if (category == -1) return 0; // Tools in column 0
		if (category == 0) return 1;  // Materials in columns 1 & 2
		if (category == 1) return 2;  // Blocks in column 3
		return 3;                     // Unknown items last
		};

	std::sort(items.begin(), items.end(), [&](const auto& a, const auto& b) {
		int catA = getCategory(a.first);
		int catB = getCategory(b.first);
		if (catA != catB) return catA < catB; // Sort by category first
		return a.first->getName() < b.first->getName(); // Then by name
		});

	// Step 3: Assign sorted items to new positions dynamically
	std::vector<int> columnCounts(COLUMNS, 0);
	std::vector<int> targetPositions(TOTAL_SLOTS, -1);

	for (auto& [item, originalIndex] : items) {
		int category = getCategory(item);
		int targetColumn = (category == 0) ? 0 : (category == 1) ? 1 : 3;

		// Handle column overflow by shifting to another column from the end
		while (columnCounts[targetColumn] >= ROWS) {
			targetColumn = (targetColumn == 3) ? 2 : targetColumn - 1;
		}

		int newIndex = columnCounts[targetColumn] + (targetColumn * ROWS);
		columnCounts[targetColumn]++;
		targetPositions[originalIndex] = newIndex;
	}

	// Step 4: Swap items dynamically, tracking real-time positions
	std::vector<int> indexToSlot(TOTAL_SLOTS);
	for (int i = 0; i < TOTAL_SLOTS; i++) {
		indexToSlot[i] = i; // Initial mapping
	}

	for (int i = 0; i < TOTAL_SLOTS; i++) {
		if (targetPositions[i] != -1 && targetPositions[i] != i) {
			int currentIndex = indexToSlot[i]; // Get real current position
			int newIndex = targetPositions[i];

			if (currentIndex != newIndex) {
				swapIndex(manager, inventory, currentIndex, newIndex);

				// Swap tracking indexes
				std::swap(indexToSlot[i], indexToSlot[newIndex]);
			}
		}
	}
}

void sortInventory(GLFWwindow* window, int action, int mods) {
	if (action != 1) return;
	
	InventoryManager* manager = &StateGame::instanceObj->player.inventoryManager;
	if (!manager->isOpen()) return;
	Inventory* inventory = manager->secondary;
	ChatGPTSort(manager, inventory);
}

$exec
{
	KeyBinds::addBind("BetterUI", "Sort inventory", glfw::Keys::R, KeyBindsScope::PLAYER, sortInventory);
}

extern "C" _declspec(dllexport) aui::VBoxContainer* getCategoryContainer() { return &categoryContainer; }