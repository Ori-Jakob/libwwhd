#ifndef LIBWWHD_D_ITEM_H
#define LIBWWHD_D_ITEM_H

#include "libwwhd/wwhd_types.h"

/**
 * libwwhd - item numbers, and where each one lands in the save (d_item.cpp)
 *
 * An item number is the index into l_itemGetFunc (wwhd_map->itemGetFuncTable),
 * the 256-entry table of give functions the game runs when Link receives an
 * item. That makes every number below a checkable fact rather than a decomp
 * transcription: decompile the entry and read which save field it writes. All
 * of them were, in USA; the table is .data and the stubs are the same shape in
 * EUR and JAP. docs/SUBSYSTEMS.md (Items and the give table) has the sweep.
 *
 * What a stub does is the other thing recorded here, because it is what an
 * editor has to reproduce:
 *
 *   INVENTORY items write mPlayerItem.mItems[slot] = id and set bit `tier` of
 *   mGetItem.mItemFlags[slot] through dSv_player_get_item_c::onItem
 *   (0x025B5CCC, asserting "0 <= i_item && i_item < 8"). One slot per family,
 *   one tier bit per upgrade: bow 0, fire & ice arrows 1, light arrows 2.
 *
 *   SWORDS, SHIELDS, the BRACELETS and the two CHARMS are not in the slots. They
 *   set a bit in mCollect.mCollect[idx] through onCollect (0x025B7944) and, for
 *   the first three, record the equipped id in mSelectEquip[idx] through the
 *   equip setter at 0x02522398.
 *
 *   BAG items scan their 8-byte array for 0xFF, put the id there, set the bag's
 *   flag bit, and (spoils) add one to a pending count in play.
 *
 *   SONGS, PEARLS and SHARDS set a bit in mCollect.mTact / mSymbol / mTriforce
 *   (0x025B7AA8 / 0x025B7CC0 / 0x025B7B80).
 *
 *   PICKUPS add to a pending delta in play that the meter folds into the save.
 *
 * NAMES are the GameCube names: the binary carries no item-name strings, so a
 * name is [P] wherever the HD release could have renamed the thing on screen.
 * Two ids are HD facts: 0x21 is the Tingle Bottle (the Tuner's slot and tier),
 * and 0x77 is the Swift Sail, tier 1 of the sail slot. Hero's Clothes 0x32 and
 * the Tingle statues 0xA3..0xA8 are empty entries in this build - their stubs
 * are a bare blr - as are 0x13, 0x17..0x19, 0x1B..0x1E, 0x2B, 0x2E, 0x37,
 * 0x3F..0x41, 0x44, 0x5A..0x60, 0x6C, 0x73..0x76, 0x79..0x81, 0x84..0x8B,
 * 0x9F, 0xA9, 0xAA and 0xB1.
 */

