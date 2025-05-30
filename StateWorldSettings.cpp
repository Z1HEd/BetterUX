#include "StateWorldSettings.h"
#include <fstream>

StateWorldSettings StateWorldSettings::instanceObj = StateWorldSettings();

void StateWorldSettings::updateProjection(const glm::ivec2& size)
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

	copyingLogText.wrapWidth = size.x;
}

void StateWorldSettings::viewportCallback(void* user, const glm::ivec4& pos, const glm::ivec2& scroll)
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

void StateWorldSettings::updateSelectedWorld() {
	std::ifstream infoFile(worldFolder + "/info.json");
	nlohmann::json infoJson = nlohmann::json::parse(infoFile);

	worldName = infoJson["name"];

	folderInput.setText(worldFolder.substr(7));
	nameInput.setText(worldName);

	copyFolderInput.setText(worldFolder.substr(7)+"(copy)");
	copyNameInput.setText(worldName + " Copy");

	deleteNameTitle.setText("Type \"" + worldName + "\" to delete:");
}

void StateWorldSettings::init(StateManager& s)
{
	int width, height;
	glfwGetWindowSize(s.window, &width, &height);

	qs = ShaderManager::get("quadShader");

	font = { ResourceManager::get("pixelfont.png"), ShaderManager::get("textShader") };

	qr.shader = qs;
	qr.init();
	// Main UI
	{ 
		mainTitle.setText("World Settings");
		mainTitle.alignX(ALIGN_CENTER_X);
		mainTitle.alignY(ALIGN_TOP);
		mainTitle.offsetY( 50);
		mainTitle.size = 4;
		mainTitle.shadow = true;
		mainTitle.fancy = true;

		mainUI.window = s.window;
		mainUI.viewportCallback = viewportCallback;
		mainUI.viewportUser = s.window;
		mainUI.font = &font;
		mainUI.qr = &qr;
		mainUI.cursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);

		folderTitle.setText("World Folder Name:");
		folderTitle.size = 2;
		folderTitle.shadow = true;
		folderTitle.alignX(ALIGN_CENTER_X);
		folderTitle.alignY(ALIGN_TOP);
		folderTitle.offsetY(150);

		folderInput.alignX(ALIGN_CENTER_X);
		folderInput.alignY(ALIGN_TOP);
		folderInput.width = 300;
		folderInput.offsetY(180);

		nameTitle.setText("World Name:");
		nameTitle.size = 2;
		nameTitle.shadow = true;
		nameTitle.alignX(ALIGN_CENTER_X);
		nameTitle.alignY(ALIGN_TOP);
		nameTitle.offsetY(260);

		nameInput.alignX(ALIGN_CENTER_X);
		nameInput.alignY(ALIGN_TOP);
		nameInput.width = 300;
		nameInput.offsetY(290);

		deleteButton.setText("Delete");
		deleteButton.width = 300;
		deleteButton.offsetY(370);
		deleteButton.alignX(ALIGN_CENTER_X);
		deleteButton.alignY(ALIGN_TOP);
		deleteButton.user = this;
		deleteButton.callback = deleteButtonCallback;

		copyButton.setText("Copy");
		copyButton.width = 300;
		copyButton.offsetY(430);
		copyButton.alignX(ALIGN_CENTER_X);
		copyButton.alignY(ALIGN_TOP);
		copyButton.user = this;
		copyButton.callback = copyButtonCallback;

		cancelButton.setText("Cancel");
		cancelButton.width = 144;
		cancelButton.offsetY(490);
		cancelButton.offsetX(-78);
		cancelButton.alignX(ALIGN_CENTER_X);
		cancelButton.alignY(ALIGN_TOP);
		cancelButton.user = &s;
		cancelButton.callback = returnButtonCallback;
		
		saveButton.setText("Save");
		saveButton.width = 144;
		saveButton.offsetY(490);
		saveButton.offsetX(78);
		saveButton.alignX(ALIGN_CENTER_X);
		saveButton.alignY(ALIGN_TOP);
		saveButton.user = &s;
		saveButton.callback = saveButtonCallback;

		mainUI.addElement(&mainTitle);
		mainUI.addElement(&folderTitle);
		mainUI.addElement(&folderInput);
		mainUI.addElement(&nameTitle);
		mainUI.addElement(&nameInput);
		mainUI.addElement(&deleteButton);
		mainUI.addElement(&copyButton);
		mainUI.addElement(&cancelButton);
		mainUI.addElement(&saveButton);
	}

	// Copy UI
	{
		copyUI.window = s.window;
		copyUI.viewportCallback = viewportCallback;
		copyUI.viewportUser = s.window;
		copyUI.font = &font;
		copyUI.qr = &qr;
		copyUI.cursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);

		copyTitle.setText("Copy World");
		copyTitle.alignX(ALIGN_CENTER_X);
		copyTitle.alignY(ALIGN_TOP);
		copyTitle.offsetY(50);
		copyTitle.size = 4;
		copyTitle.shadow = true;
		copyTitle.fancy = true;

		copyFolderTitle.setText("Copy Folder Name:");
		copyFolderTitle.size = 2;
		copyFolderTitle.shadow = true;
		copyFolderTitle.alignX(ALIGN_CENTER_X);
		copyFolderTitle.alignY(ALIGN_TOP);
		copyFolderTitle.offsetY(150);

		copyFolderInput.alignX(ALIGN_CENTER_X);
		copyFolderInput.alignY(ALIGN_TOP);
		copyFolderInput.width = 300;
		copyFolderInput.offsetY(180);

		copyNameTitle.setText("Copy Name:");
		copyNameTitle.size = 2;
		copyNameTitle.shadow = true;
		copyNameTitle.alignX(ALIGN_CENTER_X);
		copyNameTitle.alignY(ALIGN_TOP);
		copyNameTitle.offsetY(260);

		copyNameInput.alignX(ALIGN_CENTER_X);
		copyNameInput.alignY(ALIGN_TOP);
		copyNameInput.width = 300;
		copyNameInput.offsetY(290);

		copyConfirmButton.setText("Copy");
		copyConfirmButton.width = 144;
		copyConfirmButton.offsetY(370);
		copyConfirmButton.offsetX(78);
		copyConfirmButton.alignX(ALIGN_CENTER_X);
		copyConfirmButton.alignY(ALIGN_TOP);
		copyConfirmButton.user = &s;
		copyConfirmButton.callback = copyConfirmButtonCallback;

		copyCancelButton.setText("Cancel");
		copyCancelButton.width = 144;
		copyCancelButton.offsetY(370);
		copyCancelButton.offsetX(-78);
		copyCancelButton.alignX(ALIGN_CENTER_X);
		copyCancelButton.alignY(ALIGN_TOP);
		copyCancelButton.callback = copyCancelButtonCallback;

		copyUI.addElement(&copyTitle);
		copyUI.addElement(&copyFolderTitle);
		copyUI.addElement(&copyFolderInput);
		copyUI.addElement(&copyNameTitle);
		copyUI.addElement(&copyNameInput);
		copyUI.addElement(&copyConfirmButton);
		copyUI.addElement(&copyCancelButton);
	}

	// Copy in progress UI
	{
		copyingUI.window = s.window;
		copyingUI.viewportCallback = viewportCallback;
		copyingUI.viewportUser = s.window;
		copyingUI.font = &font;
		copyingUI.qr = &qr;
		copyingUI.cursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);

		copyingTitle.setText("Copying...");
		copyingTitle.alignX(ALIGN_CENTER_X);
		copyingTitle.alignY(ALIGN_TOP);
		copyingTitle.offsetY(50);
		copyingTitle.size = 4;
		copyingTitle.shadow = true;
		copyingTitle.fancy = true;

		copyingLogText.setText("");
		copyingLogText.shadow = true;
		copyingLogText.size = 1;
		copyingLogText.alignX(ALIGN_LEFT);
		copyingLogText.alignY(ALIGN_TOP);
		copyingLogText.wrapWidth = width - 40;
		copyingLogText.offsetX(20);
		copyingLogText.offsetY(100);
		copyingLogText.color.a = 0.7f;

		copyingUI.addElement(&copyingLogText);
		copyingUI.addElement(&copyingTitle);
	}

	// Delete UI
	{
		deleteUI.window = s.window;
		deleteUI.viewportCallback = viewportCallback;
		deleteUI.viewportUser = s.window;
		deleteUI.font = &font;
		deleteUI.qr = &qr;
		deleteUI.cursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);

		deleteTitle.setText("Delete World");
		deleteTitle.alignX(ALIGN_CENTER_X);
		deleteTitle.alignY(ALIGN_TOP);
		deleteTitle.offsetY(50);
		deleteTitle.size = 4;
		deleteTitle.shadow = true;
		deleteTitle.fancy = true;

		deleteNameTitle.size = 2;
		deleteNameTitle.shadow = true;
		deleteNameTitle.alignX(ALIGN_CENTER_X);
		deleteNameTitle.alignY(ALIGN_TOP);
		deleteNameTitle.offsetY(260);

		deleteNameInput.alignX(ALIGN_CENTER_X);
		deleteNameInput.alignY(ALIGN_TOP);
		deleteNameInput.width = 300;
		deleteNameInput.offsetY(290);

		deleteConfirmButton.setText("Perish");
		deleteConfirmButton.width = 144;
		deleteConfirmButton.offsetY(370);
		deleteConfirmButton.offsetX(78);
		deleteConfirmButton.alignX(ALIGN_CENTER_X);
		deleteConfirmButton.alignY(ALIGN_TOP);
		deleteConfirmButton.user = &s;
		deleteConfirmButton.callback = deleteConfirmButtonCallback;
		deleteConfirmButton.clickable = false;

		deleteCancelButton.setText("Cancel");
		deleteCancelButton.width = 144;
		deleteCancelButton.offsetY(370);
		deleteCancelButton.offsetX(-78);
		deleteCancelButton.alignX(ALIGN_CENTER_X);
		deleteCancelButton.alignY(ALIGN_TOP);
		deleteCancelButton.callback = deleteCancelButtonCallback;

		deleteUI.addElement(&deleteTitle);
		deleteUI.addElement(&deleteNameTitle);
		deleteUI.addElement(&deleteNameInput);
		deleteUI.addElement(&deleteConfirmButton);
		deleteUI.addElement(&deleteCancelButton);
	}

	ui = &mainUI;
	updateProjection({ width, height });
}

