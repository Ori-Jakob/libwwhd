#ifndef LIBWWHD_D_SAVE_ITEM_ACCESS_H
#define LIBWWHD_D_SAVE_ITEM_ACCESS_H

#include "libwwhd/wwhd_types.h"
#include "libwwhd/d/d_item.h"
#include "libwwhd/d/d_save.h"
#include "libwwhd/d/d_save_access.h"

/**
 * libwwhd - inventory, equipment and bag writes through the game info root.
 *
 * Every helper here reproduces what the item's give stub does (d_item.h), on
 * the same fields, so an edit leaves the save in a state the game itself could
 * have produced. None of them calls the game: these are plain writes to [V]
 * fields, safe from any thread that may touch the save, and NULL-safe before a
 * region is selected.
 *
 * Only what a stub was seen to write is written. In particular nothing here
 * touches the pending item deltas in play except to clear them, which is what
 * keeps an in-flight pickup from landing on top of an edit.
 */

/* --- inventory slots ---------------------------------------------------- */

/** [V] The item flag byte for a slot, bit `tier`. */
static __inline int dSv_isItemFlag(int slot, int tier) {
    dSv_player_c* p = dComIfGs_getPlayerSave();
    if (!p || slot < 0 || slot >= WWHD_ITEM_SLOTS || tier < 0 || tier > 7)
        return 0;
    return (p->mGetItem.mItemFlags[slot] >> tier) & 1;
}

/** [V] Set or clear one tier bit. Mirrors dSv_player_get_item_c::onItem. */
static __inline void dSv_setItemFlag(int slot, int tier, int on) {
    dSv_player_c* p = dComIfGs_getPlayerSave();
    if (!p || slot < 0 || slot >= WWHD_ITEM_SLOTS || tier < 0 || tier > 7)
        return;
    if (on)
        p->mGetItem.mItemFlags[slot] |= (u8)(1u << tier);
    else
        p->mGetItem.mItemFlags[slot] &= (u8)~(1u << tier);
}

/** The slot table entry for an item number, or NULL. */
static __inline const wwhd_slotItem_t* dSv_findSlotItem(u8 id) {
    int i;
    for (i = 0; i < WWHD_SLOT_ITEM_COUNT; i++)
        if (wwhd_slotItems[i].id == id)
            return &wwhd_slotItems[i];
    return (const wwhd_slotItem_t*)0;
}

/** The slot table entry currently in `slot`, or NULL when it is empty or
 *  holds something the table does not know. */
static __inline const wwhd_slotItem_t* dSv_getSlotItem(int slot) {
    return dSv_findSlotItem(dSv_getItem(slot));
}

/**
 * [V] Empty a slot: 0xFF in the slot and every tier bit the table knows for
 * that slot cleared. Bits the table does not name are left alone - byte 0
 * also carries the bottle-contents flags (0x025B5DBC), and a bottle slot has
 * no tier bits of its own.
 */
static __inline void dSv_clearSlot(int slot) {
    int i;
    if (!dSv_setItem(slot, (u8)WWHD_ITEM_NONE))
        return;
    for (i = 0; i < WWHD_SLOT_ITEM_COUNT; i++)
        if (wwhd_slotItems[i].slot == slot)
            dSv_setItemFlag(slot, wwhd_slotItems[i].tier, 0);
}

/**
 * [V] Put an inventory item in its slot the way its give stub does: the id in
 * the slot, its tier bit set, and every lower tier's bit set too, since the
 * game only ever hands the tiers out in order. The bow and bomb stubs also
 * open a 30-capacity quiver or bag; that is reproduced when the capacity is
 * still zero, and left alone when Link already has a bigger one.
 */
static __inline void dSv_setSlotItem(const wwhd_slotItem_t* item) {
    dSv_player_c* p = dComIfGs_getPlayerSave();
    int i;
    if (!p || !item)
        return;
    dSv_clearSlot(item->slot);
    dSv_setItem(item->slot, item->id);
    for (i = 0; i < WWHD_SLOT_ITEM_COUNT; i++)
        if (wwhd_slotItems[i].slot == item->slot && wwhd_slotItems[i].tier <= item->tier)
            dSv_setItemFlag(item->slot, wwhd_slotItems[i].tier, 1);
    if (item->slot == dSv_SLOT_BOW && p->mItemMax.mArrowNum == 0) {
        p->mItemMax.mArrowNum = WWHD_AMMO_CAP_30;
        p->mItemRecord.mArrowNum = WWHD_AMMO_CAP_30;
    }
    if (item->slot == dSv_SLOT_BOMBS && p->mItemMax.mBombNum == 0) {
        p->mItemMax.mBombNum = WWHD_AMMO_CAP_30;
        p->mItemRecord.mBombNum = WWHD_AMMO_CAP_30;
    }
}

