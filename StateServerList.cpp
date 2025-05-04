#define GLFW_EXPOSE_NATIVE_WIN32 
#include "StateServerList.h"
#include "fstream"
#include <GLFW/glfw3native.h>

std::string savedName = "";
std::string savedSkinPath = "";
StateServerList StateServerList::instanceObj = StateServerList();

void StateServerList::updateProjection(const glm::ivec2& size)
{
	glm::mat4 projection2D = glm::ortho(0.f, (float)size.x, (float)size.y, 0.f, -1.0f, 1.0f);
	glViewport(0, 0, size.x, size.y);

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
void StateServerList::viewportCallback(void* user, const glm::ivec4& pos, const glm::ivec2& scroll)
{
	GLFWwindow* window = (GLFWwindow*)user;

	// update the viewport
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

void StateServerList::updateServerListContainer(int wWidth, int wHeight) {
	//serverListContainer.selectedElemIndex = -1;
	if (serverListContainer.elements.size()>0)serverListContainer.elements.clear();
	serverListContainer.width = wWidth / 2;
	serverListContainer.height = wHeight -250;

	for (int i = 0;i < serverButtons.size();i++) {
		serverButtons[i].width = serverListContainer.width - 100;
		serverButtons[i].height = 50;
		editServerButtons[i].offsetX(serverButtons[i].xOffset + serverButtons[i].width + editServerButtons[i].width + 15);
		serverListContainer.addElement(&serverButtons[i]);
		serverListContainer.addElement(&editServerButtons[i]);
	}

	if (serverListContainer.empty() && std::find(mainUI.elements.begin(), mainUI.elements.end(), &noServersText) == mainUI.elements.end())
		mainUI.addElement(&noServersText);
	if (!serverListContainer.empty() && std::find(mainUI.elements.begin(), mainUI.elements.end(), &noServersText) != mainUI.elements.end())
		mainUI.removeElement(&noServersText);
}

nlohmann::json StateServerList::ServerInfo::save() {
	return {
		{"displayedName", displayedName},
		{"address", address},
		{"playerNameOverride",playerNameOverride},
		{"uuidOverride",uuidOverride},
		{"skinPathOverride",skinPathOverride}
	};;
}

StateServerList::ServerInfo StateServerList::ServerInfo::createFromJSON(nlohmann::json j) {
	return{ j.at("address").get<std::string>(),
			j.at("displayedName").get<std::string>(),
			j.at("playerNameOverride").get<std::string>(),
			j.at("uuidOverride").get<std::string>(), 
			j.at("skinPathOverride").get<std::string>()};
}

void StateServerList::loadServers() {
	servers.clear();
	serverButtons.clear();
	editServerButtons.clear();
	std::ifstream serversFile("servers.json");
	if (!serversFile.is_open()) return;

	nlohmann::json serversJson = nlohmann::json::parse(serversFile);
	for (auto server : serversJson) {
		servers.push_back(ServerInfo::createFromJSON(server));
	}

	for (int i = 0;i < servers.size();i++) {
		gui::Button& newButton =serverButtons.emplace_back();
		newButton.setText(servers[i].displayedName);
		newButton.user = &servers[i];
		newButton.alignX(ALIGN_CENTER_X);
		newButton.alignY(ALIGN_TOP);
		newButton.offsetY(10+i*60);
		newButton.offsetX(-30);
		newButton.callback = serverButtonCallback;

		aui::ImgButton& settingsButton = editServerButtons.emplace_back(ResourceManager::get("assets/SettingsIcon.png", true),50, 50, 0, 0);
		settingsButton.offsetX(newButton.xOffset + newButton.width + settingsButton.width + 15);
		settingsButton.offsetY(newButton.yOffset);
		settingsButton.user = (void*)i;
		settingsButton.callback = editServerCallback;
		settingsButton.mouseDown = false;
		settingsButton.selected = false;

	}
}

void StateServerList::saveServers() {
	std::ofstream serversFile("servers.json");
	if (!serversFile.is_open()) return;

	nlohmann::json output = {};

	for (auto server : servers)
		output.push_back(nlohmann::json{server.save()});
	serversFile << output.dump(4);
	serversFile.close();
}

// Dont save overriden stuff
$hook(void, StateSettings, save,__int16 a2) {
	
	if (savedSkinPath!="") {
		StateSkinChooser::instanceObj.skinPath= savedSkinPath;
		StateSkinChooser::instanceObj.skin.load(StateSkinChooser::instanceObj.skinPath);
		StateSkinChooser::instanceObj.skinRenderer.skin = &StateSkinChooser::instanceObj.skin;
		savedSkinPath = "";
	}
	original(self,a2);
}

$hook(void, StateMultiplayer, close,StateManager& s) {
	if (savedName != "") {
		self->displayNameInput.setText(savedName);
		savedName = "";
	}
	original(self, s);
}

// Buttons callbacks

void loadDefaultPlayerSettings() {

	// SKIN

	std::ifstream file("settings.json");
	if (!file.is_open()) return;
	
	nlohmann::json jsonFile = nlohmann::json::parse(file);
	file.close();

	stl::string skin = jsonFile["skin"].get<std::string>();

	StateSkinChooser::instanceObj.skinPath = skin;
	StateSkinChooser::instanceObj.skin.load(StateSkinChooser::instanceObj.skinPath);
	StateSkinChooser::instanceObj.skinRenderer.skin = &StateSkinChooser::instanceObj.skin;
	savedSkinPath = "";

	// PLAYER NAME

	std::ifstream file2("mpSettings.json");
	if (!file2.is_open()) return;

	nlohmann::json jsonFile2 = nlohmann::json::parse(file2);
	file2.close();

	StateMultiplayer::instanceObj.displayNameInput.setText(jsonFile2["displayName"].get<std::string>());
	savedName = "";
	
	// UUID

	std::ifstream file3("uuid.txt");
	if (!file3.is_open()) return;

	std::string uuid;
	file3 >> uuid;

	StateMultiplayer::instanceObj.uuidInput.setText(uuid);

	file3.close();
}

void StateServerList::addServerCallback(void* user) {
	StateServerList::instanceObj.currentUI = &StateServerList::instanceObj.addServerUI;
}

void StateServerList::serverButtonCallback(void* user) {
	ServerInfo& info = (*(ServerInfo*)user);

	loadDefaultPlayerSettings();

	StateServerList::instanceObj.manager->changeState(&StateMultiplayer::instanceObj);
	StateMultiplayer::instanceObj.serverAddressInput.setText(info.address);
	if (info.uuidOverride != "")
	{
		std::ifstream uuidFile(info.uuidOverride);
		if (!uuidFile.is_open()) return;
		std::string uuid;
		uuidFile >> uuid;
		StateMultiplayer::instanceObj.uuidInput.setText(uuid);
	}
	if (info.playerNameOverride != "") {
		savedName = StateMultiplayer::instanceObj.displayNameInput.text;
		StateMultiplayer::instanceObj.displayNameInput.setText(info.playerNameOverride);
	}
	if (info.skinPathOverride != "") {
		savedSkinPath = StateSkinChooser::instanceObj.skinPath;
		StateSkinChooser::instanceObj.skinPath = info.skinPathOverride;
		StateSkinChooser::instanceObj.skin.load(StateSkinChooser::instanceObj.skinPath);
		StateSkinChooser::instanceObj.skinRenderer.skin = &StateSkinChooser::instanceObj.skin;
	}

	StateMultiplayer::instanceObj.connectionErrorOkButton.callback =
		[](void* user) {StateServerList::instanceObj.manager->changeState(&StateServerList::instanceObj);};
	StateMultiplayer::instanceObj.joinButtonCallback(StateMultiplayer::instanceObj.joinButton.user);

}

void StateServerList::addServerConfirmCallback(void* user) {
	StateServerList::instanceObj.servers.emplace_back(
		StateServerList::instanceObj.addServerAdressInput.text,
		StateServerList::instanceObj.addServerNameInput.text,
		StateServerList::instanceObj.addServerNameOverrideInput.text,
		StateServerList::instanceObj.addServerUuidOverrideInput.text,
		StateServerList::instanceObj.addServerSkinPathOverrideInput.text
		);
	StateServerList::instanceObj.saveServers();
	StateServerList::instanceObj.loadServers();
	int wWidth, wHeight;
	glfwGetWindowSize(StateServerList::instanceObj.currentUI->getGLFWwindow(), &wWidth, &wHeight);
	StateServerList::instanceObj.updateServerListContainer(wWidth, wHeight);
	StateServerList::instanceObj.currentUI = &StateServerList::instanceObj.mainUI;
}

void StateServerList::editServerConfirmCallback(void* user) {
	((StateServerList*)user)->servers[((StateServerList*)user)->editedServerIndex].address = ((StateServerList*)user)->editServerAdressInput.text;
	((StateServerList*)user)->servers[((StateServerList*)user)->editedServerIndex].displayedName = ((StateServerList*)user)->editServerNameInput.text;
	((StateServerList*)user)->servers[((StateServerList*)user)->editedServerIndex].playerNameOverride = ((StateServerList*)user)->editServerNameOverrideInput.text;
	((StateServerList*)user)->servers[((StateServerList*)user)->editedServerIndex].uuidOverride = ((StateServerList*)user)->editServerUuidOverrideInput.text;
	((StateServerList*)user)->servers[((StateServerList*)user)->editedServerIndex].skinPathOverride = ((StateServerList*)user)->editServerSkinPathOverrideInput.text;
	((StateServerList*)user)->serverButtons[((StateServerList*)user)->editedServerIndex].setText(((StateServerList*)user)->editServerNameInput.text);
	((StateServerList*)user)->currentUI =&((StateServerList*)user)->mainUI;
	((StateServerList*)user)->saveServers();
}

void StateServerList::editServerRemoveCallback(void* user) {
	((StateServerList*)user)->servers.erase(((StateServerList*)user)->servers.begin() + ((StateServerList*)user)->editedServerIndex);
	((StateServerList*)user)->serverButtons.erase(((StateServerList*)user)->serverButtons.begin() + ((StateServerList*)user)->editedServerIndex);
	((StateServerList*)user)->editServerButtons.erase(((StateServerList*)user)->editServerButtons.begin() + ((StateServerList*)user)->editedServerIndex);
	((StateServerList*)user)->editedServerIndex = -1;
	((StateServerList*)user)->currentUI = &((StateServerList*)user)->mainUI;
	((StateServerList*)user)->saveServers();
	int wWidth, wHeight;
	glfwGetWindowSize(((StateServerList*)user)->currentUI->getGLFWwindow(), &wWidth, &wHeight);
	((StateServerList*)user)->updateServerListContainer(wWidth, wHeight);

}

void StateServerList::editServerCallback(void* user) {
	StateServerList::instanceObj.editServerAdressInput.setText( StateServerList::instanceObj.servers[(int)user].address);
	StateServerList::instanceObj.editServerNameInput.setText(StateServerList::instanceObj.servers[(int)user].displayedName);
	StateServerList::instanceObj.editServerNameOverrideInput.setText(StateServerList::instanceObj.servers[(int)user].playerNameOverride);
	StateServerList::instanceObj.editServerUuidOverrideInput.setText(StateServerList::instanceObj.servers[(int)user].uuidOverride);
	StateServerList::instanceObj.editServerSkinPathOverrideInput.setText(StateServerList::instanceObj.servers[(int)user].skinPathOverride);

	StateServerList::instanceObj.editedServerIndex = (int)user;
	StateServerList::instanceObj.currentUI = &StateServerList::instanceObj.editServerUI;
}

void StateServerList::savePlayerSettingsCallback(void* user) {
	StateServerList::instanceObj.currentUI = &StateServerList::instanceObj.mainUI;
	StateMultiplayer::instanceObj.close(*StateServerList::instanceObj.manager);
	StateMultiplayer::instanceObj.init(*StateServerList::instanceObj.manager);
}

std::filesystem::path getUUIDPath()
{
	wchar_t szFile[MAX_PATH] = {};
	OPENFILENAMEW ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = glfwGetWin32Window(StateServerList::instanceObj.manager->window);
	ofn.hInstance = nullptr;
	ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0\0";
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = MAX_PATH;
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_NOCHANGEDIR;
	if (GetOpenFileNameW(&ofn))
		return std::filesystem::path(ofn.lpstrFile);
	return {};
}

std::filesystem::path getSkinPath()
{
	wchar_t szFile[MAX_PATH] = {};
	OPENFILENAMEW ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = glfwGetWin32Window(StateServerList::instanceObj.manager->window);
	ofn.hInstance = nullptr;
	ofn.lpstrFilter = L"PNG Files (*.png)\0*.png\0All Files (*.*)\0*.*\0\0";
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = MAX_PATH;
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_NOCHANGEDIR;
	if (GetOpenFileNameW(&ofn))
		return std::filesystem::path(ofn.lpstrFile);
	return {};
}

// Inputs and updates
void StateServerList::init(StateManager& s)
{
	int width, height;
	glfwGetWindowSize(s.window, &width, &height);

	qs = ShaderManager::get("quadShader");

	font = { ResourceManager::get("pixelfont.png"), ShaderManager::get("textShader") };

	qr.shader = qs;
	qr.init();

	StateMultiplayer::instanceObj.init(*StateServerList::instanceObj.manager);

	StateSettings::instanceObj.init(*StateServerList::instanceObj.manager);

	StateSkinChooser::instanceObj.init(*StateServerList::instanceObj.manager);

	loadDefaultPlayerSettings();

	// Main UI
	{
		mainTitle.setText("Server List");
		mainTitle.alignX(ALIGN_CENTER_X);
		mainTitle.alignY(ALIGN_TOP);
		mainTitle.offsetY(50);
		mainTitle.size = 4;
		mainTitle.shadow = true;
		mainTitle.fancy = true;

		serverListContainer.parent = &mainUI;
		serverListContainer.alignX(ALIGN_CENTER_X);
		serverListContainer.alignY(ALIGN_CENTER_Y);
		
		returnButton.setText("Cancel");
		returnButton.offsetX(50);
		returnButton.offsetY(-50);
		returnButton.width = 200;
		returnButton.user = &s;
		returnButton.callback = StateMultiplayer::cancelButtonCallback;
		returnButton.alignX(ALIGN_LEFT);
		returnButton.alignY(ALIGN_BOTTOM);

		playerSettingsButton.setText("Player Settings");
		playerSettingsButton.offsetY(-50);
		playerSettingsButton.width = 250;
		playerSettingsButton.user = this;
		playerSettingsButton.callback = [](void* user) {
			((StateServerList*)user)->currentUI= &((StateServerList*)user)->playerSettingsUI;
			};
		playerSettingsButton.alignX(ALIGN_CENTER_X);
		playerSettingsButton.alignY(ALIGN_BOTTOM);

		addServerButton.setText("Add Server");
		addServerButton.offsetX(-50);
		addServerButton.offsetY(-50);
		addServerButton.width = 250;
		addServerButton.callback = addServerCallback;
		addServerButton.alignX(ALIGN_RIGHT);
		addServerButton.alignY(ALIGN_BOTTOM);

		noServersText.setText("No servers yet...");
		noServersText.alignX(ALIGN_CENTER_X);
		noServersText.alignY(ALIGN_CENTER_Y);
		noServersText.shadow = true;
		noServersText.size = 2;
		noServersText.color = { 1,0,0,1 };

		mainUI.addElement(&mainTitle);
		mainUI.addElement(&serverListContainer);
		mainUI.addElement(&returnButton);
		mainUI.addElement(&playerSettingsButton);
		mainUI.addElement(&addServerButton);

		mainUI.window = s.window;
		mainUI.viewportCallback = viewportCallback;
		mainUI.viewportUser = s.window;
		mainUI.font = &font;
		mainUI.qr = &qr;
		mainUI.cursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);

	}

	// Add server UI
	{
		// MAIN TITLE

		addServerTitle.setText("Add Server");
		addServerTitle.alignX(ALIGN_CENTER_X);
		addServerTitle.alignY(ALIGN_TOP);
		addServerTitle.offsetY(50);
		addServerTitle.size = 4;
		addServerTitle.shadow = true;
		addServerTitle.fancy = true;

		// SERVER NAME INPUT

		addServerNameTitle.setText("Displayed Name:");
		addServerNameTitle.alignX(ALIGN_CENTER_X);
		addServerNameTitle.alignY(ALIGN_TOP);
		addServerNameTitle.offsetX(-250);
		addServerNameTitle.offsetY(150);
		addServerNameTitle.size = 2;
		addServerNameTitle.shadow = true;

		addServerNameInput.width = 300;
		addServerNameInput.alignX(ALIGN_CENTER_X);
		addServerNameInput.alignY(ALIGN_TOP);
		addServerNameInput.offsetX(-250);
		addServerNameInput.offsetY(200);

		// SERVER ADDRESS INPUT

		addServerAdressTitle.setText("Adress:");
		addServerAdressTitle.alignX(ALIGN_CENTER_X);
		addServerAdressTitle.alignY(ALIGN_TOP);
		addServerAdressTitle.offsetX(-250);
		addServerAdressTitle.offsetY(300);
		addServerAdressTitle.size = 2;
		addServerAdressTitle.shadow = true;

		addServerAdressInput.width = 300;
		addServerAdressInput.alignX(ALIGN_CENTER_X);
		addServerAdressInput.alignY(ALIGN_TOP);
		addServerAdressInput.offsetX(-250);
		addServerAdressInput.offsetY(350);

		// PLAYER UUID OVERRIDE
	
		addServerUuidOverrideTitle.setText("UUID Override:");
		addServerUuidOverrideTitle.alignX(ALIGN_CENTER_X);
		addServerUuidOverrideTitle.alignY(ALIGN_TOP);
		addServerUuidOverrideTitle.offsetX(250);
		addServerUuidOverrideTitle.offsetY(150);
		addServerUuidOverrideTitle.size = 2;
		addServerUuidOverrideTitle.shadow = true;

		addServerUuidOverrideInput.editable = false;
		addServerUuidOverrideInput.width = 300;
		addServerUuidOverrideInput.alignX(ALIGN_CENTER_X);
		addServerUuidOverrideInput.alignY(ALIGN_TOP);
		addServerUuidOverrideInput.offsetX(250);
		addServerUuidOverrideInput.offsetY(200);

		addServerClearUuidOverrideButton.setText("Clear");
		addServerClearUuidOverrideButton.width = 90;
		addServerClearUuidOverrideButton.callback = [](void* user) {StateServerList::instanceObj.addServerUuidOverrideInput.setText("");};
		addServerClearUuidOverrideButton.alignX(ALIGN_CENTER_X);
		addServerClearUuidOverrideButton.alignY(ALIGN_TOP);
		addServerClearUuidOverrideButton.offsetX(45);
		addServerClearUuidOverrideButton.offsetY(200);

		addServerSelectUuidOverrideButton.setText("Pick");
		addServerSelectUuidOverrideButton.width = 90;
		addServerSelectUuidOverrideButton.callback = [](void* user) {
			StateServerList::instanceObj.addServerUuidOverrideInput.setText(getUUIDPath().string());
			};
		addServerSelectUuidOverrideButton.alignX(ALIGN_CENTER_X);
		addServerSelectUuidOverrideButton.alignY(ALIGN_TOP);
		addServerSelectUuidOverrideButton.offsetX(455);
		addServerSelectUuidOverrideButton.offsetY(200);

		// SKIN PATH OVERRIDE

		addServerSkinPathOverrideTitle.setText("Skin Path Override:");
		addServerSkinPathOverrideTitle.alignX(ALIGN_CENTER_X);
		addServerSkinPathOverrideTitle.alignY(ALIGN_TOP);
		addServerSkinPathOverrideTitle.offsetX(250);
		addServerSkinPathOverrideTitle.offsetY(300);
		addServerSkinPathOverrideTitle.size = 2;
		addServerSkinPathOverrideTitle.shadow = true;

		addServerSkinPathOverrideInput.editable = false;
		addServerSkinPathOverrideInput.width = 300;
		addServerSkinPathOverrideInput.alignX(ALIGN_CENTER_X);
		addServerSkinPathOverrideInput.alignY(ALIGN_TOP);
		addServerSkinPathOverrideInput.offsetX(250);
		addServerSkinPathOverrideInput.offsetY(350);

		addServerClearSkinPathOverrideButton.setText("Clear");
		addServerClearSkinPathOverrideButton.width = 90;
		addServerClearSkinPathOverrideButton.callback = [](void* user) {StateServerList::instanceObj.addServerSkinPathOverrideInput.setText("");};
		addServerClearSkinPathOverrideButton.alignX(ALIGN_CENTER_X);
		addServerClearSkinPathOverrideButton.alignY(ALIGN_TOP);
		addServerClearSkinPathOverrideButton.offsetX(45);
		addServerClearSkinPathOverrideButton.offsetY(350);

		addServerSelectSkinPathOverrideButton.setText("Pick");
		addServerSelectSkinPathOverrideButton.width = 90;
		addServerSelectSkinPathOverrideButton.callback = [](void* user) {
			StateServerList::instanceObj.addServerSkinPathOverrideInput.setText(getSkinPath().string());
			};
		addServerSelectSkinPathOverrideButton.alignX(ALIGN_CENTER_X);
		addServerSelectSkinPathOverrideButton.alignY(ALIGN_TOP);
		addServerSelectSkinPathOverrideButton.offsetX(455);
		addServerSelectSkinPathOverrideButton.offsetY(350);

		// PLAYER NAME OVERRIDE

		addServerNameOverrideTitle.setText("Player Name Override:");
		addServerNameOverrideTitle.alignX(ALIGN_CENTER_X);
		addServerNameOverrideTitle.alignY(ALIGN_TOP);
		addServerNameOverrideTitle.offsetY(450);
		addServerNameOverrideTitle.size = 2;
		addServerNameOverrideTitle.shadow = true;

		addServerNameOverrideInput.width = 300;
		addServerNameOverrideInput.alignX(ALIGN_CENTER_X);
		addServerNameOverrideInput.alignY(ALIGN_TOP);
		addServerNameOverrideInput.offsetY(500);

		// BUTTONS

		addServerCancelButton.setText("Cancel");
		addServerCancelButton.offsetX(50);
		addServerCancelButton.offsetY(-50);
		addServerCancelButton.width = 200;
		addServerCancelButton.user = &s;
		addServerCancelButton.callback = [](void* user) {StateServerList::instanceObj.currentUI = &StateServerList::instanceObj.mainUI;};
		addServerCancelButton.alignX(ALIGN_LEFT);
		addServerCancelButton.alignY(ALIGN_BOTTOM);

		addServerConfirmButton.setText("Add Server");
		addServerConfirmButton.offsetX(-50);
		addServerConfirmButton.offsetY(-50);
		addServerConfirmButton.width = 250;
		addServerConfirmButton.callback = addServerConfirmCallback;
		addServerConfirmButton.alignX(ALIGN_RIGHT);
		addServerConfirmButton.alignY(ALIGN_BOTTOM);

		// FINISH UP

		addServerUI.addElement(&addServerTitle);
		addServerUI.addElement(&addServerAdressTitle);
		addServerUI.addElement(&addServerAdressInput);
		addServerUI.addElement(&addServerNameTitle);
		addServerUI.addElement(&addServerNameInput);
		addServerUI.addElement(&addServerCancelButton);
		addServerUI.addElement(&addServerConfirmButton);

		addServerUI.addElement(&addServerClearSkinPathOverrideButton);
		addServerUI.addElement(&addServerSelectSkinPathOverrideButton);
		addServerUI.addElement(&addServerSkinPathOverrideTitle);
		addServerUI.addElement(&addServerSkinPathOverrideInput);

		addServerUI.addElement(&addServerClearUuidOverrideButton);
		addServerUI.addElement(&addServerSelectUuidOverrideButton);
		addServerUI.addElement(&addServerUuidOverrideTitle);
		addServerUI.addElement(&addServerUuidOverrideInput);

		addServerUI.addElement(&addServerNameOverrideTitle);
		addServerUI.addElement(&addServerNameOverrideInput);

		addServerUI.window = s.window;
		addServerUI.viewportCallback = viewportCallback;
		addServerUI.viewportUser = s.window;
		addServerUI.font = &font;
		addServerUI.qr = &qr;
		addServerUI.cursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
	}

	// Edit server UI
	{
		// TITLE

		editServerTitle.setText("Server Settings");
		editServerTitle.alignX(ALIGN_CENTER_X);
		editServerTitle.alignY(ALIGN_TOP);
		editServerTitle.offsetY(50);
		editServerTitle.size = 4;
		editServerTitle.shadow = true;
		editServerTitle.fancy = true;

		// SERVER NAME

		editServerNameTitle.setText("Displayed Name:");
		editServerNameTitle.alignX(ALIGN_CENTER_X);
		editServerNameTitle.alignY(ALIGN_TOP);
		editServerNameTitle.offsetX(-250);
		editServerNameTitle.offsetY(150);
		editServerNameTitle.size = 2;
		editServerNameTitle.shadow = true;

		editServerNameInput.width = 300;
		editServerNameInput.alignX(ALIGN_CENTER_X);
		editServerNameInput.alignY(ALIGN_TOP);
		editServerNameInput.offsetX(-250);
		editServerNameInput.offsetY(200);

		// SERVER ADDRESS

		editServerAdressTitle.setText("Adress:");
		editServerAdressTitle.alignX(ALIGN_CENTER_X);
		editServerAdressTitle.alignY(ALIGN_TOP);
		editServerAdressTitle.offsetX(-250);
		editServerAdressTitle.offsetY(300);
		editServerAdressTitle.size = 2;
		editServerAdressTitle.shadow = true;

		editServerAdressInput.width = 300;
		editServerAdressInput.alignX(ALIGN_CENTER_X);
		editServerAdressInput.alignY(ALIGN_TOP);
		editServerAdressInput.offsetX(-250);
		editServerAdressInput.offsetY(350);

		// PLAYER UUID OVERRIDE

		editServerUuidOverrideTitle.setText("UUID Override:");
		editServerUuidOverrideTitle.alignX(ALIGN_CENTER_X);
		editServerUuidOverrideTitle.alignY(ALIGN_TOP);
		editServerUuidOverrideTitle.offsetX(250);
		editServerUuidOverrideTitle.offsetY(150);
		editServerUuidOverrideTitle.size = 2;
		editServerUuidOverrideTitle.shadow = true;

		editServerUuidOverrideInput.editable = false;
		editServerUuidOverrideInput.width = 300;
		editServerUuidOverrideInput.alignX(ALIGN_CENTER_X);
		editServerUuidOverrideInput.alignY(ALIGN_TOP);
		editServerUuidOverrideInput.offsetX(250);
		editServerUuidOverrideInput.offsetY(200);

		editServerClearUuidOverrideButton.setText("Clear");
		editServerClearUuidOverrideButton.width = 90;
		editServerClearUuidOverrideButton.callback = [](void* user) {StateServerList::instanceObj.editServerUuidOverrideInput.setText("");};
		editServerClearUuidOverrideButton.alignX(ALIGN_CENTER_X);
		editServerClearUuidOverrideButton.alignY(ALIGN_TOP);
		editServerClearUuidOverrideButton.offsetX(45);
		editServerClearUuidOverrideButton.offsetY(200);

		editServerSelectUuidOverrideButton.setText("Pick");
		editServerSelectUuidOverrideButton.width = 90;
		editServerSelectUuidOverrideButton.callback = [](void* user) {
			StateServerList::instanceObj.editServerUuidOverrideInput.setText(getUUIDPath().string());
			};
		editServerSelectUuidOverrideButton.alignX(ALIGN_CENTER_X);
		editServerSelectUuidOverrideButton.alignY(ALIGN_TOP);
		editServerSelectUuidOverrideButton.offsetX(455);
		editServerSelectUuidOverrideButton.offsetY(200);

		// SKIN PATH OVERRIDE

		editServerSkinPathOverrideTitle.setText("Skin Path Override:");
		editServerSkinPathOverrideTitle.alignX(ALIGN_CENTER_X);
		editServerSkinPathOverrideTitle.alignY(ALIGN_TOP);
		editServerSkinPathOverrideTitle.offsetX(250);
		editServerSkinPathOverrideTitle.offsetY(300);
		editServerSkinPathOverrideTitle.size = 2;
		editServerSkinPathOverrideTitle.shadow = true;

		editServerSkinPathOverrideInput.editable = false;
		editServerSkinPathOverrideInput.width = 300;
		editServerSkinPathOverrideInput.alignX(ALIGN_CENTER_X);
		editServerSkinPathOverrideInput.alignY(ALIGN_TOP);
		editServerSkinPathOverrideInput.offsetX(250);
		editServerSkinPathOverrideInput.offsetY(350);

		editServerClearSkinPathOverrideButton.setText("Clear");
		editServerClearSkinPathOverrideButton.width = 90;
		editServerClearSkinPathOverrideButton.callback = [](void* user) {StateServerList::instanceObj.editServerSkinPathOverrideInput.setText("");};
		editServerClearSkinPathOverrideButton.alignX(ALIGN_CENTER_X);
		editServerClearSkinPathOverrideButton.alignY(ALIGN_TOP);
		editServerClearSkinPathOverrideButton.offsetX(45);
		editServerClearSkinPathOverrideButton.offsetY(350);

		editServerSelectSkinPathOverrideButton.setText("Pick");
		editServerSelectSkinPathOverrideButton.width = 90;
		editServerSelectSkinPathOverrideButton.callback = [](void* user) {
			StateServerList::instanceObj.editServerSkinPathOverrideInput.setText(getSkinPath().string());
			};
		editServerSelectSkinPathOverrideButton.alignX(ALIGN_CENTER_X);
		editServerSelectSkinPathOverrideButton.alignY(ALIGN_TOP);
		editServerSelectSkinPathOverrideButton.offsetX(455);
		editServerSelectSkinPathOverrideButton.offsetY(350);

		// PLAYER NAME OVERRIDE

		editServerNameOverrideTitle.setText("Player Name Override:");
		editServerNameOverrideTitle.alignX(ALIGN_CENTER_X);
		editServerNameOverrideTitle.alignY(ALIGN_TOP);
		editServerNameOverrideTitle.offsetY(450);
		editServerNameOverrideTitle.size = 2;
		editServerNameOverrideTitle.shadow = true;

		editServerNameOverrideInput.width = 300;
		editServerNameOverrideInput.alignX(ALIGN_CENTER_X);
		editServerNameOverrideInput.alignY(ALIGN_TOP);
		editServerNameOverrideInput.offsetY(500);

		// BUTTONS

		editServerCancelButton.setText("Cancel");
		editServerCancelButton.offsetX(50);
		editServerCancelButton.offsetY(-50);
		editServerCancelButton.width = 200;
		editServerCancelButton.user = &s;
		editServerCancelButton.callback = [](void* user) {StateServerList::instanceObj.currentUI = &StateServerList::instanceObj.mainUI;};
		editServerCancelButton.alignX(ALIGN_LEFT);
		editServerCancelButton.alignY(ALIGN_BOTTOM);

		editServerConfirmButton.setText("Save");
		editServerConfirmButton.offsetX(-50);
		editServerConfirmButton.offsetY(-50);
		editServerConfirmButton.width = 200;
		editServerConfirmButton.user = this;
		editServerConfirmButton.callback = editServerConfirmCallback;
		editServerConfirmButton.alignX(ALIGN_RIGHT);
		editServerConfirmButton.alignY(ALIGN_BOTTOM);

		editServerRemoveButton.setText("Delete");
		editServerRemoveButton.offsetY(-50);
		editServerRemoveButton.width = 200;
		editServerRemoveButton.user = this;
		editServerRemoveButton.callback = editServerRemoveCallback;
		editServerRemoveButton.alignY(ALIGN_BOTTOM);
		editServerRemoveButton.alignX(ALIGN_CENTER_X);

		editServerUI.addElement(&editServerTitle);
		editServerUI.addElement(&editServerAdressTitle);
		editServerUI.addElement(&editServerAdressInput);
		editServerUI.addElement(&editServerNameTitle);
		editServerUI.addElement(&editServerNameInput);
		editServerUI.addElement(&editServerCancelButton);
		editServerUI.addElement(&editServerConfirmButton);
		editServerUI.addElement(&editServerRemoveButton);

		editServerUI.addElement(&editServerClearSkinPathOverrideButton);
		editServerUI.addElement(&editServerSelectSkinPathOverrideButton);
		editServerUI.addElement(&editServerSkinPathOverrideTitle);
		editServerUI.addElement(&editServerSkinPathOverrideInput);

		editServerUI.addElement(&editServerClearUuidOverrideButton);
		editServerUI.addElement(&editServerSelectUuidOverrideButton);
		editServerUI.addElement(&editServerUuidOverrideTitle);
		editServerUI.addElement(&editServerUuidOverrideInput);

		editServerUI.addElement(&editServerNameOverrideTitle);
		editServerUI.addElement(&editServerNameOverrideInput);

		editServerUI.window = s.window;
		editServerUI.viewportCallback = viewportCallback;
		editServerUI.viewportUser = s.window;
		editServerUI.font = &font;
		editServerUI.qr = &qr;
		editServerUI.cursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
	}

	// Player settings UI
	{
		playerSettingsTitle.setText("Player Settings");
		playerSettingsTitle.alignX(ALIGN_CENTER_X);
		playerSettingsTitle.alignY(ALIGN_TOP);
		playerSettingsTitle.offsetY(50);
		playerSettingsTitle.size = 4;
		playerSettingsTitle.shadow = true;
		playerSettingsTitle.fancy = true;

		playerSettingsReturnButton.setText("Return");
		playerSettingsReturnButton.offsetX(50);
		playerSettingsReturnButton.offsetY(-50);
		playerSettingsReturnButton.width = 200;
		playerSettingsReturnButton.user = &s;
		playerSettingsReturnButton.callback = savePlayerSettingsCallback;
		playerSettingsReturnButton.alignX(ALIGN_LEFT);
		playerSettingsReturnButton.alignY(ALIGN_BOTTOM);

		playerSettingsUI.addElement(&playerSettingsTitle);
		playerSettingsUI.addElement(&playerSettingsReturnButton);
		playerSettingsUI.addElement(&StateMultiplayer::instanceObj.changeSkinButton);
		playerSettingsUI.addElement(&StateMultiplayer::instanceObj.displayNameInput);
		playerSettingsUI.addElement(&StateMultiplayer::instanceObj.displayNameLabel);
		playerSettingsUI.addElement(&StateMultiplayer::instanceObj.uuidCopyButton);
		playerSettingsUI.addElement(&StateMultiplayer::instanceObj.uuidInput);
		playerSettingsUI.addElement(&StateMultiplayer::instanceObj.uuidDescription);
		playerSettingsUI.addElement(&StateMultiplayer::instanceObj.uuidLabel);

		playerSettingsUI.window = s.window;
		playerSettingsUI.viewportCallback = viewportCallback;
		playerSettingsUI.viewportUser = s.window;
		playerSettingsUI.font = &font;
		playerSettingsUI.qr = &qr;
		playerSettingsUI.cursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);

	}


	loadServers();

	int wWidth, wHeight;
	glfwGetWindowSize(s.window,&wWidth,&wHeight);
	updateServerListContainer(wWidth, wHeight);

	currentUI = &mainUI;
	updateProjection({ width, height });
}
void StateServerList::render(StateManager& s)
{
	glEnable(GL_DEPTH_TEST);
	StateTitleScreen::instanceObj.render(s);

	// get width and height of window
	int w, h;
	glfwGetWindowSize(s.window, &w, &h);

	glDisable(GL_DEPTH_TEST);

	// render a background using the QuadRenderer
	qr.setColor(0, 0, 0, 0.6f);
	qr.setPos(0, 0, w, h);
	qr.setQuadRendererMode(QuadRenderer::MODE_FILL);
	qr.render();

	currentUI->render();

	glEnable(GL_DEPTH_TEST);
}