void StateWorldSettings::render(StateManager& s)
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

	// render the ui
	ui->render();

	if (copying && copyFilesTotal > 0)
	{
		// bar fill
		qr.setColor(
			{
				{ 0,1,1,1 },
				{ 0,1,1,1 },
				{ 1,0,1,1 },
				{ 1,0,1,1 },
			});
		qr.setPos(20, h - 100 + 40, (w - 40) * ((float)(int)copyFilesCopied / (float)(int)copyFilesTotal), 30);
		qr.setQuadRendererMode(QuadRenderer::MODE_FILL);
		qr.render();

		// bar outline
		qr.setColor(1, 1, 1, 1);
		qr.setPos(20, h - 100 + 40, w - 40, 30);
		qr.setQuadRendererMode(QuadRenderer::MODE_LINES);
		qr.render();

		font.centered = true;
		font.fontSize = 2;
		font.pos = glm::ivec2{ w / 2, h - 100 + 40 + 15 };
		font.setText(std::format("{}/{}", (int)copyFilesCopied, (int)copyFilesTotal));

		// shadow
		font.color = glm::vec4{ 0,0,0,0.7f };
		font.pos += glm::ivec2{ 2, 2 };
		font.updateModel();
		font.render();

		// text
		font.color = glm::vec4{ 1 };
		font.pos -= glm::ivec2{ 2, 2 };
		font.updateModel();
		font.render();
	}

	glEnable(GL_DEPTH_TEST);
}

