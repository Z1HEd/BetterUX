#include <4dm.h>
#include "auilib/auilib.h"
#include <glm/gtc/random.hpp>
#include "InventoryActions.h"
#include "InventorySorter.h"
#include "4DKeyBinds.h"
#include "Config.h"

QuadRenderer qr{};
FontRenderer font{};
gui::Interface ui;
gui::Text healthText;
gui::TextInput craftSearchInput;



class Popup : public gui::Text{
public:

	double tempPosition; // yOffset is int, it fails with slower movements
	int targetYOffset;
	double updateTime;

	bool isFadingOut = false;
	double fadeOutBeginTime = 0;

	bool isGone = false;

	std::string name;
	int count;
	bool isDeadly = false;

	Popup() : Text::Text(), updateTime(glfwGetTime()) { 
		size = 2; 
		offsetX(-15);
		alignY(ALIGN_BOTTOM); 
		alignX(ALIGN_RIGHT); 
	}
	Popup(std::string Name, int Count, bool IsDeadly) : 
		Text::Text(), name(Name), count(Count),isDeadly(IsDeadly), updateTime(glfwGetTime()) {
		size = 2; 
		offsetX(-15);
		alignY(ALIGN_BOTTOM); 
		alignX(ALIGN_RIGHT); 
	}

	void render(gui::Window* w) override {
		
		if (isGone) return;

		double time = glfwGetTime();

		if (!isFadingOut && time - updateTime > popupLifeTime) {
			isFadingOut = true;
			fadeOutBeginTime = time;
		}

		if (isFadingOut && time - fadeOutBeginTime > popupFadeTime) {
			isGone = true;
			return;
		}
		double towardsTarget = targetYOffset- yOffset ;

		if (std::abs(towardsTarget) > 0.01) {

			double step = std::copysign(
				std::min(popupMoveSpeed, std::abs(towardsTarget)),
				towardsTarget);

			tempPosition += step;
			yOffset = tempPosition;
		}
		text = std::format("{} x{}", name, count);
		shadow = !isDeadly;
		if (isDeadly) {// Deadly item effect

			glm::vec2 offset1 = glm::diskRand(5.0);
			glm::vec2 offset2 = glm::diskRand(5.0);

			int savedX = xOffset, savedY = yOffset;

			color = { 1,0,1,color.a * 0.7 };
			offsetX(savedX + offset1.x);
			offsetY(savedY + offset1.y);
			Text::render(w);

			color = { 0,1,1,color.a };
			offsetX(savedX + offset2.x);
			offsetY(savedY + offset2.y);
			Text::render(w);

			offsetX(savedX);
			offsetY(savedY);
		}
		color.a = isFadingOut ? 1 - (time - fadeOutBeginTime) / popupFadeTime : 1;
		color = { 1,1,1,color.a };
		Text::render(w);
		
	}

	void update() {
		updateTime = glfwGetTime(); isFadingOut = false;
	}
};

std::vector<Popup> popups{};

bool shiftHeldDown = false;
bool ctrlHeldDown = false;

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

// Item popups

void updatePopupPositions() {
	for (int i = 0;i < popups.size();i++) {
		popups[i].targetYOffset = -i * 20 - 10;
	}
}

void itemCollected(Item* item) {
	if (!popupsEnabled) return;
	auto it = std::find_if(popups.begin(), popups.end(), [&](Popup& is) {return is.name == item->getName();});
	if (popups.size() && it != popups.end()) {
		it->count += item->count;
		it->update();
	}
	else {
		popups.emplace(popups.begin(), item->getName(), item->count, item->isDeadly());
		popups.begin()->tempPosition = 20;
	}
	updatePopupPositions();
}

$hook(void, WorldSingleplayer, localPlayerEvent, Player* player, Packet::ClientPacket eventType, int64_t eventValue, void* data) {
	if (eventType == Packet::C_ITEM_COLLECT) {
		itemCollected(((EntityItem*)data)->item.get());
	}
	original(self, player, eventType, eventValue, data);
}

$hook(bool, WorldClient, handleWorldMessage, const Connection::InMessage& message, Player* player) {
	if ((Packet::ServerPacket)message.getPacketType() == Packet::S_ITEM_COLLECT) {
		nlohmann::json item = nlohmann::json::parse(message.getStrData());
		itemCollected(Item::createFromJson(item).get());
	}
	return original(self, message, player);
}


