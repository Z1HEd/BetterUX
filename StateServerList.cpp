#include "StateServerList.h"
#include "fstream"

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
			{"address", address}
	};;
}

StateServerList::ServerInfo StateServerList::ServerInfo::createFromJSON(nlohmann::json j) {
	return{ j.at("address").get<std::string>(),
			j.at("displayedName").get<std::string>() };
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

// Buttons callbacks

void StateServerList::addServerCallback(void* user) {
	StateServerList::instanceObj.currentUI = &StateServerList::instanceObj.addServerUI;
}

void StateServerList::serverButtonCallback(void* user) {
	StateServerList::instanceObj.manager->changeState(&StateMultiplayer::instanceObj);
	StateMultiplayer::instanceObj.serverAddressInput.setText(((ServerInfo*)user)->address);
	StateMultiplayer::instanceObj.joinButtonCallback(StateMultiplayer::instanceObj.joinButton.user);

}

void StateServerList::addServerConfirmCallback(void* user) {
	StateServerList::instanceObj.servers.push_back({
		StateServerList::instanceObj.addServerAdressInput.text,
		StateServerList::instanceObj.addServerNameInput.text});
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
	StateServerList::instanceObj.editedServerIndex = (int)user;
	StateServerList::instanceObj.currentUI = &StateServerList::instanceObj.editServerUI;
}

void StateServerList::savePlayerSettingsCallback(void* user) {
	StateServerList::instanceObj.currentUI = &StateServerList::instanceObj.mainUI;
	StateMultiplayer::instanceObj.close(*StateServerList::instanceObj.manager);
	StateMultiplayer::instanceObj.init(*StateServerList::instanceObj.manager);
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
	StateMultiplayer::instanceObj.connectionErrorOkButton.callback =
		[](void* user) {StateServerList::instanceObj.manager->changeState(&StateServerList::instanceObj);};

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
		addServerTitle.setText("Add Server");
		addServerTitle.alignX(ALIGN_CENTER_X);
		addServerTitle.alignY(ALIGN_TOP);
		addServerTitle.offsetY(50);
		addServerTitle.size = 4;
		addServerTitle.shadow = true;
		addServerTitle.fancy = true;

		addServerNameTitle.setText("Displayed Name:");
		addServerNameTitle.alignX(ALIGN_CENTER_X);
		addServerNameTitle.alignY(ALIGN_TOP);
		addServerNameTitle.offsetY(150);
		addServerNameTitle.size = 2;
		addServerNameTitle.shadow = true;

		addServerNameInput.width = 300;
		addServerNameInput.alignX(ALIGN_CENTER_X);
		addServerNameInput.alignY(ALIGN_TOP);
		addServerNameInput.offsetY(200);

		addServerAdressTitle.setText("Adress:");
		addServerAdressTitle.alignX(ALIGN_CENTER_X);
		addServerAdressTitle.alignY(ALIGN_TOP);
		addServerAdressTitle.offsetY(300);
		addServerAdressTitle.size = 2;
		addServerAdressTitle.shadow = true;

		addServerAdressInput.width = 300;
		addServerAdressInput.alignX(ALIGN_CENTER_X);
		addServerAdressInput.alignY(ALIGN_TOP);
		addServerAdressInput.offsetY(350);

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
	
		addServerUI.addElement(&addServerTitle);
		addServerUI.addElement(&addServerAdressTitle);
		addServerUI.addElement(&addServerAdressInput);
		addServerUI.addElement(&addServerNameTitle);
		addServerUI.addElement(&addServerNameInput);
		addServerUI.addElement(&addServerCancelButton);
		addServerUI.addElement(&addServerConfirmButton);

		addServerUI.window = s.window;
		addServerUI.viewportCallback = viewportCallback;
		addServerUI.viewportUser = s.window;
		addServerUI.font = &font;
		addServerUI.qr = &qr;
		addServerUI.cursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
	}

	// Edit server UI
	{
		editServerTitle.setText("Server Settings");
		editServerTitle.alignX(ALIGN_CENTER_X);
		editServerTitle.alignY(ALIGN_TOP);
		editServerTitle.offsetY(50);
		editServerTitle.size = 4;
		editServerTitle.shadow = true;
		editServerTitle.fancy = true;

		editServerNameTitle.setText("Displayed Name:");
		editServerNameTitle.alignX(ALIGN_CENTER_X);
		editServerNameTitle.alignY(ALIGN_TOP);
		editServerNameTitle.offsetY(150);
		editServerNameTitle.size = 2;
		editServerNameTitle.shadow = true;

		editServerNameInput.width = 300;
		editServerNameInput.alignX(ALIGN_CENTER_X);
		editServerNameInput.alignY(ALIGN_TOP);
		editServerNameInput.offsetY(200);

		editServerAdressTitle.setText("Adress:");
		editServerAdressTitle.alignX(ALIGN_CENTER_X);
		editServerAdressTitle.alignY(ALIGN_TOP);
		editServerAdressTitle.offsetY(300);
		editServerAdressTitle.size = 2;
		editServerAdressTitle.shadow = true;

		editServerAdressInput.width = 300;
		editServerAdressInput.alignX(ALIGN_CENTER_X);
		editServerAdressInput.alignY(ALIGN_TOP);
		editServerAdressInput.offsetY(350);

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