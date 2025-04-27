#include <4dm.h>
#include "auilib/auilib.h"
#include "utils.h"

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

	StateSettings::instanceObj.renderDistanceSlider.range = 31;
	StateSettings::instanceObj.renderDistanceSlider.setText(std::format("Render Distance: {}", StateSettings::instanceObj.currentRenderDistance));

	utils::setTextHeaderStyle(&self->settingsTitleText, 1); // main title must be big
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

	utils::setTextHeaderStyle(&controlsText, 2);
	controlsText.setText("Controls");
	controlsContainer.addElement(&controlsText);


	utils::setTextHeaderStyle(&graphicsText, 2);
	graphicsText.setText("Graphics");
	graphicsContainer.addElement(&graphicsText);


	utils::setTextHeaderStyle(&gameplayText, 2);
	gameplayText.setText("Gameplay");
	gameplayContainer.addElement(&gameplayText);

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

	categoryContainer.addElement(&otherContainer);

	initializedSettings = true;

}

$hook(void, StateSettings, init, StateManager& s)
{
	initializedSettings = false;
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