// Render UI

$hook(void, Player, renderHud, GLFWwindow* window) {
	original(self, window);

	// Hp indicator
	static float timeSinceDamage;

	timeSinceDamage = glfwGetTime() - self->damageTime;


	static glm::vec4 textColor;
	if (self->health > 70) textColor = { .5,1,.5,1 }; // light green if high health
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
	healthText.offsetX(42 - width / 2);
	healthText.render(&ui);

	if (self->inventoryManager.isOpen())
		ui.render();

	// Popups

	int i = 0;
	int size = popups.size();
	while (popupsEnabled){
		if (i >= popups.size()) break;
		if (popups[i].isGone) popups.erase(popups.begin() + i);
		else {
			popups[i].render(&ui);
			i++;
		}
	}
	if (popups.size() != size) updatePopupPositions();

	glEnable(GL_DEPTH_TEST);
}

// Recipe filtering

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

bool hotbarSwapInput(StateManager& s, int key) {
	if (key<GLFW_KEY_1 || key>GLFW_KEY_8) return false;

	double cursorX, cursorY;
	glfwGetCursorPos(s.window, &cursorX, &cursorY);

	Inventory* inventory = &StateGame::instanceObj.player.inventory;
	int cursorSlotIndex = inventory->getSlotIndex({ cursorX,cursorY });

	if (cursorSlotIndex < 0) {
		inventory = &StateGame::instanceObj.player.hotbar;
		cursorSlotIndex = inventory->getSlotIndex({ cursorX,cursorY });
	}

	if (cursorSlotIndex < 0) {
		inventory = &StateGame::instanceObj.player.equipment;
		cursorSlotIndex = inventory->getSlotIndex({ cursorX,cursorY });
	}

	if (cursorSlotIndex < 0 && StateGame::instanceObj.player.inventoryManager.secondary) {
		inventory = StateGame::instanceObj.player.inventoryManager.secondary;
		cursorSlotIndex = inventory->getSlotIndex({ cursorX,cursorY });
	}

	if (cursorSlotIndex < 0) return false;


	int hotbarSlotIndex = key - GLFW_KEY_1;

	if (inventory == &StateGame::instanceObj.player.hotbar && hotbarSlotIndex == cursorSlotIndex) return true;

	InventoryManager* manager = &StateGame::instanceObj.player.inventoryManager;

	InventoryActions::swapIndex(manager,
		inventory,
		&StateGame::instanceObj.player.hotbar,
		cursorSlotIndex,
		hotbarSlotIndex, &StateGame::instanceObj.player.hotbar);

	return true;
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

	for (unsigned int i = 0; i < fastcraftCount - 1; i++) {
		original(recipeIndex, user);
	}

	return original(recipeIndex, user);
}

// Keybinds

void sortInventory(GLFWwindow* window, int action, int mods) {
	if (action != GLFW_PRESS) return;

	InventoryManager* manager = &StateGame::instanceObj.player.inventoryManager;
	if (!manager->isOpen()) return;
	if (manager->secondary->name == "inventoryAndEquipment")
		InventorySorter::sort(manager, (InventoryGrid*)((InventoryPlayer*)manager->secondary)->hotbar, (InventoryGrid*)manager->primary);
	else
		InventorySorter::sort(manager, (InventoryGrid*)manager->secondary,(Inventory*)manager->primary);
}

void swapHands(GLFWwindow* window, int action, int mods) {
	if (action != GLFW_PRESS || craftSearchInput.active) return;

	InventoryManager* manager = &StateGame::instanceObj.player.inventoryManager;

	if (!manager->isOpen()) {
		InventoryActions::swapIndex(manager,
			&StateGame::instanceObj.player.equipment,
			&StateGame::instanceObj.player.hotbar,
			0,
			StateGame::instanceObj.player.hotbar.selectedIndex, &StateGame::instanceObj.player.hotbar);
		return;
	}

	double x, y; 
	glfwGetCursorPos(window, &x, &y);

	int index;
	Inventory* inventory = manager->primary;
	index = manager->primary->getSlotIndex({ x,y });

	if (index == -1) {
		index = manager->secondary->getSlotIndex({ x,y });
		inventory = manager->secondary;
	}
	if (index == -1)
		return;
	if (&inventory->getSlot(index) == &StateGame::instanceObj.player.equipment.getSlot(0))
		return;
	InventoryActions::swapIndex(manager,
		inventory,
		&StateGame::instanceObj.player.equipment,
		index,
		0, &StateGame::instanceObj.player.hotbar);
}