// Button callbacks

inline void StateWorldSettings::deleteButtonCallback(void* user)
{
	StateWorldSettings* worldSettings = (StateWorldSettings*)user;
	worldSettings->ui = &worldSettings->deleteUI;
}

inline void StateWorldSettings::copyButtonCallback(void* user)
{
	StateWorldSettings* worldSettings = (StateWorldSettings*)user;
	worldSettings->ui = &worldSettings->copyUI;
}

inline void StateWorldSettings::returnButtonCallback(void* user)
{
	StateManager* stateManager = (StateManager*)user;
	stateManager->popState();
}

inline void StateWorldSettings::saveButtonCallback(void* user)
{
	StateWorldSettings* worldSettings = &StateWorldSettings::instanceObj;
	StateManager* stateManager = (StateManager*)user;

	std::ifstream infoFile(std::format("{}/info.json", worldSettings->worldFolder));
	nlohmann::json infoJson = nlohmann::json::parse(infoFile);
	infoFile.close();

	infoJson["name"] = worldSettings->nameInput.text;

	std::ofstream outputInfoFile(std::format("{}/info.json", worldSettings->worldFolder));
	outputInfoFile << infoJson.dump();
	outputInfoFile.close();

	if (instanceObj.folderInput.text == instanceObj.worldFolder) return stateManager->popState();

	std::filesystem::rename(worldSettings->worldFolder,std::format("worlds/{}", worldSettings->folderInput.text));

	StateSingleplayer::instanceObj.init(*stateManager);

	stateManager->popState();
}

