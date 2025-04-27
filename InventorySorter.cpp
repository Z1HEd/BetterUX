#include "InventorySorter.h"

void InventorySorter::synchronizedCursorTransfer(InventoryManager* manager, Inventory* inventory, int a) {
    const nlohmann::json action = {
            {"action","transfer"},
            {"cursorContents",manager->cursor.item != nullptr ? manager->cursor.item->save().dump() : ""},
            {"inventory",inventory->name},
            {"other",manager->primary!=nullptr? manager->primary->name: inventory->name},
            {"slotContents", inventory->getSlot(a) != nullptr ? inventory->getSlot(a)->save().dump() : ""},
            {"slotIndex",a},
            {"transferAction",InventoryManager::ACTION_SWAP}
    };
    if (manager->callback) 
        manager->callback(action, manager->user);

    manager->applyAction(StateGame::instanceObj.world.get(), &StateGame::instanceObj.player,action);

}

void InventorySorter::swapIndex(InventoryManager* manager, Inventory* inventory, int a, int b) {
    synchronizedCursorTransfer(manager, inventory, a);
    synchronizedCursorTransfer(manager, inventory, b);
    synchronizedCursorTransfer(manager, inventory, a);
}

void InventorySorter::combineItem(InventoryManager* manager, Inventory* inventory, int fromIndex) {
    synchronizedCursorTransfer(manager, inventory, fromIndex);

    for (int i = 0;i < inventory->getSlotCount();i++) {
        if (inventory->getSlot(i) == nullptr || inventory->getSlot(i)->getName() != manager->cursor.item->getName() || inventory->getSlot(i)->count>= inventory->getSlot(i)->getStackLimit()) continue;
        if (manager->callback)
            manager->callback({
                {"action","transfer"},
                {"cursorContents",manager->cursor.item != nullptr ? manager->cursor.item->save().dump() : ""},
                {"inventory",inventory->name},
                {"other",manager->primary != nullptr ? manager->primary->name : inventory->name},
                {"slotContents", inventory->getSlot(i) != nullptr ? inventory->getSlot(i)->save().dump() : ""},
                {"slotIndex",i},
                {"transferAction",InventoryManager::ACTION_GIVE_MAX}
                },
                manager->user);
        manager->applyTransfer(InventoryManager::ACTION_GIVE_MAX, inventory->getSlot(i), manager->cursor.item, manager->secondary);
        if (!manager->cursor.item || manager->cursor.item->count < 1) return;
    }
    synchronizedCursorTransfer(manager, inventory, fromIndex);
}

// only combines with those that are wrong
void InventorySorter::combineItemInOrder(InventoryManager* manager, Inventory* inventory, int fromIndex,std::vector<InventorySorter::SortedItemInfo> order) {
    synchronizedCursorTransfer(manager, inventory, fromIndex);

    for (int i = 0;i < inventory->getSlotCount();i++) {
        if (inventory->getSlot(i) == nullptr ||
            inventory->getSlot(i)->getName() != manager->cursor.item->getName() || 
            inventory->getSlot(i)->count >= inventory->getSlot(i)->getStackLimit() ||
            inventory->getSlot(i)->count == order[i].currentStackCount
            ) continue;
        if (manager->callback)
            manager->callback({
                {"action","transfer"},
                {"cursorContents",manager->cursor.item != nullptr ? manager->cursor.item->save().dump() : ""},
                {"inventory",manager->secondary->name},
                {"other",manager->primary->name},
                {"slotContents", inventory->getSlot(i) != nullptr ? inventory->getSlot(i)->save().dump() : ""},
                {"slotIndex",i},
                {"transferAction",InventoryManager::ACTION_GIVE_MAX}
                },
                manager->user);
        manager->applyTransfer(InventoryManager::ACTION_GIVE_MAX, inventory->getSlot(i), manager->cursor.item, manager->secondary);
        if (!manager->cursor.item || manager->cursor.item->count < 1) return;
    }
    synchronizedCursorTransfer(manager, inventory, fromIndex);
}

int InventorySorter::getItemCategory(Item* item) { 
    if (item == nullptr) return 2;

    //I will define the reality myself
    if (item->getName() == "Stick") return 1;
    if (item->getName() == "Compass" || item->getName() == "4D Glasses") return 0;
    if (item->getName() == "Alidade") return 2;

    if (0 == strcmp(typeid(*item).name(), "class ItemTool")) return 0;
    if (0 == strcmp(typeid(*item).name(), "class ItemMaterial")) return 1;
    if (0 == strcmp(typeid(*item).name(), "class ItemBlock")) return 2;

    return 0; // should not be the case
}

bool InventorySorter::compareNames(InventorySorter::SortedItemInfo& i, InventorySorter::SortedItemInfo& j) { return std::strcmp(i.itemName.c_str(), j.itemName.c_str()) == 1; }

std::vector<InventorySorter::SortedItemInfo> InventorySorter::generateSortedInventoryMap(InventoryGrid* inventory) { //Leetcode ahh algorythm
    static int transferCount = 0;
    
    int columns = inventory->size.x;
    int rows = inventory->size.y;
    int totalSlots = inventory->getSlotCount();


    std::vector<SortedItemInfo> inventoryMap(totalSlots);

    std::vector<SortedItemInfo> tools, materials, blocks;
    std::vector<std::vector<SortedItemInfo>> columnsInfo(columns);


    // Extract items into separate categories
    for (int i = 0; i < totalSlots; ++i) {
        Item* item = inventory->getSlot(i).get();
        if (!item) continue;

        SortedItemInfo info;
        info.itemName = item->getName();
        info.currentStackCount = item->count;
        info.maxStackCount = item->getStackLimit();

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


void InventorySorter::sort(InventoryManager* manager, InventoryGrid* inventory) {
    int columns = inventory->size.x;
    int rows = inventory->size.y;
    int totalSlots = inventory->getSlotCount();

    auto sortedInventoryMap = generateSortedInventoryMap(inventory);

    for (int i = 0; i < totalSlots; ++i) { // For each item in the inventory
        auto* item = &inventory->getSlot(i);
        
        while ((*item) && ((*item)->getName() != sortedInventoryMap[i].itemName || (*item)->count> sortedInventoryMap[i].currentStackCount)) {
            InventorySorter::combineItemInOrder(manager, inventory, i, sortedInventoryMap); //Try to combine with the others
            if (!(*item)  || (*item)->count<1 || 
                (*item)->getName() == sortedInventoryMap[i].itemName) break; // If combined with no remainder, continue

            int j = 0; // Find first index of that item in supposed arrangement
            for (;j < sortedInventoryMap.size();j++) {
                auto& itemI = inventory->getSlot(j); 
                if (itemI != nullptr && sortedInventoryMap[j].itemName == itemI->getName()) continue;
                if (sortedInventoryMap[j].itemName == (*item)->getName()) break;
            }
            if (j == sortedInventoryMap.size()) {
                Console::printLine("Something went wrong in the sorting function!");
                return;
            }
            InventorySorter::swapIndex(manager, inventory,i,j); // Swap it with whatever was on that index
            item = &inventory->getSlot(i); // if something was on that index, will have to sort it in its place before proceeding
        }
    }
}