/* [V] Item numbers. The comment names the field the give stub writes. */
typedef enum dItemNo_e {
    /* pickups: each adds to a pending delta in play (d_com_inf_game.h) */
    dItemNo_HEART               = 0x00, /* [V] life */
    dItemNo_GREEN_RUPEE         = 0x01, /* [V] rupees +1 */
    dItemNo_BLUE_RUPEE          = 0x02, /* [V] +5 */
    dItemNo_YELLOW_RUPEE        = 0x03, /* [V] +10 */
    dItemNo_RED_RUPEE           = 0x04, /* [V] +20 */
    dItemNo_PURPLE_RUPEE        = 0x05, /* [V] +50 */
    dItemNo_ORANGE_RUPEE        = 0x06, /* [V] +100 */
    dItemNo_PIECE_OF_HEART      = 0x07, /* [V] max life +1 quarter */
    dItemNo_HEART_CONTAINER     = 0x08, /* [V] max life +4, refills */
    dItemNo_SMALL_MAGIC_JAR     = 0x09, /* [V] magic +4 */
    dItemNo_LARGE_MAGIC_JAR     = 0x0A, /* [V] magic +8 */
    dItemNo_BOMBS_5             = 0x0B, /* [V] bombs +5 */
    dItemNo_BOMBS_10            = 0x0C, /* [V] +10; also writes the bomb slot */
    dItemNo_BOMBS_20            = 0x0D, /* [V] +20 */
    dItemNo_BOMBS_30            = 0x0E, /* [V] +30 */
    dItemNo_SILVER_RUPEE        = 0x0F, /* [V] rupees +200 */
    dItemNo_ARROWS_10           = 0x10, /* [V] arrows +10 */
    dItemNo_ARROWS_20           = 0x11, /* [V] +20 */
    dItemNo_ARROWS_30           = 0x12, /* [V] +30 */
    dItemNo_JOY_PENDANT         = 0x1F, /* [V] spoils bag, index 7 */

    /* inventory slots: mItems[slot] = id, mItemFlags[slot] |= 1 << tier */
    dItemNo_TELESCOPE           = 0x20, /* [V] slot 0, tier 0 */
    dItemNo_TINGLE_BOTTLE       = 0x21, /* [V] slot 7, tier 0. Name [P]: the
                                         *     Tuner's number, HD's bottle   */
    dItemNo_WIND_WAKER          = 0x22, /* [V] slot 2, tier 0 */
    dItemNo_PICTO_BOX           = 0x23, /* [V] slot 8, tier 0 */
    dItemNo_SPOILS_BAG          = 0x24, /* [V] slot 4, tier 0 */
    dItemNo_GRAPPLING_HOOK      = 0x25, /* [V] slot 3, tier 0 */
    dItemNo_DELUXE_PICTO_BOX    = 0x26, /* [V] slot 8, tier 1 */
    dItemNo_BOW                 = 0x27, /* [V] slot 12, tier 0; quiver 30/30 */
    dItemNo_POWER_BRACELETS     = 0x28, /* [V] collect[2] bit 0, equip[2] */
    dItemNo_IRON_BOOTS          = 0x29, /* [V] slot 9, tier 0 */
    dItemNo_MAGIC_ARMOR         = 0x2A, /* [V] slot 10, tier 0 */
    dItemNo_BAIT_BAG            = 0x2C, /* [V] slot 11, tier 0 */
    dItemNo_BOOMERANG           = 0x2D, /* [V] slot 5, tier 0 */
    dItemNo_HOOKSHOT            = 0x2F, /* [V] slot 19, tier 0 */
    dItemNo_DELIVERY_BAG        = 0x30, /* [V] slot 18, tier 0 */
    dItemNo_BOMBS               = 0x31, /* [V] slot 13, tier 0; bag 30/30 */
    dItemNo_SKULL_HAMMER        = 0x33, /* [V] slot 20, tier 0 */
    dItemNo_DEKU_LEAF           = 0x34, /* [V] slot 6, tier 0; magic +16/+16 */
    dItemNo_FIRE_ICE_ARROWS     = 0x35, /* [V] slot 12, tier 1 */
    dItemNo_LIGHT_ARROWS        = 0x36, /* [V] slot 12, tier 2 */

    /* equipment: collect bank + mSelectEquip */
    dItemNo_HEROS_SWORD         = 0x38, /* [V] collect[0] bit 0, equip[0] */
    dItemNo_MASTER_SWORD        = 0x39, /* [V] collect[0] bit 1 (powerless) */
    dItemNo_MASTER_SWORD_HALF   = 0x3A, /* [V] collect[0] bit 2 */
    dItemNo_HEROS_SHIELD        = 0x3B, /* [V] collect[1] bit 0, equip[1] */
    dItemNo_MIRROR_SHIELD       = 0x3C, /* [V] collect[1] bit 1 */
    dItemNo_HEROS_SWORD_AGAIN   = 0x3D, /* [V] same stub body as 0x38 */
    dItemNo_MASTER_SWORD_FULL   = 0x3E, /* [V] collect[0] bit 3 */
    dItemNo_PIRATES_CHARM       = 0x42, /* [V] collect[3] bit 0 */
    dItemNo_HEROS_CHARM         = 0x43, /* [V] collect[4] bit 0 */

    /* spoils bag, in the order of their index */
    dItemNo_SKULL_NECKLACE      = 0x45, /* [V] index 0 */
    dItemNo_BOKO_BABA_SEED      = 0x46, /* [V] 1 */
    dItemNo_GOLDEN_FEATHER      = 0x47, /* [V] 2 */
    dItemNo_KNIGHTS_CREST       = 0x48, /* [V] 3 */
    dItemNo_RED_CHU_JELLY       = 0x49, /* [V] 4 */
    dItemNo_GREEN_CHU_JELLY     = 0x4A, /* [V] 5 */
    dItemNo_BLUE_CHU_JELLY      = 0x4B, /* [V] 6 */

    /* dungeon items of the current stage: mDungeonItem bits */
    dItemNo_DUNGEON_MAP         = 0x4C, /* [V] bit 0 */
    dItemNo_COMPASS             = 0x4D, /* [V] bit 1 */
    dItemNo_BIG_KEY             = 0x4E, /* [V] bit 2 */

    /* bottle contents: what a bottle slot holds */
    dItemNo_EMPTY_BOTTLE        = 0x50, /* [V] placed in the first free bottle */
    dItemNo_RED_POTION          = 0x51, /* [V] replaces an empty bottle */
    dItemNo_GREEN_POTION        = 0x52, /* [V] */
    dItemNo_BLUE_POTION         = 0x53, /* [V] */
    dItemNo_ELIXIR_SOUP_HALF    = 0x54, /* [P] flag only; name GC */
    dItemNo_ELIXIR_SOUP         = 0x55, /* [P] */
    dItemNo_WATER               = 0x56, /* [P] */
    dItemNo_FAIRY               = 0x57, /* [P] */
    dItemNo_FOREST_FIREFLY      = 0x58, /* [P] */
    dItemNo_FOREST_WATER        = 0x59, /* [P] */

    /* collect bank bits */
    dItemNo_TRIFORCE_SHARD_1    = 0x61, /* [V] mTriforce bit 0 .. 7 */
    dItemNo_TRIFORCE_SHARD_8    = 0x68,
    dItemNo_NAYRUS_PEARL        = 0x69, /* [V] mSymbol bit 0 */
    dItemNo_DINS_PEARL          = 0x6A, /* [V] bit 1 */
    dItemNo_FARORES_PEARL       = 0x6B, /* [V] bit 2 */
    dItemNo_WINDS_REQUIEM       = 0x6D, /* [V] mTact bit 0; name [P] */
    dItemNo_BALLAD_OF_GALES     = 0x6E, /* [V] bit 1 */
    dItemNo_COMMAND_MELODY      = 0x6F, /* [V] bit 2 */
    dItemNo_EARTH_GODS_LYRIC    = 0x70, /* [V] bit 3 */
    dItemNo_WIND_GODS_ARIA      = 0x71, /* [V] bit 4 */
    dItemNo_SONG_OF_PASSING     = 0x72, /* [V] bit 5 */

    dItemNo_SWIFT_SAIL          = 0x77, /* [V] slot 1, tier 1. HD only. */
    dItemNo_SAIL                = 0x78, /* [V] slot 1, tier 0 */

    /* bait bag */
    dItemNo_ALL_PURPOSE_BAIT    = 0x82, /* [V] index 0 */
    dItemNo_HYOI_PEAR           = 0x83, /* [V] index 1 */

    /* delivery bag, by flag index */
    dItemNo_TOWN_FLOWER         = 0x8C, /* [V] index 0 */
    dItemNo_SEA_FLOWER          = 0x8D, /* [V] 1 */
    dItemNo_EXOTIC_FLOWER       = 0x8E, /* [V] 2 */
    dItemNo_HEROS_FLAG          = 0x8F, /* [V] 3 */
    dItemNo_BIG_CATCH_FLAG      = 0x90, /* [V] 4 */
    dItemNo_BIG_SALE_FLAG       = 0x91, /* [V] 5 */
    dItemNo_PINWHEEL            = 0x92, /* [V] 6 */
    dItemNo_SICKLE_MOON_FLAG    = 0x93, /* [V] 7 */
    dItemNo_SKULL_TOWER_IDOL    = 0x94, /* [V] 8 */
    dItemNo_FOUNTAIN_IDOL       = 0x95, /* [V] 9 */
    dItemNo_POSTMAN_STATUE      = 0x96, /* [V] 10 */
    dItemNo_SHOP_GURU_STATUE    = 0x97, /* [V] 11 */
    dItemNo_FATHERS_LETTER      = 0x98, /* [V] 12 */
    dItemNo_NOTE_TO_MOM         = 0x99, /* [V] 13 */
    dItemNo_MAGGIES_LETTER      = 0x9A, /* [V] 14 */
    dItemNo_MOBLINS_LETTER      = 0x9B, /* [V] 15 */
    dItemNo_CABANA_DEED         = 0x9C, /* [V] 16 */
    dItemNo_COMPLIMENTARY_ID    = 0x9D, /* [V] 17 */
    dItemNo_FILL_UP_COUPON      = 0x9E, /* [V] 18 */
    dItemNo_LEGENDARY_PICTOGRAPH= 0xA0, /* [V] 20; name [P] */
    dItemNo_DELIVERY_A1         = 0xA1, /* [V] 21; name unknown */
    dItemNo_DELIVERY_A2         = 0xA2, /* [V] 22; name unknown */

    /* capacity upgrades: write the capacity directly */
    dItemNo_WALLET_1000         = 0xAB, /* [V] mWalletSize = 1 */
    dItemNo_WALLET_5000         = 0xAC, /* [V] mWalletSize = 2 */
    dItemNo_BOMB_BAG_60         = 0xAD, /* [V] bombs 60/60 */
    dItemNo_BOMB_BAG_99         = 0xAE, /* [V] 99/99 */
    dItemNo_QUIVER_60           = 0xAF, /* [V] arrows 60/60 */
    dItemNo_QUIVER_99           = 0xB0, /* [V] 99/99 */
    dItemNo_MAGIC_METER_UPGRADE = 0xB2, /* [V] max magic +32 pending */
    dItemNo_RUPEE_BONUS_50      = 0xB3, /* [V] rupees +50 .. 0xB8 is +500 */

    /* Treasure Charts: chart 61 is 0xC2 and chart 1 is 0xFE (d_save_access.h) */
    dItemNo_TREASURE_CHART_61   = 0xC2,
    dItemNo_TREASURE_CHART_1    = 0xFE,
    dItemNo_NONE                = 0xFF
} dItemNo_e;