void hotbarCycleLeft(GLFWwindow* window, int action, int mods) {
	if (action != GLFW_PRESS) return;

	InventoryManager* manager = &StateGame::instanceObj.player.inventoryManager;
	
	Player& player = StateGame::instanceObj.player;

	for (int i = 0;i < 8;i++) {
		for (int j = 3;j > -1;j--) {
			InventoryActions::cursorTransfer(manager, &player.inventory, j * 8 + i, &player.hotbar);
		}
		InventoryActions::cursorTransfer(manager, &player.hotbar, i, &player.inventory);
		InventoryActions::cursorTransfer(manager, &player.inventory, i+24, &player.hotbar);
	}
	
}

void hotbarCycleRight(GLFWwindow* window, int action, int mods) {
	if (action != GLFW_PRESS) return;

	InventoryManager* manager = &StateGame::instanceObj.player.inventoryManager;

	Player& player = StateGame::instanceObj.player;

	for (int i = 0;i < 8;i++) {
		for (int j = 0;j < 4;j++) {
			InventoryActions::cursorTransfer(manager, &player.inventory, j * 8 + i, &player.hotbar);
		}
		InventoryActions::cursorTransfer(manager, &player.hotbar, i, &player.inventory);
		InventoryActions::cursorTransfer(manager, &player.inventory, i, &player.hotbar);
	}

}

using LocalPlayerEventHandler = void (__thiscall *)(World*,Player*, Packet::ClientPacket, int64_t, void*);
void dropItemInventory(LocalPlayerEventHandler original) {

	Player& player = StateGame::instanceObj.player;
	InventoryManager* manager = &player.inventoryManager;
	InventoryGrid* hotbar = &player.hotbar;

	double x, y;
	glfwGetCursorPos(StateGame::instanceObj.ui.getGLFWwindow(), &x, &y);

	Inventory* inventory = manager->primary;
	int index = inventory->getSlotIndex({ x,y });

	if (index == -1) {
		inventory = manager->secondary;
		index = inventory->getSlotIndex({ x,y });
	}
	
	if (index == -1) return;

	if (!inventory->getSlot(index)) return;

	if (shiftHeldDown) { // drop all
		
		InventoryActions::cursorTransfer(manager, inventory, index, inventory);
		
		StateGame::instanceObj.world->localPlayerEvent(&player, Packet::C_ITEM_THROW_CURSOR, 0, nullptr);

		InventoryActions::cursorTransfer(manager, inventory, index, inventory);
	}
	else { // drop one
		if (inventory!=hotbar || index !=hotbar->selectedIndex)
			InventoryActions::swapIndex(manager, hotbar, inventory, hotbar->selectedIndex, index, inventory);

		original(StateGame::instanceObj.world.get(), &player, Packet::C_ITEM_THROW_HOTBAR, 0, nullptr);

		if (inventory != hotbar || index != hotbar->selectedIndex)
			InventoryActions::swapIndex(manager, hotbar, inventory, hotbar->selectedIndex, index, inventory);
	}
	
}

$hook(void, WorldSingleplayer, localPlayerEvent, Player* player, Packet::ClientPacket eventType, int64_t eventValue, void* data) {
	if (eventType == Packet::C_ITEM_THROW_HOTBAR && player->inventoryManager.isOpen()) {
		return dropItemInventory(reinterpret_cast<LocalPlayerEventHandler>(original));
	}
	return original(self, player, eventType, eventValue, data);
}

$hook(void, WorldClient, localPlayerEvent, Player* player, Packet::ClientPacket eventType, int64_t eventValue, void* data) {
	if (eventType == Packet::C_ITEM_THROW_HOTBAR && player->inventoryManager.isOpen()) {
		return dropItemInventory(reinterpret_cast<LocalPlayerEventHandler>(original));
	}
	return original(self, player, eventType, eventValue, data);
}

