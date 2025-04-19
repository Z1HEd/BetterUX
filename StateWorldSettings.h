#pragma once

#include <4dm.h>
#include "auilib/auilib.h"

using namespace fdm;

class StateWorldSettings : public State
{
public:

	static StateWorldSettings instanceObj;

	QuadRenderer qr;
	const Shader* qs;

	FontRenderer font;

	std::string worldFolder = "";
	std::string worldName = "";

	// current ui
	gui::Interface* ui;

	// ui - main
	gui::Interface mainUI;

	gui::Text mainTitle;

	gui::Text folderTitle;
	gui::TextInput folderInput;

	gui::Text nameTitle;
	gui::TextInput nameInput;

	gui::Button deleteButton;
	gui::Button copyButton;
	gui::Button cancelButton;
	gui::Button saveButton;

	// ui - copy
	gui::Interface copyUI;

	gui::Text copyTitle;

	gui::Text copyFolderTitle;
	gui::TextInput copyFolderInput;

	gui::Text copyNameTitle;
	gui::TextInput copyNameInput;

	gui::Button copyConfirmButton;
	gui::Button copyCancelButton;

	// ui - copy in progress

	gui::Interface copyingUI;
	gui::Text copyingTitle;

	// ui - delete
	gui::Interface deleteUI;

	gui::Text deleteTitle;

	gui::Text deleteNameTitle;
	gui::TextInput deleteNameInput;

	gui::Button deleteConfirmButton;
	gui::Button deleteCancelButton;

	StateWorldSettings() {}

	inline static void deleteButtonCallback(void* user);
	inline static void copyButtonCallback(void* user);
	inline static void returnButtonCallback(void* user);
	inline static void saveButtonCallback(void* user);
	inline static void copyConfirmButtonCallback(void* user);
	inline static void copyCancelButtonCallback(void* user);
	inline static void deleteConfirmButtonCallback(void* user);
	inline static void deleteCancelButtonCallback(void* user);

	void updateSelectedWorld();

	void updateProjection(const glm::ivec2& size);
	static void viewportCallback(void* user, const glm::ivec4& pos, const glm::ivec2& scroll);

	void init(StateManager& s) override;
	void close(StateManager& s) override;
	void render(StateManager& s) override;
	void update(StateManager& s,double dt) override;
	void mouseInput(StateManager& s, double xpos, double ypos) override;
	void scrollInput(StateManager& s, double xoff, double yoff) override;
	void mouseButtonInput(StateManager& s, int button, int action, int mods) override;
	void keyInput(StateManager&, int key, int scancode, int action, int mods) override;
	void windowResize(StateManager&, int width, int height) override;
	void charInput(StateManager&, uint32_t codepoint) override;
};