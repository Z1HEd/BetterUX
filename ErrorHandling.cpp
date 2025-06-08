#include <4dm.h>

using namespace fdm;

static StateManager* stateManager;

$hook(void, StateIntro, init, StateManager& s) {
	stateManager = &s;
	original(self,s);
}

void cancelButtonCallback(void* user) {
	stateManager->popState();
	StateMultiplayer::instanceObj.connectionErrorOkButton.callback = StateMultiplayer::connectionErrorOkButtonCallback;
}

$hookStatic(std::unique_ptr<Item>, Item, createFromJson, nlohmann::json& j) {
	if (Item::blueprints.contains(j["name"])) 
		return original(j);

	stateManager->changeState(&StateMultiplayer::instanceObj);
	std::string errorMsg = std::format("Failed to create item: {} (Missing a mod?)", j["name"].get<std::string>());
	StateMultiplayer::instanceObj.connectionStatusText.setText(errorMsg);
	StateMultiplayer::instanceObj.connectionStatusText.color = {1,0,0,1};
	StateMultiplayer::instanceObj.currentUI = &StateMultiplayer::instanceObj.ui2;
	StateMultiplayer::instanceObj.currentUI->addElement(&StateMultiplayer::instanceObj.connectionStatusText);
	StateMultiplayer::instanceObj.currentUI->addElement(&StateMultiplayer::instanceObj.connectionErrorOkButton);
	StateMultiplayer::instanceObj.connectionErrorOkButton.callback = cancelButtonCallback;
	
	return nullptr;
}