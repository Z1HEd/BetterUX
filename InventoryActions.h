#pragma once

#include <4dm.h>

using namespace fdm;

class InventoryActions {
public:
	using actionPredicate = std::function<bool(const std::unique_ptr<Item>&)>;
	static void combineItem(InventoryManager* manager, Inventory* inventory, int fromIndex);
	static void combineItemIf(InventoryManager* manager, Inventory* inventory, int fromIndex, actionPredicate predicate);
	static void cursorTransfer(InventoryManager* manager, Inventory* inventory, int index);
	static void swapIndex(InventoryManager* manager, Inventory* inventoryA, Inventory* inventoryB, int indexA, int indexB);
};