#include <4dm.h>
#include "auilib/auilib.h"
#include "utils.h"
#include "Config.h"

static bool initializedSettings = false;

aui::VBoxContainer categoryContainer;

aui::VBoxContainer modLoaderOptionsContainer;
aui::VBoxContainer controlsContainer;
aui::VBoxContainer graphicsContainer;
aui::VBoxContainer gameplayContainer;
aui::VBoxContainer multiplayerContainer;
aui::VBoxContainer audioContainer;
aui::VBoxContainer betterUXContainer;
std::vector<aui::VBoxContainer> otherContainers;

gui::Text controlsText;
gui::Text graphicsText;
gui::Text gameplayText;

gui::Text betterUXText;
gui::Slider titleScreenWorldRenderDistanceSlider;
gui::CheckBox popupsEnabledCheckbox;
gui::Slider popupLifeTimeSlider;
gui::Slider popupFadeSlider;
gui::Slider popupMoveSpeedSlider;


inline static gui::Text* getTextElement(gui::Element* element) {
	if (0 == strcmp(typeid(*element).name(), "class gui::Text"))
		return ((gui::Text*)element);
	else if (0 == strcmp(typeid(*element).name(), "class fdm::gui::Text"))
		return ((gui::Text*)element);
	return nullptr;
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

static bool putIntoCategory(gui::Element* e) {
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
	else {
		if (getTextElement(e) && getTextElement(e)->size>2) { // Title, create new category
			otherContainers.emplace_back();
			otherContainers[otherContainers.size() - 1].ySpacing = 20;
		}
		otherContainers[otherContainers.size() - 1].addElement(e);
	}
}

//Add custom settings 
$hook(void, StateSettings, render, StateManager& s)
{
	self->secretButton.offsetY(categoryContainer.height + 500);
	self->mainContentBox.scrollH = categoryContainer.height + 600 - self->mainContentBox.height;

	original(self, s);
	if (initializedSettings)
		return;

	StateSettings::instanceObj.renderDistanceSlider.range = 31;
	StateSettings::instanceObj.renderDistanceSlider.setText(std::format("Render Distance: {}", StateSettings::instanceObj.currentRenderDistance));

	utils::setTextHeaderStyle(&self->settingsTitleText, 1); // main title must be big
	self->settingsTitleText.offsetY(32); // and centered

	categoryContainer.clear();
	categoryContainer.maxColumns = 2;

	modLoaderOptionsContainer.clear();
	controlsContainer.clear();
	graphicsContainer.clear();
	gameplayContainer.clear();
	multiplayerContainer.clear();
	audioContainer.clear();
	betterUXContainer.clear();
	otherContainers.clear();

	categoryContainer.ySpacing = 43;
	categoryContainer.xSpacing = 50;

	modLoaderOptionsContainer.ySpacing = 20;
	controlsContainer.ySpacing = 20;
	graphicsContainer.ySpacing = 20;
	gameplayContainer.ySpacing = 20;
	multiplayerContainer.ySpacing = 20;
	audioContainer.ySpacing = 20;
	betterUXContainer.ySpacing = 20;
	

	utils::setTextHeaderStyle(&controlsText, 2);
	controlsText.setText("Controls");
	controlsContainer.addElement(&controlsText);


	utils::setTextHeaderStyle(&graphicsText, 2);
	graphicsText.setText("Graphics");
	graphicsContainer.addElement(&graphicsText);


	utils::setTextHeaderStyle(&gameplayText, 2);
	gameplayText.setText("Gameplay");
	gameplayContainer.addElement(&gameplayText);

	utils::setTextHeaderStyle(&betterUXText, 2);
	betterUXText.setText("BetterUX");
	betterUXContainer.addElement(&betterUXText);

	popupsEnabledCheckbox.checked = popupsEnabled;
	popupsEnabledCheckbox.alignX(ALIGN_CENTER_X);
	popupsEnabledCheckbox.alignY(ALIGN_TOP);
	popupsEnabledCheckbox.setText("Item Popups:");

	popupsEnabledCheckbox.callback = [](void* user, bool value) {
		popupsEnabled = value;
		popupLifeTimeSlider.clickable = popupsEnabled;
		popupFadeSlider.clickable = popupsEnabled;
		popupMoveSpeedSlider.clickable = popupsEnabled;
		saveConfig();
		};


	titleScreenWorldRenderDistanceSlider.alignX(ALIGN_CENTER_X);
	titleScreenWorldRenderDistanceSlider.alignY(ALIGN_TOP);
	popupLifeTimeSlider.alignX(ALIGN_CENTER_X);
	popupLifeTimeSlider.alignY(ALIGN_TOP);
	popupFadeSlider.alignX(ALIGN_CENTER_X);
	popupFadeSlider.alignY(ALIGN_TOP);
	popupMoveSpeedSlider.alignX(ALIGN_CENTER_X);
	popupMoveSpeedSlider.alignY(ALIGN_TOP);

	titleScreenWorldRenderDistanceSlider.width = popupLifeTimeSlider.width = popupFadeSlider.width = popupMoveSpeedSlider.width = 500;

	titleScreenWorldRenderDistanceSlider.range = 16;
	popupLifeTimeSlider.range = 19;
	popupFadeSlider.range = 9;
	popupMoveSpeedSlider.range = 9;

	titleScreenWorldRenderDistanceSlider.value = titleScreenWorldRenderDistance;
	popupLifeTimeSlider.value = popupLifeTime*2-1;
	popupFadeSlider.value = popupFadeTime * 5 - 1;
	popupMoveSpeedSlider.value = popupMoveSpeed * 10 - 1;

	titleScreenWorldRenderDistanceSlider.setText(std::format("BG World Render Distance: {}", titleScreenWorldRenderDistance));
	popupLifeTimeSlider.setText(std::format("Lifetime: {:.1f}", popupLifeTime));
	popupFadeSlider.setText(std::format("Fade Duration: {:.1f}", popupFadeTime));
	popupMoveSpeedSlider.setText(std::format("Speed: {:.1f}", popupMoveSpeed));

	titleScreenWorldRenderDistanceSlider.callback = [](void* user, int value) {
		titleScreenWorldRenderDistance = value;
		titleScreenWorldRenderDistanceSlider.setText(std::format("BG World Render Distance: {}", titleScreenWorldRenderDistance));
		saveConfig();
		};
	popupLifeTimeSlider.callback = [](void* user, int value) {
		popupLifeTime = (double)value / 2 + 0.5;
		popupLifeTimeSlider.setText(std::format("Lifetime: {:.1f}", popupLifeTime));
		saveConfig();
		};
	popupFadeSlider.callback = [](void* user, int value) {
		popupFadeTime = (double)value / 5 + 0.2;
		popupFadeSlider.setText(std::format("Fade Duration: {:.1f}", popupFadeTime));
		saveConfig();
		};
	popupMoveSpeedSlider.callback = [](void* user, int value) {
		popupMoveSpeed = (double)value / 10 + 0.1;
		popupMoveSpeedSlider.setText(std::format("Speed: {:.1f}", popupMoveSpeed));
		saveConfig();
		};

	popupLifeTimeSlider.clickable = popupsEnabled;
	popupFadeSlider.clickable = popupsEnabled;
	popupMoveSpeedSlider.clickable = popupsEnabled;

	betterUXContainer.addElement(&titleScreenWorldRenderDistanceSlider);
	betterUXContainer.addElement(&popupsEnabledCheckbox);
	betterUXContainer.addElement(&popupLifeTimeSlider);
	betterUXContainer.addElement(&popupFadeSlider);
	betterUXContainer.addElement(&popupMoveSpeedSlider);

	for (auto& e : self->mainContentBox.elements)
	{
		// Hmmm where should i put my favourite button?
		if (e == &self->secretButton)
			// Perish.
			continue;

		if (auto* t = getTextElement(e)) utils::setTextHeaderStyle(t, 2); // all titles must be bigger than rest of the text, but smaller than main title


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

	for (auto& container : otherContainers)
		categoryContainer.addElement(&container);
	initializedSettings = true;
}

$hook(void, StateSettings, init, StateManager& s)
{
	initializedSettings = false;
	self->ui.clear();
	original(self, s);
}

// "render distance" is the only lowercase setting ._.
$hookStatic(void, StateSettings, renderDistanceSliderCallback, void* user, int value)
{
	original(user, value);
	StateSettings::instanceObj.renderDistanceSlider.setText(std::format("Render Distance: {}", value + 1));
}

// Extern functions

extern "C" _declspec(dllexport) aui::VBoxContainer* getCategoryContainer() { return &categoryContainer; }