/** [V] What bottle 0..3 holds: a content id, or 0xFF for no bottle. */
static __inline u8 dSv_getBottle(int bottle) {
    if (bottle < 0 || bottle >= WWHD_BOTTLE_COUNT)
        return (u8)WWHD_ITEM_NONE;
    return dSv_getItem(dSv_SLOT_BOTTLE_1 + bottle);
}

/** [V] Set what bottle 0..3 holds. 0xFF removes the bottle. */
static __inline void dSv_setBottle(int bottle, u8 content) {
    if (bottle < 0 || bottle >= WWHD_BOTTLE_COUNT)
        return;
    dSv_setItem(dSv_SLOT_BOTTLE_1 + bottle, content);
}

/* --- equipment and the collect bank ------------------------------------- */

/** [V] Test bit `bit` of mCollect[idx]. */
static __inline int dSv_isCollectBit(int idx, int bit) {
    return dSv_collect_isCollect(dComIfGs_getCollect(), idx, bit);
}

/** [V] Set or clear bit `bit` of mCollect[idx]. Mirrors onCollect. */
static __inline void dSv_setCollectBit(int idx, int bit, int on) {
    dSv_player_collect_c* c = dComIfGs_getCollect();
    if (!c || idx < 0 || idx >= 8 || bit < 0 || bit > 7)
        return;
    if (on)
        c->mCollect[idx] |= (u8)(1u << bit);
    else
        c->mCollect[idx] &= (u8)~(1u << bit);
}

/** [V] The id recorded for an equip category, or 0xFF. */
static __inline u8 dSv_getEquip(int category) {
    dSv_player_c* p = dComIfGs_getPlayerSave();
    if (!p || category < 0 || category > 3)
        return (u8)WWHD_ITEM_NONE;
    return p->mPlayerStatusA.mSelectEquip[category];
}

/**
 * [V] Equip a sword by id, or 0xFF for none. Sets collect[0] bits for that
 * sword and every earlier one, the state the give stubs leave behind.
 */
static __inline void dSv_setSword(u8 id) {
    dSv_player_c* p = dComIfGs_getPlayerSave();
    int level = -1, i;
    if (!p)
        return;
    for (i = 0; i < 4; i++)
        if (wwhd_swords[i].id == id)
            level = i;
    p->mPlayerStatusA.mSelectEquip[WWHD_EQUIP_SWORD] = level >= 0 ? id : (u8)WWHD_ITEM_NONE;
    for (i = 0; i < 4; i++)
        dSv_setCollectBit(WWHD_COLLECT_SWORD, wwhd_swords[i].bit, i <= level);
}

/** [V] Equip a shield by id, or 0xFF for none. Same bit rule as swords. */
static __inline void dSv_setShield(u8 id) {
    dSv_player_c* p = dComIfGs_getPlayerSave();
    int level = -1, i;
    if (!p)
        return;
    for (i = 0; i < 2; i++)
        if (wwhd_shields[i].id == id)
            level = i;
    p->mPlayerStatusA.mSelectEquip[WWHD_EQUIP_SHIELD] = level >= 0 ? id : (u8)WWHD_ITEM_NONE;
    for (i = 0; i < 2; i++)
        dSv_setCollectBit(WWHD_COLLECT_SHIELD, wwhd_shields[i].bit, i <= level);
}

/** [V] Non-zero when the Power Bracelets are held. */
static __inline int dSv_hasPowerBracelets(void) {
    return dSv_isCollectBit(WWHD_COLLECT_BRACELET, 0);
}

/** [V] Grant or remove the Power Bracelets: collect[2] bit 0 and equip[2]. */
static __inline void dSv_setPowerBracelets(int on) {
    dSv_player_c* p = dComIfGs_getPlayerSave();
    if (!p)
        return;
    dSv_setCollectBit(WWHD_COLLECT_BRACELET, 0, on);
    p->mPlayerStatusA.mSelectEquip[WWHD_EQUIP_BRACELETS] =
        on ? (u8)dItemNo_POWER_BRACELETS : (u8)WWHD_ITEM_NONE;
}

/** [V] Songs: mTact bit. */
static __inline int dSv_hasSong(int bit) {
    dSv_player_collect_c* c = dComIfGs_getCollect();
    return c && bit >= 0 && bit < 8 && ((c->mTact >> bit) & 1);
}
static __inline void dSv_setSong(int bit, int on) {
    dSv_player_collect_c* c = dComIfGs_getCollect();
    if (!c || bit < 0 || bit >= 8)
        return;
    if (on) c->mTact |= (u8)(1u << bit); else c->mTact &= (u8)~(1u << bit);
}