/** [V] The 21 inventory slots, named by what each give stub writes into it. */
typedef enum dSv_itemSlot_e {
    dSv_SLOT_TELESCOPE      = 0,
    dSv_SLOT_SAIL           = 1,
    dSv_SLOT_WIND_WAKER     = 2,
    dSv_SLOT_GRAPPLING_HOOK = 3,
    dSv_SLOT_SPOILS_BAG     = 4,
    dSv_SLOT_BOOMERANG      = 5,
    dSv_SLOT_DEKU_LEAF      = 6,
    dSv_SLOT_TINGLE_BOTTLE  = 7,
    dSv_SLOT_PICTO_BOX      = 8,
    dSv_SLOT_IRON_BOOTS     = 9,
    dSv_SLOT_MAGIC_ARMOR    = 10,
    dSv_SLOT_BAIT_BAG       = 11,
    dSv_SLOT_BOW            = 12,
    dSv_SLOT_BOMBS          = 13,
    dSv_SLOT_BOTTLE_1       = 14,
    dSv_SLOT_BOTTLE_2       = 15,
    dSv_SLOT_BOTTLE_3       = 16,
    dSv_SLOT_BOTTLE_4       = 17,
    dSv_SLOT_DELIVERY_BAG   = 18,
    dSv_SLOT_HOOKSHOT       = 19,
    dSv_SLOT_SKULL_HAMMER   = 20,
    dSv_SLOT_COUNT          = 21
} dSv_itemSlot_e;

