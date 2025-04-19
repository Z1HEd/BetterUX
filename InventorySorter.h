#pragma once

#include <4dm.h>

using namespace fdm;

class InventorySorter {
private:
	static void synchronizedCursorTransfer(InventoryManager* manager, Inventory* inventory, int a);
	static void swapIndex(InventoryManager* manager, Inventory* inventory, int a,int b);
	static void combineItem(InventoryManager* manager, Inventory* inventory, int fromIndex);

	struct SortedItemInfo {
		std::string itemName;
		int currentStackCount = 0;
		int maxStackCount = 0;
	};

	static std::vector<InventorySorter::SortedItemInfo> generateSortedInventoryMap(InventoryGrid* inventory);
	static int getItemCategory(Item* item);
	static bool compareNames(InventorySorter::SortedItemInfo& i, InventorySorter::SortedItemInfo& j);

public:
	static void sort(InventoryManager* manager, InventoryGrid* inventory);
};