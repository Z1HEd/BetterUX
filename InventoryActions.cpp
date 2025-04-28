#include "InventoryActions.h"


void InventoryActions::cursorTransfer(InventoryManager* manager, Inventory* inventory, int index, Inventory* other) {
    const nlohmann::json action = {
            {"action","transfer"},
            {"cursorContents",manager->cursor.item != nullptr ? manager->cursor.item->save().dump() : ""},
            {"inventory",inventory->name},
            {"other",other->name},
            {"slotContents", inventory->getSlot(index) != nullptr ? inventory->getSlot(index)->save().dump() : ""},
            {"slotIndex",index},
            {"transferAction",InventoryManager::ACTION_SWAP}
    };
    if (manager->callback)
        manager->callback(action, manager->user);

    manager->applyTransfer(InventoryManager::ACTION_SWAP, inventory->getSlot(index), manager->cursor.item, other);;
}

void InventoryActions::swapIndex(InventoryManager* manager, Inventory* inventoryA, Inventory* inventoryB, int indexA, int indexB, Inventory* other) {
    cursorTransfer(manager, inventoryA, indexA, other);
    cursorTransfer(manager, inventoryB, indexB, other);
    cursorTransfer(manager, inventoryA, indexA, other);
}

void InventoryActions::combineItem(InventoryManager* manager, Inventory* inventory, int fromIndex, Inventory* other) {
    cursorTransfer(manager, inventory, fromIndex, other);

    for (int i = 0;i < (int)inventory->getSlotCount();i++) {
        if (inventory->getSlot(i) == nullptr || inventory->getSlot(i)->getName() != manager->cursor.item->getName() || inventory->getSlot(i)->count >= inventory->getSlot(i)->getStackLimit()) continue;
        if (manager->callback)
            manager->callback({
                {"action","transfer"},
                {"cursorContents",manager->cursor.item != nullptr ? manager->cursor.item->save().dump() : ""},
                {"inventory",inventory->name},
                {"other",other->name},
                {"slotContents", inventory->getSlot(i) != nullptr ? inventory->getSlot(i)->save().dump() : ""},
                {"slotIndex",i},
                {"transferAction",InventoryManager::ACTION_GIVE_MAX}
                },
                manager->user);
        manager->applyTransfer(InventoryManager::ACTION_GIVE_MAX, inventory->getSlot(i), manager->cursor.item, manager->secondary);
        if (!manager->cursor.item || manager->cursor.item->count < 1) return;
    }
    cursorTransfer(manager, inventory, fromIndex, other);
}

// only combines with those that are wrong
void InventoryActions::combineItemIf(InventoryManager* manager, Inventory* inventory, int fromIndex, actionPredicate predicate, Inventory* other) {
    cursorTransfer(manager, inventory, fromIndex,other);

    for (int i = 0;i < (int)inventory->getSlotCount();i++) {
        if (inventory->getSlot(i) == nullptr ||
            (manager->cursor.item && inventory->getSlot(i)->getName() != manager->cursor.item->getName()) ||
            inventory->getSlot(i)->count >= inventory->getSlot(i)->getStackLimit() ||
            !predicate(inventory->getSlot(i))
            ) continue;
        if (manager->callback)
            manager->callback({
                {"action","transfer"},
                {"cursorContents",manager->cursor.item != nullptr ? manager->cursor.item->save().dump() : ""},
                {"inventory",inventory->name},
                {"other",other->name},
                {"slotContents", inventory->getSlot(i) != nullptr ? inventory->getSlot(i)->save().dump() : ""},
                {"slotIndex",i},
                {"transferAction",InventoryManager::ACTION_GIVE_MAX}
                },
                manager->user);
        manager->applyTransfer(InventoryManager::ACTION_GIVE_MAX, inventory->getSlot(i), manager->cursor.item, manager->secondary);
        if (!manager->cursor.item || manager->cursor.item->count < 1) return;
    }
    cursorTransfer(manager, inventory, fromIndex, other);
}