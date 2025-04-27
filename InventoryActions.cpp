#include "InventoryActions.h"


void InventoryActions::cursorTransfer(InventoryManager* manager, Inventory* inventory, int index) {
    const nlohmann::json action = {
            {"action","transfer"},
            {"cursorContents",manager->cursor.item != nullptr ? manager->cursor.item->save().dump() : ""},
            {"inventory",inventory->name},
            {"other",manager->primary != nullptr ? manager->primary->name : inventory->name},
            {"slotContents", inventory->getSlot(index) != nullptr ? inventory->getSlot(index)->save().dump() : ""},
            {"slotIndex",index},
            {"transferAction",InventoryManager::ACTION_SWAP}
    };
    if (manager->callback)
        manager->callback(action, manager->user);

    manager->applyAction(StateGame::instanceObj.world.get(), &StateGame::instanceObj.player, action);
}

void InventoryActions::swapIndex(InventoryManager* manager, Inventory* inventoryA, Inventory* inventoryB, int indexA, int indexB) {
    cursorTransfer(manager, inventoryA, indexA);
    cursorTransfer(manager, inventoryB, indexB);
    cursorTransfer(manager, inventoryA, indexA);
}

void InventoryActions::combineItem(InventoryManager* manager, Inventory* inventory, int fromIndex) {
    cursorTransfer(manager, inventory, fromIndex);

    for (int i = 0;i < inventory->getSlotCount();i++) {
        if (inventory->getSlot(i) == nullptr || inventory->getSlot(i)->getName() != manager->cursor.item->getName() || inventory->getSlot(i)->count >= inventory->getSlot(i)->getStackLimit()) continue;
        if (manager->callback)
            manager->callback({
                {"action","transfer"},
                {"cursorContents",manager->cursor.item != nullptr ? manager->cursor.item->save().dump() : ""},
                {"inventory",inventory->name},
                {"other",inventory->name},
                {"slotContents", inventory->getSlot(i) != nullptr ? inventory->getSlot(i)->save().dump() : ""},
                {"slotIndex",i},
                {"transferAction",InventoryManager::ACTION_GIVE_MAX}
                },
                manager->user);
        manager->applyTransfer(InventoryManager::ACTION_GIVE_MAX, inventory->getSlot(i), manager->cursor.item, manager->secondary);
        if (!manager->cursor.item || manager->cursor.item->count < 1) return;
    }
    cursorTransfer(manager, inventory, fromIndex);
}

// only combines with those that are wrong
void InventoryActions::combineItemIf(InventoryManager* manager, Inventory* inventory, int fromIndex, actionPredicate predicate) {
    cursorTransfer(manager, inventory, fromIndex);

    for (int i = 0;i < inventory->getSlotCount();i++) {
        if (inventory->getSlot(i) == nullptr ||
            inventory->getSlot(i)->getName() != manager->cursor.item->getName() ||
            inventory->getSlot(i)->count >= inventory->getSlot(i)->getStackLimit() ||
            !predicate(inventory->getSlot(i))
            ) continue;
        if (manager->callback)
            manager->callback({
                {"action","transfer"},
                {"cursorContents",manager->cursor.item != nullptr ? manager->cursor.item->save().dump() : ""},
                {"inventory",inventory->name},
                {"other",inventory->name},
                {"slotContents", inventory->getSlot(i) != nullptr ? inventory->getSlot(i)->save().dump() : ""},
                {"slotIndex",i},
                {"transferAction",InventoryManager::ACTION_GIVE_MAX}
                },
                manager->user);
        manager->applyTransfer(InventoryManager::ACTION_GIVE_MAX, inventory->getSlot(i), manager->cursor.item, manager->secondary);
        if (!manager->cursor.item || manager->cursor.item->count < 1) return;
    }
    cursorTransfer(manager, inventory, fromIndex);
}