/** [V] Pearls: mSymbol bit. */
static __inline int dSv_hasPearl(int bit) {
    dSv_player_collect_c* c = dComIfGs_getCollect();
    return c && bit >= 0 && bit < 8 && ((c->mSymbol >> bit) & 1);
}
static __inline void dSv_setPearl(int bit, int on) {
    dSv_player_collect_c* c = dComIfGs_getCollect();
    if (!c || bit < 0 || bit >= 8)
        return;
    if (on) c->mSymbol |= (u8)(1u << bit); else c->mSymbol &= (u8)~(1u << bit);
}

/** [V] Triforce shards 0..7: mTriforce bit. */
static __inline int dSv_hasShard(int bit) {
    return dSv_collect_isTriforce(dComIfGs_getCollect(), bit);
}
static __inline void dSv_setShard(int bit, int on) {
    dSv_player_collect_c* c = dComIfGs_getCollect();
    if (!c || bit < 0 || bit >= 8)
        return;
    if (on) c->mTriforce |= (u8)(1u << bit); else c->mTriforce &= (u8)~(1u << bit);
}

/* --- bags ----------------------------------------------------------------
 * Each bag is an 8-byte list of ids in the order they were obtained, filled
 * from the front with 0xFF behind, plus a flag bit per item and (spoils, bait)
 * a count per index. The give stubs append; removal here closes the gap so
 * the list stays the shape the stubs produce.
 * ---------------------------------------------------------------------- */

static __inline int wwhd_bagFind(const u8* slots, u8 id) {
    int i;
    for (i = 0; i < 8; i++)
        if (slots[i] == id)
            return i;
    return -1;
}

static __inline void wwhd_bagAdd(u8* slots, u8 id) {
    int i;
    if (wwhd_bagFind(slots, id) >= 0)
        return;
    for (i = 0; i < 8; i++) {
        if (slots[i] == (u8)WWHD_ITEM_NONE) {
            slots[i] = id;
            return;
        }
    }
}

static __inline void wwhd_bagRemove(u8* slots, u8 id) {
    int at = wwhd_bagFind(slots, id), i;
    if (at < 0)
        return;
    for (i = at; i + 1 < 8; i++)
        slots[i] = slots[i + 1];
    slots[7] = (u8)WWHD_ITEM_NONE;
}

/** [V] Spoils bag, by index 0..7. */
static __inline int dSv_hasSpoil(int index) {
    dSv_player_c* p = dComIfGs_getPlayerSave();
    return p && index >= 0 && index < 8 && ((p->mGetBagItem.mBeastFlags >> index) & 1);
}
static __inline u8 dSv_getSpoilNum(int index) {
    dSv_player_c* p = dComIfGs_getPlayerSave();
    return (p && index >= 0 && index < 8) ? p->mBagItemRecord.mBeastNum[index] : (u8)0;
}
static __inline void dSv_setSpoilNum(int index, u8 count) {
    dSv_player_c* p = dComIfGs_getPlayerSave();
    if (p && index >= 0 && index < 8)
        p->mBagItemRecord.mBeastNum[index] = count;
}
static __inline void dSv_setSpoil(int index, int on) {
    dSv_player_c* p = dComIfGs_getPlayerSave();
    if (!p || index < 0 || index >= 8)
        return;
    if (on) {
        p->mGetBagItem.mBeastFlags |= (u8)(1u << index);
        wwhd_bagAdd(p->mBagItem.mBeast, wwhd_spoils[index].id);
    } else {
        p->mGetBagItem.mBeastFlags &= (u8)~(1u << index);
        wwhd_bagRemove(p->mBagItem.mBeast, wwhd_spoils[index].id);
        p->mBagItemRecord.mBeastNum[index] = 0;
    }
}

/** [V] Bait bag, by index 0..1. A newly given bait starts with 3, as the
 *  setter at 0x025B6E98 leaves it. */
static __inline int dSv_hasBait(int index) {
    dSv_player_c* p = dComIfGs_getPlayerSave();
    return p && index >= 0 && index < 8 && ((p->mGetBagItem.mBaitFlags >> index) & 1);
}
static __inline u8 dSv_getBaitNum(int index) {
    dSv_player_c* p = dComIfGs_getPlayerSave();
    return (p && index >= 0 && index < 8) ? p->mBagItemRecord.mBaitNum[index] : (u8)0;
}
static __inline void dSv_setBaitNum(int index, u8 count) {
    dSv_player_c* p = dComIfGs_getPlayerSave();
    if (p && index >= 0 && index < 8)
        p->mBagItemRecord.mBaitNum[index] = count;
}
static __inline void dSv_setBait(int index, int on) {
    dSv_player_c* p = dComIfGs_getPlayerSave();
    if (!p || index < 0 || index >= 2)
        return;
    if (on) {
        p->mGetBagItem.mBaitFlags |= (u8)(1u << index);
        wwhd_bagAdd(p->mBagItem.mBait, wwhd_baits[index].id);
        if (p->mBagItemRecord.mBaitNum[index] == 0)
            p->mBagItemRecord.mBaitNum[index] = 3;
    } else {
        p->mGetBagItem.mBaitFlags &= (u8)~(1u << index);
        wwhd_bagRemove(p->mBagItem.mBait, wwhd_baits[index].id);
        p->mBagItemRecord.mBaitNum[index] = 0;
    }
}

