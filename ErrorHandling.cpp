#include <4dm.h>
#include "StateWorldError.h"

using namespace fdm;

static StateManager* stateManager;

$hook(void, StateIntro, init, StateManager& s) {
	stateManager = &s;
	original(self,s);
}

void cancelButtonCallback(void* user) {
	stateManager->popState();
}

$hookStatic(std::unique_ptr<Item>, Item, createFromJson, nlohmann::json& j) {
	if (Item::blueprints.contains(j["name"])) 
		return original(j);

	StateWorldError::instanceObj.init(*stateManager);

	stateManager->changeState(&StateWorldError::instanceObj);
	
	std::string errorMsg = std::format("Failed to create item: {} (Missing a mod?)", j["name"].get<std::string>());
	StateWorldError::instanceObj.errorMessage.setText(errorMsg);
	
	return nullptr;
}