/** [V] Bottle slots run 14..17. */
#define WWHD_BOTTLE_COUNT 4

/** [V] mSelectEquip categories. Each give stub passes the category to the
 *  equip setter with the id it records there. */
#define WWHD_EQUIP_SWORD      0
#define WWHD_EQUIP_SHIELD     1
#define WWHD_EQUIP_BRACELETS  2

/** [V] mCollect indices the stubs write. */
#define WWHD_COLLECT_SWORD    0
#define WWHD_COLLECT_SHIELD   1
#define WWHD_COLLECT_BRACELET 2
#define WWHD_COLLECT_PIRATES_CHARM 3
#define WWHD_COLLECT_HEROS_CHARM   4

/** [V] mDungeonItem bits, from the 0x4C..0x4E stubs. Bit 4 is set by a heart
 *  container taken inside a dungeon and is not named. */
#define WWHD_DUNGEON_ITEM_MAP     0
#define WWHD_DUNGEON_ITEM_COMPASS 1
#define WWHD_DUNGEON_ITEM_BIG_KEY 2

/** [V] Ammo capacities the game hands out. */
#define WWHD_AMMO_CAP_NONE  0
#define WWHD_AMMO_CAP_30   30
#define WWHD_AMMO_CAP_60   60
#define WWHD_AMMO_CAP_99   99