/** [V] Delivery bag, by the table entry. Flags are a 32-bit word indexed by
 *  the entry's index; the list holds at most 8 at once. */
static __inline int dSv_hasDeliveryItem(const wwhd_bagItem_t* item) {
    dSv_player_c* p = dComIfGs_getPlayerSave();
    return p && item && ((p->mGetBagItem.mReserveFlags >> item->index) & 1u);
}
static __inline void dSv_setDeliveryItem(const wwhd_bagItem_t* item, int on) {
    dSv_player_c* p = dComIfGs_getPlayerSave();
    if (!p || !item || item->index >= 32)
        return;
    if (on) {
        p->mGetBagItem.mReserveFlags |= (u32)1 << item->index;
        wwhd_bagAdd(p->mBagItem.mReserve, item->id);
    } else {
        p->mGetBagItem.mReserveFlags &= ~((u32)1 << item->index);
        wwhd_bagRemove(p->mBagItem.mReserve, item->id);
    }
}

/* --- capacities ----------------------------------------------------------- */

/** [V] Magic capacity. Current magic is brought down with it. The meter clamps
 *  mMaxMagic to dSv_MAGIC_MAX on its own, so nothing larger is accepted. */
static __inline void dSv_setMaxMagic(u8 cap) {
    dSv_player_c* p = dComIfGs_getPlayerSave();
    if (!p)
        return;
    if (cap > dSv_MAGIC_MAX)
        cap = dSv_MAGIC_MAX;
    p->mPlayerStatusA.mMaxMagic = cap;
    if (p->mPlayerStatusA.mMagic > cap)
        p->mPlayerStatusA.mMagic = cap;
}

/** [V] Wallet size 0..2 (500 / 1000 / 5000). Rupees are clamped to the new cap
 *  the way the meter would on its next pass. */
static __inline u8 dSv_getWalletSize(void) {
    dSv_player_c* p = dComIfGs_getPlayerSave();
    return p ? p->mPlayerStatusA.mWalletSize : (u8)0;
}
static __inline void dSv_setWalletSize(u8 size) {
    dSv_player_c* p = dComIfGs_getPlayerSave();
    if (!p)
        return;
    if (size >= WWHD_WALLET_SIZES)
        size = WWHD_WALLET_SIZES - 1;
    p->mPlayerStatusA.mWalletSize = size;
    if (p->mPlayerStatusA.mRupee > dSv_walletCapacity(size))
        p->mPlayerStatusA.mRupee = dSv_walletCapacity(size);
}

/** [V] Quiver capacity; arrows carried are clamped to it. */
static __inline void dSv_setMaxArrowNum(u8 cap) {
    dSv_player_c* p = dComIfGs_getPlayerSave();
    if (!p)
        return;
    p->mItemMax.mArrowNum = cap;
    if (p->mItemRecord.mArrowNum > cap)
        p->mItemRecord.mArrowNum = cap;
}

/** [V] Bomb bag capacity; bombs carried are clamped to it. */
static __inline void dSv_setMaxBombNum(u8 cap) {
    dSv_player_c* p = dComIfGs_getPlayerSave();
    if (!p)
        return;
    p->mItemMax.mBombNum = cap;
    if (p->mItemRecord.mBombNum > cap)
        p->mItemRecord.mBombNum = cap;
}

/* --- dungeon items of the current stage ----------------------------------- */

/** [V] mDungeonItem bit of the stage Link is in. */
static __inline int dSv_isDungeonItem(int bit) {
    dSv_memBit_c* b = dSv_getCurrentMemBit();
    return b && bit >= 0 && bit < 6 && ((b->mDungeonItem >> bit) & 1);
}
static __inline void dSv_setDungeonItem(int bit, int on) {
    dSv_memBit_c* b = dSv_getCurrentMemBit();
    if (!b || bit < 0 || bit >= 6)
        return;
    if (on) b->mDungeonItem |= (u8)(1u << bit); else b->mDungeonItem &= (u8)~(1u << bit);
}

#endif /* LIBWWHD_D_SAVE_ITEM_ACCESS_H */
