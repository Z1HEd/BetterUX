#include "InventorySorter.h"

void InventorySorter::swapIndex(InventoryManager* manager, Inventory* inventory, int a, int b) {
	manager->applyTransfer(InventoryManager::ACTION_SWAP, *inventory->getSlot(a), manager->cursor.item, inventory);
	manager->applyTransfer(InventoryManager::ACTION_SWAP, *inventory->getSlot(b), manager->cursor.item, inventory);
	manager->applyTransfer(InventoryManager::ACTION_SWAP, *inventory->getSlot(a), manager->cursor.item, inventory);
}

void InventorySorter::combineItem(InventoryManager* manager, Inventory* inventory, int fromIndex) {
	manager->applyTransfer(InventoryManager::ACTION_SWAP, *inventory->getSlot(fromIndex), manager->cursor.item, manager->secondary);
	manager->secondary->combineItem(manager->cursor.item);
	if (!(manager->cursor.item))
		return;
	manager->applyTransfer(InventoryManager::ACTION_SWAP, *inventory->getSlot(fromIndex), manager->cursor.item, manager->secondary);
}

int InventorySorter::getItemCategory(Item* item) { //Leetcode ahh algorythm
    if (item == nullptr) return 2;

    //I will define the reality myself
    if (item->getName() == "Stick") return 1;
    if (item->getName() == "Compass" || item->getName() == "4D Glasses") return 0;
    if (item->getName() == "Alidade") return 2;

    if (0 == strcmp(typeid(*item).name(), "class ItemTool")) return 0;
    if (0 == strcmp(typeid(*item).name(), "class ItemMaterial")) return 1;
    if (0 == strcmp(typeid(*item).name(), "class ItemBlock")) return 2;

    return 0; Console::printLine("UNKNOWN CATEGORY"); // should not be the case
}

int InventorySorter::getMaxCount(Item* item) {
    return item->getStackLimit();
}

bool InventorySorter::compareNames(InventorySorter::SortedItemInfo& i, InventorySorter::SortedItemInfo& j) { return std::strcmp(i.itemName.c_str(), j.itemName.c_str()) == 1; }

std::vector<InventorySorter::SortedItemInfo> InventorySorter::generateSortedInventoryMap(InventoryGrid* inventory) {
    static int transferCount = 0;
    
    int columns = 4;
    int rows = 8;
    int totalSlots = rows*columns; // Hardcoded because InventoryGrid::size contains bullshit values


    std::vector<SortedItemInfo> inventoryMap(totalSlots);

    std::vector<SortedItemInfo> tools, materials, blocks;
    std::vector<std::vector<SortedItemInfo>> columnsInfo(columns);


    // Extract items into separate categories
    for (int i = 0; i < totalSlots; ++i) {
        Item* item = inventory->getSlot(i)->get();
        if (!item) continue;

        SortedItemInfo info;
        info.itemName = item->getName();
        info.currentStackCount = item->count;
        info.maxStackCount = getMaxCount(item);

        //Push each item into respective category, combining stacks
        int category = getItemCategory(item);
        if (category == 0) {
            for (auto& placedInfo : tools) {
                if (placedInfo.itemName != info.itemName) continue;
                transferCount = std::min(placedInfo.maxStackCount - placedInfo.currentStackCount, info.currentStackCount);
                if (transferCount > 0) {
                    placedInfo.currentStackCount += transferCount;
                    info.currentStackCount -= transferCount;
                    if (info.currentStackCount < 1) break;
                }
            }
            if (info.currentStackCount > 0)
                tools.push_back(info);
        }
        else if (category == 1) {
            for (auto& placedInfo : materials) {
                if (placedInfo.itemName != info.itemName) continue;
                transferCount = std::min(placedInfo.maxStackCount - placedInfo.currentStackCount, info.currentStackCount);
                if (transferCount > 0) {
                    placedInfo.currentStackCount += transferCount;
                    info.currentStackCount -= transferCount;
                    if (info.currentStackCount < 1) break;
                }
            }
            if (info.currentStackCount > 0)
                materials.push_back(info);
        }
        else {
            for (auto& placedInfo : blocks) {
                if (placedInfo.itemName != info.itemName) continue;
                transferCount = std::min(placedInfo.maxStackCount - placedInfo.currentStackCount, info.currentStackCount);
                if (transferCount > 0) {
                    placedInfo.currentStackCount += transferCount;
                    info.currentStackCount -= transferCount;
                    if (info.currentStackCount < 1) break;
                }
            }
            if (info.currentStackCount > 0)
                blocks.push_back(info);
        }
    }

    // Sort every category alphabetically
    std::sort(tools.begin(), tools.end(), compareNames);
    std::sort(materials.begin(), materials.end(), compareNames);
    std::sort(blocks.begin(), blocks.end(), compareNames);

    // Arrange categories into columns
    int j = 0;
    for (int i = 0;i < tools.size();i++) {
        while (columnsInfo[j].size() >= rows) 
            j++;
        columnsInfo[j].push_back(tools[i]);
    }
    j = columns-1;
    for (int i = 0;i < blocks.size();i++) {
        while (columnsInfo[j].size() >= rows) 
            j--;
        columnsInfo[j].push_back(blocks[i]);
    }
    for (int i = 0;i < materials.size();i++) {
        while (columnsInfo[j].size() >= rows) 
            j--;
        columnsInfo[j].push_back(materials[i]);
    }

    // Copy columns into return map
    for (int i=0;i<columns;i++){
        std::copy(columnsInfo[i].begin(), columnsInfo[i].end(), inventoryMap.begin() + rows * i);
    }
    return inventoryMap;
}


void InventorySorter::Sort(InventoryManager* manager, InventoryGrid* inventory) {
    int columns = 4;
    int rows = 8;
    int totalSlots = rows * columns; // Hardcoded because InventoryGrid::size contains bullshit values

    auto sortedInventoryMap = generateSortedInventoryMap(inventory);

    for (int i = 0; i < totalSlots; ++i) { // For each item in the inventory
        Item* item = inventory->getSlot(i)->get();

        while (item != nullptr && item->getName() != sortedInventoryMap[i].itemName) {
            InventorySorter::combineItem(manager, inventory, i); //Try to combine with the others
            if (item == nullptr || item->count<1) break; // If combined with no remainder, continue

            int j = 0; // Find first index of that item in supposed arrangement
            for (;j < sortedInventoryMap.size();j++) {
                Item* itemI = inventory->getSlot(j)->get();
                if (itemI != nullptr && sortedInventoryMap[j].itemName == itemI->getName()) continue;
                if (sortedInventoryMap[j].itemName == item->getName()) break;
            }
            InventorySorter::swapIndex(manager, inventory,i,j); // Swap it with whatever was on that index
            item = inventory->getSlot(i)->get(); // if something was on that index, will have to sort it in its place before proceeding
        }
    }
}