/** [V] Wallet sizes: index into dSv_walletCapacity(). */
#define WWHD_WALLET_SIZES 3

/** An inventory item: number, the slot its stub writes, the tier bit it sets.
 *  Entries sharing a slot are that slot's upgrades, in tier order. */
typedef struct wwhd_slotItem_t {
    u8          id;
    u8          slot;
    u8          tier;
    const char* name;
} wwhd_slotItem_t;

/** [V] ids, slots and tiers; names [P]. Ordered by slot, then tier. */
static const wwhd_slotItem_t wwhd_slotItems[] = {
    { 0x20,  0, 0, "Telescope" },
    { 0x78,  1, 0, "Sail" },
    { 0x77,  1, 1, "Swift Sail" },
    { 0x22,  2, 0, "Wind Waker" },
    { 0x25,  3, 0, "Grappling Hook" },
    { 0x24,  4, 0, "Spoils Bag" },
    { 0x2D,  5, 0, "Boomerang" },
    { 0x34,  6, 0, "Deku Leaf" },
    { 0x21,  7, 0, "Tingle Bottle" },
    { 0x23,  8, 0, "Picto Box" },
    { 0x26,  8, 1, "Deluxe Picto Box" },
    { 0x29,  9, 0, "Iron Boots" },
    { 0x2A, 10, 0, "Magic Armor" },
    { 0x2C, 11, 0, "Bait Bag" },
    { 0x27, 12, 0, "Bow" },
    { 0x35, 12, 1, "Fire & Ice Arrows" },
    { 0x36, 12, 2, "Light Arrows" },
    { 0x31, 13, 0, "Bombs" },
    { 0x30, 18, 0, "Delivery Bag" },
    { 0x2F, 19, 0, "Hookshot" },
    { 0x33, 20, 0, "Skull Hammer" },
};
#define WWHD_SLOT_ITEM_COUNT ((int)(sizeof(wwhd_slotItems) / sizeof(wwhd_slotItems[0])))

/** A bag entry: item number, its index in the bag's flags and counts. */
typedef struct wwhd_bagItem_t {
    u8          id;
    u8          index;
    const char* name;
} wwhd_bagItem_t;

/** [V] ids and indices; names [P]. */
static const wwhd_bagItem_t wwhd_spoils[8] = {
    { 0x45, 0, "Skull Necklace" },
    { 0x46, 1, "Boko Baba Seed" },
    { 0x47, 2, "Golden Feather" },
    { 0x48, 3, "Knight's Crest" },
    { 0x49, 4, "Red Chu Jelly" },
    { 0x4A, 5, "Green Chu Jelly" },
    { 0x4B, 6, "Blue Chu Jelly" },
    { 0x1F, 7, "Joy Pendant" },
};

