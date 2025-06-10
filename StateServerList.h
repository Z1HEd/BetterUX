#pragma once

#include <4dm.h>
#include "auilib/auilib.h"

using namespace fdm;

class StateServerList : public State
{
public:
	struct ServerInfo {
		stl::string address;
		stl::string displayedName;
		stl::string playerNameOverride;
		stl::string uuidOverride;
		stl::string skinPathOverride;

		nlohmann::json save();
		static ServerInfo createFromJSON(nlohmann::json j);
	};

	static StateServerList instanceObj;

	QuadRenderer qr;
	const Shader* qs;

	FontRenderer font;
	StateManager* manager;

	std::vector<ServerInfo> servers = {};
	int editedServerIndex;

	// UI

	gui::Interface* currentUI;

	// Main UI
	gui::Interface mainUI;
	gui::Text mainTitle;

	gui::Button addServerButton; 
	gui::Button returnButton; 
	gui::Button playerSettingsButton;
	gui::ContentBox serverListContainer;
	gui::Text noServersText;
	std::vector<gui::Button> serverButtons;
	std::vector<aui::ImgButton> editServerButtons;
	std::vector<stl::string> serverAddresses;
	
	// Add server UI

	gui::Interface addServerUI;
	gui::Text addServerTitle;

	gui::Text addServerNameTitle;
	gui::TextInput addServerNameInput;
	gui::Text addServerAddressTitle;
	gui::TextInput addServerAddressInput;

	gui::Text addServerNameOverrideTitle;
	gui::TextInput addServerNameOverrideInput;
	gui::Text addServerUuidOverrideTitle;
	gui::TextInput addServerUuidOverrideInput;
	gui::Button addServerSelectUuidOverrideButton;
	gui::Button addServerClearUuidOverrideButton;
	gui::Text addServerSkinPathOverrideTitle;
	gui::TextInput addServerSkinPathOverrideInput;
	gui::Button addServerSelectSkinPathOverrideButton;
	gui::Button addServerClearSkinPathOverrideButton;

	gui::Button addServerConfirmButton;
	gui::Button addServerCancelButton;

	// Edit server UI

	gui::Interface editServerUI;
	gui::Text editServerTitle;

	gui::Text editServerNameTitle;
	gui::TextInput editServerNameInput;
	gui::Text editServerAddressTitle;
	gui::TextInput editServerAddressInput;
	gui::Text editServerNameOverrideTitle;
	gui::TextInput editServerNameOverrideInput;
	gui::Text editServerUuidOverrideTitle;
	gui::TextInput editServerUuidOverrideInput;
	gui::Button editServerSelectUuidOverrideButton;
	gui::Button editServerClearUuidOverrideButton;
	gui::Text editServerSkinPathOverrideTitle;
	gui::TextInput editServerSkinPathOverrideInput;
	gui::Button editServerSelectSkinPathOverrideButton;
	gui::Button editServerClearSkinPathOverrideButton;


	gui::Button editServerConfirmButton;
	gui::Button editServerCancelButton;
	gui::Button editServerRemoveButton;

	// Player settings UI

	gui::Interface playerSettingsUI;
	gui::Text playerSettingsTitle;

	gui::Button playerSettingsReturnButton;

	StateServerList() {}

	void updateProjection(const glm::ivec2& size);
	static void viewportCallback(void* user, const glm::ivec4& pos, const glm::ivec2& scroll);
	void updateServerListContainer(int wWidth, int wHeight);
	

	void loadServers();
	void saveServers();

	// Button callbacks
	static void addServerCallback(void* user);
	static void serverButtonCallback(void* user);
	static void addServerConfirmCallback(void* user);
	static void editServerConfirmCallback(void* user);
	static void editServerRemoveCallback(void* user);
	static void editServerCallback(void* user);
	static void savePlayerSettingsCallback(void* user);

	void init(StateManager& s) override;
	void close(StateManager& s) override;
	void render(StateManager& s) override;
	void update(StateManager& s, double dt) override;
	void mouseInput(StateManager& s, double xpos, double ypos) override;
	void scrollInput(StateManager& s, double xoff, double yoff) override;
	void mouseButtonInput(StateManager& s, int button, int action, int mods) override;
	void keyInput(StateManager&, int key, int scancode, int action, int mods) override;
	void windowResize(StateManager&, int width, int height) override;
	void charInput(StateManager&, uint32_t codepoint) override;
};