inline void StateWorldSettings::copyConfirmButtonCallback(void* user)
{
	using std::filesystem::path;

	StateManager* stateManager = (StateManager*)user;
	StateWorldSettings& worldSettings = StateWorldSettings::instanceObj;

	worldSettings.copyLog.clear();

	worldSettings.copyFinished = false;
	worldSettings.copying = true;
	worldSettings.copyFilesTotal = 0;
	worldSettings.copyFilesCopied = 0;
	worldSettings.ui = &worldSettings.copyingUI;

	worldSettings.copyThread = std::thread([]() {
		StateWorldSettings& worldSettings = StateWorldSettings::instanceObj;

		path newWorldPath = path("worlds") / worldSettings.copyFolderInput.text;

		// count files
		worldSettings.copyFilesTotal = 0;
		for (auto& entry : std::filesystem::recursive_directory_iterator(worldSettings.worldFolder))
		{
			if (!entry.is_directory())
				++worldSettings.copyFilesTotal;
		}
		// copy files
		worldSettings.copyFilesCopied = 0;
		for (auto& entry : std::filesystem::recursive_directory_iterator(worldSettings.worldFolder))
		{
			try
			{
				path relative = std::filesystem::relative(entry.path(), worldSettings.worldFolder);
				//printf("entry: \"%s\": ", entry.path().string().c_str());
				if (!entry.is_directory())
				{
					//printf("file\n");
					if (!std::filesystem::exists(newWorldPath / relative.parent_path()))
					{
						//printf("creating missing dir \"%s\"\n", (newWorldPath / relative.parent_path()).string().c_str());
						std::filesystem::create_directories(newWorldPath / relative.parent_path());
					}
					path newPath = newWorldPath / std::filesystem::relative(entry.path(), worldSettings.worldFolder);
					std::filesystem::copy(
						std::filesystem::absolute(entry.path()),
						std::filesystem::absolute(newPath),
						std::filesystem::copy_options::overwrite_existing);
					{
						std::lock_guard lock{ worldSettings.copyLogMutex };
						worldSettings.copyLog.emplace_back(
							std::format("{}\n",
								std::filesystem::relative(newPath, "./worlds").string()));
					}
					++worldSettings.copyFilesCopied;
				}
			}
			catch (const std::filesystem::filesystem_error& e)
			{
				std::lock_guard lock{ worldSettings.copyLogMutex };
				std::string err = std::format("Failed to copy \"{}\": Exception: \"{}\"\n", entry.path().string(), e.what());
				worldSettings.copyLog.emplace_back(err);
				printf("%s", err.c_str());
			}
		}

		std::ifstream infoFile(newWorldPath / "info.json");
		if (infoFile.is_open())
		{
			nlohmann::json infoJson = nlohmann::json::parse(infoFile);
			infoFile.close();

			infoJson["name"] = worldSettings.copyNameInput.text;

			std::ofstream infoFileOut(newWorldPath / "info.json");
			if (infoFileOut.is_open())
			{
				infoFileOut << infoJson.dump();
				infoFileOut.close();
			}
		}

		worldSettings.copyFinished = true;
	});
	worldSettings.copyThread.detach();
}