static const wwhd_bagItem_t wwhd_baits[2] = {
    { 0x82, 0, "All-Purpose Bait" },
    { 0x83, 1, "Hyoi Pear" },
};

static const wwhd_bagItem_t wwhd_deliveryItems[21] = {
    { 0x8C,  0, "Town Flower" },
    { 0x8D,  1, "Sea Flower" },
    { 0x8E,  2, "Exotic Flower" },
    { 0x8F,  3, "Hero's Flag" },
    { 0x90,  4, "Big Catch Flag" },
    { 0x91,  5, "Big Sale Flag" },
    { 0x92,  6, "Pinwheel" },
    { 0x93,  7, "Sickle Moon Flag" },
    { 0x94,  8, "Skull Tower Idol" },
    { 0x95,  9, "Fountain Idol" },
    { 0x96, 10, "Postman Statue" },
    { 0x97, 11, "Shop Guru Statue" },
    { 0x98, 12, "Father's Letter" },
    { 0x99, 13, "Note to Mom" },
    { 0x9A, 14, "Maggie's Letter" },
    { 0x9B, 15, "Moblin's Letter" },
    { 0x9C, 16, "Cabana Deed" },
    { 0x9D, 17, "Complimentary ID" },
    { 0x9E, 18, "Fill-Up Coupon" },
    { 0xA0, 20, "Legendary Pictograph" },
    { 0xA1, 21, "Delivery item 0xA1" },
};
/* 0xA2 (index 22) is the same shape, left out only because nothing names it. */

/** [V] What a bottle slot can hold, 0x50..0x59. index is unused. */
static const wwhd_bagItem_t wwhd_bottleContents[10] = {
    { 0x50, 0, "Empty Bottle" },
    { 0x51, 0, "Red Potion" },
    { 0x52, 0, "Green Potion" },
    { 0x53, 0, "Blue Potion" },
    { 0x54, 0, "Elixir Soup (half)" },
    { 0x55, 0, "Elixir Soup" },
    { 0x56, 0, "Water" },
    { 0x57, 0, "Fairy" },
    { 0x58, 0, "Forest Firefly" },
    { 0x59, 0, "Forest Water" },
};

/** A collect-bank entry: item number and the bit its stub sets. */
typedef struct wwhd_collectBit_t {
    u8          id;
    u8          bit;
    const char* name;
} wwhd_collectBit_t;

/** [V] mTact bits 0..5; names [P] in GameCube order. */
static const wwhd_collectBit_t wwhd_songs[6] = {
    { 0x6D, 0, "Wind's Requiem" },
    { 0x6E, 1, "Ballad of Gales" },
    { 0x6F, 2, "Command Melody" },
    { 0x70, 3, "Earth God's Lyric" },
    { 0x71, 4, "Wind God's Aria" },
    { 0x72, 5, "Song of Passing" },
};

/** [V] mSymbol bits 0..2. */
static const wwhd_collectBit_t wwhd_pearls[3] = {
    { 0x69, 0, "Nayru's Pearl" },
    { 0x6A, 1, "Din's Pearl" },
    { 0x6B, 2, "Farore's Pearl" },
};

/** [V] mCollect[0] bits, one per sword the game can hand out. Holding a later
 *  sword implies the earlier bits, which is how the give stubs leave them. */
static const wwhd_collectBit_t wwhd_swords[4] = {
    { 0x38, 0, "Hero's Sword" },
    { 0x39, 1, "Master Sword" },
    { 0x3A, 2, "Master Sword (half power)" },
    { 0x3E, 3, "Master Sword (full power)" },
};

/** [V] mCollect[1] bits. */
static const wwhd_collectBit_t wwhd_shields[2] = {
    { 0x3B, 0, "Hero's Shield" },
    { 0x3C, 1, "Mirror Shield" },
};

#endif /* LIBWWHD_D_ITEM_H */
