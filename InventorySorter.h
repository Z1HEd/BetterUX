#pragma once

#include <4dm.h>

using namespace fdm;

class InventorySorter {
	struct SortedItemInfo {
		std::string itemName;
		int currentStackCount = 0;
		int maxStackCount = 0;
	};
private:
	static std::vector<InventorySorter::SortedItemInfo> generateSortedInventoryMap(InventoryGrid* inventory);
	static int getItemCategory(Item* item);
	static bool compareNames(InventorySorter::SortedItemInfo& i, InventorySorter::SortedItemInfo& j);
public:
	static void sort(InventoryManager* manager, InventoryGrid* inventory, Inventory* other);
};