void findAndSwap(Player* player,std::string name,Inventory* destinationInventory,int destinationIndex) {
	if (destinationInventory->getSlot(destinationIndex) && 
		destinationInventory->getSlot(destinationIndex)->getName() == name)
		return;

	int slotIndex = 0;
	Inventory* inventory = &player->equipment;

	
	if (!inventory->getSlot(slotIndex) || inventory->getSlot(slotIndex)->getName() != name)
		for (int i = 0;i < player->hotbar.getSlotCount();i++) {
			if (player->hotbar.getSlot(i) && player->hotbar.getSlot(i)->getName() == name) {
				slotIndex = i;
				inventory = &player->hotbar;
				break;
			}
		}

	if (!inventory->getSlot(slotIndex) || inventory->getSlot(slotIndex)->getName() != name)
		for (int i = 0;i < player->inventory.getSlotCount();i++) {
			if (player->inventory.getSlot(i) && player->inventory.getSlot(i)->getName() == name) {
				slotIndex = i;
				inventory = &player->inventory;
				break;
			}
		}

	if (inventory->getSlot(slotIndex) && inventory->getSlot(slotIndex)->getName() == name)
		InventoryActions::swapIndex(&player->inventoryManager,
			destinationInventory,
			inventory,
			destinationIndex,
			slotIndex, &player->hotbar);
}

void pickBlock(StateGame* s) {
	if (!s->player.targetingBlock)
		return;
	uint8_t targetBlock = s->world->getBlock(s->player.targetBlock);
	std::string targetBlockName = BlockInfo::blockNames[targetBlock];

	findAndSwap(&s->player, targetBlockName, &s->player.hotbar, s->player.hotbar.selectedIndex);
}

$hook(bool, Player, keyInput, GLFWwindow* window, World* world, int key, int scancode, int action, int mods)
{
	if (!KeyBinds::isLoaded())
	{
		if (key == GLFW_KEY_R)
			sortInventory(window, action, mods);
		if (key == GLFW_KEY_F)
			swapHands(window, action, mods);
		if (key == GLFW_KEY_Z)
			hotbarCycleLeft(window, action, mods);
		if (key == GLFW_KEY_X)
			hotbarCycleRight(window, action, mods);
	}
	return original(self, window, world, key, scancode, action, mods);
}
$exec
{
	KeyBinds::addBind("BetterUX", "Sort Inventory", glfw::Keys::R, KeyBindsScope::PLAYER, sortInventory);
	KeyBinds::addBind("BetterUX", "Swap Hands", glfw::Keys::F, KeyBindsScope::PLAYER, swapHands);
	KeyBinds::addBind("BetterUX", "Hotbar cycle left", glfw::Keys::Z, KeyBindsScope::PLAYER, hotbarCycleLeft);
	KeyBinds::addBind("BetterUX", "Hotbar cycle right", glfw::Keys::X, KeyBindsScope::PLAYER, hotbarCycleRight);
}

// Passing inputs into UI

$hook(void, StateGame, charInput, StateManager& s, uint32_t codepoint)
{
	if (!self->player.inventoryManager.isOpen() || !craftSearchInput.active || !ui.charInput(codepoint))
		return original(self, s, codepoint);
	self->player.inventoryManager.craftingMenu.updateAvailableRecipes();
}
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

	if (self->player.inventoryManager.isOpen() && action == GLFW_PRESS && hotbarSwapInput(s, key))
		return;
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
	// Block picking
	if (!self->player.inventoryManager.isOpen() &&
		button == GLFW_MOUSE_BUTTON_MIDDLE &&
		action == GLFW_PRESS)
		return pickBlock(self);

	// UI input
	if (self->player.inventoryManager.isOpen() && !ui.mouseButtonInput(button, action, mods))
		return original(self, s, button, action, mods);

	return original(self, s, button, action, mods);
}
$hook(void, StateGame, scrollInput, StateManager& s, double xoffset, double yoffset)
{
	original(self, s, xoffset, yoffset);
	if (!self->player.inventoryManager.isOpen()) return;
	ui.scrollInput(xoffset, yoffset);
}