inline void StateWorldSettings::copyCancelButtonCallback(void* user)
{
	StateWorldSettings::instanceObj.ui = &StateWorldSettings::instanceObj.mainUI;
}

inline void StateWorldSettings::deleteConfirmButtonCallback(void* user)
{
	std::filesystem::remove_all(StateWorldSettings::instanceObj.worldFolder);

	StateWorldSettings::instanceObj.deleteNameInput.setText("");

	StateWorldSettings::instanceObj.ui = &StateWorldSettings::instanceObj.mainUI;
	
	StateManager* stateManager = (StateManager*)user;

	StateSingleplayer::instanceObj.worldListContainer.clear();
	StateSingleplayer::instanceObj.init(*stateManager);

	stateManager->popState();
}

inline void StateWorldSettings::deleteCancelButtonCallback(void* user)
{
	StateWorldSettings::instanceObj.ui = &StateWorldSettings::instanceObj.mainUI;
}

// Inputs and updates
void StateWorldSettings::mouseInput(StateManager& s, double xPos, double yPos)
{
	if (!copying)
		ui->mouseInput(xPos, yPos);
}
void StateWorldSettings::scrollInput(StateManager& s, double xOff, double yOff)
{
	if (!copying)
		ui->scrollInput(xOff, yOff);
}
void StateWorldSettings::keyInput(StateManager& s, int key, int scancode, int action, int mods)
{
	if (!copying)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			if (ui == &mainUI)
				s.popState();
			else
				ui = &mainUI;
		else
			ui->keyInput(key, scancode, action, mods);

		copyConfirmButton.clickable = !std::filesystem::exists(std::format("worlds/{}", copyFolderInput.text));
		copyConfirmButton.setText(copyConfirmButton.clickable ? "Copy" : "Exists!");

		deleteConfirmButton.clickable = deleteNameInput.text == worldName;

		saveButton.clickable =
			!std::filesystem::exists(std::format("worlds/{}", folderInput.text)) ||
			worldFolder == std::format("worlds\\{}", folderInput.text);

		saveButton.setText(saveButton.clickable ? "Save" : "Exists!");
	}

}
void StateWorldSettings::charInput(StateManager& s, uint32_t codepoint)
{
	if (!copying)
		ui->charInput(codepoint);
}
void StateWorldSettings::mouseButtonInput(StateManager& s, int btn, int action, int mods)
{
	if (!copying)
		ui->mouseButtonInput(btn, action, mods);
}
void StateWorldSettings::update(StateManager& s, double dt)
{
	StateTitleScreen::instanceObj.update(s, dt);

	int w, h;
	glfwGetWindowSize(s.window, &w, &h);

	if (copying)
	{
		if (copyFinished)
		{
			copying = false;
			copyFinished = false;

			StateSingleplayer::instanceObj.worldListContainer.clear();
			StateSingleplayer::instanceObj.init(s);

			ui = &mainUI;

			s.popState();
		}
		else
		{
			std::lock_guard lock{ copyLogMutex };

			int fits = (h - 200) / 12;
			printf("%i\n", fits);
			while (copyLog.size() > fits)
				copyLog.erase(copyLog.begin());
			std::string fullLog = "";
			for (auto& str : copyLog)
			{
				fullLog += str;
			}
			copyingLogText.setText(fullLog);
		}
	}
}
void StateWorldSettings::close(StateManager& s)
{
	mainUI.clear();
	copyUI.clear();
	copyingUI.clear();
	deleteUI.clear();
}
void StateWorldSettings::windowResize(StateManager& s, int w, int h)
{
	updateProjection({ w, h });
}