void StateServerList::mouseInput(StateManager& s, double xPos, double yPos)
{
	currentUI->mouseInput(xPos, yPos);
}
void StateServerList::scrollInput(StateManager& s, double xOff, double yOff)
{
	currentUI->scrollInput(xOff, yOff);
}
void StateServerList::keyInput(StateManager& s, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		if (currentUI == &mainUI)
			s.popState();
		else
			currentUI = &mainUI;
	else
		currentUI->keyInput(key, scancode, action, mods);

}
void StateServerList::charInput(StateManager& s, uint32_t codepoint)
{
	currentUI->charInput(codepoint);
}
void StateServerList::mouseButtonInput(StateManager& s, int btn, int action, int mods)
{
	currentUI->mouseButtonInput(btn, action, mods);
}
void StateServerList::update(StateManager& s, double dt)
{
	StateTitleScreen::instanceObj.update(s, dt);

	int w, h;
	glfwGetWindowSize(s.window, &w, &h);

}
void StateServerList::close(StateManager& s)
{
	mainUI.clear();
}
void StateServerList::windowResize(StateManager& s, int w, int h)
{
	updateProjection({ w, h });
	updateServerListContainer(w, h);
}

$hook(void, StateTitleScreen, init, StateManager& s) {
	original(self, s);
	StateServerList::instanceObj.manager = &s;
	self->multiplayerButton.callback = [](void* user) {
		StateServerList::instanceObj.manager->pushState(&StateServerList::instanceObj);
		StateServerList::instanceObj.init(*StateServerList::instanceObj.manager);
		};
}

// Dont load skin when loading settings wtf
bool inGame = false;
bool isLoadingSettings = false;

$hook(void, StateGame, init, StateManager& s) {
	inGame = true;
	original(self, s);
}

$hook(void, StateGame, init, StateManager& s) {
	inGame = false;
	original(self, s);
}

$hook(void, StateSettings,load, GLFWwindow * window) {
	isLoadingSettings = true;
	original(self, window);
	isLoadingSettings = false;
}

$hook(bool,StateSkinChooser, loadSkin, const stl::path& p) {
	if (!isLoadingSettings || !inGame) return original(self, p);
	return false;
}
