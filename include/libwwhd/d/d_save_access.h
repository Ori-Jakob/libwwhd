#ifndef LIBWWHD_D_SAVE_ACCESS_H
#define LIBWWHD_D_SAVE_ACCESS_H

#include "libwwhd/wwhd_types.h"
#include "libwwhd/d/d_save.h"
#include "libwwhd/d/d_save_bits.h"
#include "libwwhd/d/d_map.h"
#include "libwwhd/d/d_com_inf_game.h"

/**
 * libwwhd - save reads and writes through the game info root.
 *
 * d_save.h describes the save block and holds the helpers that take a bank
 * pointer explicitly; this header holds the ones that go and FIND the block,
 * which is why it needs the root and lives separately.
 *
 * Only fields whose offset is [V] get a setter. Hearts are quarter-hearts, so
 * one container is 4. Every helper is NULL-safe before a region is selected.
 */

/** [V] Current rupees, or 0 when unavailable. */
static __inline u16 dSv_getRupee(void) {
    dSv_player_c* p = dComIfGs_getPlayerSave();
    return p ? p->mPlayerStatusA.mRupee : (u16)0;
}

/**
 * [V] Set rupees. The wallet cap is not applied - the game clamps on its own
 * next update, and libwwhd has not verified mWalletSize.
 *
 * This writes the save value only. The on-screen counter does NOT follow it:
 * the meter re-reads the save value when a pending delta arrives and at no
 * other time, so a value set here appears on the next pickup or purchase. To
 * show it now, pair this with dMeter_setRupeeDisplay() (d_meter.h). A pending
 * pickup is the other thing to mind: see dComIfGp_clearItemDeltas() in
 * d_com_inf_game.h, and prefer dSv_setRupeeExact() if you want the number you
 * asked for.
 */
static __inline void dSv_setRupee(u16 rupees) {
    dSv_player_c* p = dComIfGs_getPlayerSave();
    if (p)
        p->mPlayerStatusA.mRupee = rupees;
}

/** [V] Set rupees and drop any pending pickup, so the wallet ends up holding
 *  exactly this. */
static __inline void dSv_setRupeeExact(u16 rupees) {
    dComIfGp_clearItemDeltas();
    dSv_setRupee(rupees);
}

/** [V] Current life in quarter-hearts. */
static __inline u16 dSv_getLife(void) {
    dSv_player_c* p = dComIfGs_getPlayerSave();
    return p ? p->mPlayerStatusA.mLife : (u16)0;
}

/** [V] Maximum life in quarter-hearts. */
static __inline u16 dSv_getMaxLife(void) {
    dSv_player_c* p = dComIfGs_getPlayerSave();
    return p ? p->mPlayerStatusA.mMaxLife : (u16)0;
}

/** [V] Set current life, clamped to the maximum so the meter cannot overfill. */
static __inline void dSv_setLife(u16 quarters) {
    dSv_player_c* p = dComIfGs_getPlayerSave();
    if (!p)
        return;
    if (quarters > p->mPlayerStatusA.mMaxLife)
        quarters = p->mPlayerStatusA.mMaxLife;
    p->mPlayerStatusA.mLife = quarters;
}

/** [V] Set maximum life, bringing current life down with it if it would
 *  otherwise exceed the new maximum. */
static __inline void dSv_setMaxLife(u16 quarters) {
    dSv_player_c* p = dComIfGs_getPlayerSave();
    if (!p)
        return;
    p->mPlayerStatusA.mMaxLife = quarters;
    if (p->mPlayerStatusA.mLife > quarters)
        p->mPlayerStatusA.mLife = quarters;
}

/** [V] Refill to full. Drops any pending life delta first so the result is
 *  exactly full rather than full plus an in-flight heart. */
static __inline void dSv_healFull(void) {
    dComIfGp_clearItemDeltas();
    dSv_setLife(dSv_getMaxLife());
}

/* --- Magic ---------------------------------------------------------------
 * The meter at 0x0259B698 owns both bytes: it grants a pending max increase
 * capped at dSv_MAGIC_MAX, applies a pending delta floored at zero, and then
 * clamps current down to max. Writing max without current, or the reverse, is
 * therefore safe - the next meter pass reconciles them.
 * ---------------------------------------------------------------------- */

/** [V] Current magic, or 0 when unavailable. */
static __inline u8 dSv_getMagic(void) {
    dSv_player_c* p = dComIfGs_getPlayerSave();
    return p ? p->mPlayerStatusA.mMagic : (u8)0;
}

/** [V] Magic capacity, or 0 when unavailable. Zero is a real value: Link has
 *  no magic meter at all until the first upgrade. */
static __inline u8 dSv_getMaxMagic(void) {
    dSv_player_c* p = dComIfGs_getPlayerSave();
    return p ? p->mPlayerStatusA.mMaxMagic : (u8)0;
}

/** [V] Set current magic, clamped to capacity so the meter cannot overfill. */
static __inline void dSv_setMagic(u8 magic) {
    dSv_player_c* p = dComIfGs_getPlayerSave();
    if (!p)
        return;
    if (magic > p->mPlayerStatusA.mMaxMagic)
        magic = p->mPlayerStatusA.mMaxMagic;
    p->mPlayerStatusA.mMagic = magic;
}

/** [V] Refill magic to capacity. A no-op while capacity is zero, which is what
 *  keeps this from handing out a meter Link has not earned. */
static __inline void dSv_refillMagic(void) {
    dSv_setMagic(dSv_getMaxMagic());
}

/* --- Arrows and bombs ---------------------------------------------------
 * Carried counts live in mItemRecord, capacities in mItemMax. The pairing is
 * not inferred: the bow stub at 0x0254E630 writes 30 to the arrow count and 30
 * to the arrow capacity in consecutive stores, and the bomb bag stub at
 * 0x0254E888 does the same for bombs.
 * ---------------------------------------------------------------------- */

/** [V] Arrows carried, or 0 when unavailable. */
static __inline u8 dSv_getArrowNum(void) {
    dSv_player_c* p = dComIfGs_getPlayerSave();
    return p ? p->mItemRecord.mArrowNum : (u8)0;
}

/** [V] Quiver capacity, or 0 when unavailable. Zero until the bow is found. */
static __inline u8 dSv_getMaxArrowNum(void) {
    dSv_player_c* p = dComIfGs_getPlayerSave();
    return p ? p->mItemMax.mArrowNum : (u8)0;
}

/** [V] Set arrows carried, clamped to the quiver. */
static __inline void dSv_setArrowNum(u8 arrows) {
    dSv_player_c* p = dComIfGs_getPlayerSave();
    if (!p)
        return;
    if (arrows > p->mItemMax.mArrowNum)
        arrows = p->mItemMax.mArrowNum;
    p->mItemRecord.mArrowNum = arrows;
}

/** [V] Bombs carried, or 0 when unavailable. */
static __inline u8 dSv_getBombNum(void) {
    dSv_player_c* p = dComIfGs_getPlayerSave();
    return p ? p->mItemRecord.mBombNum : (u8)0;
}

/** [V] Bomb bag capacity, or 0 when unavailable. */
static __inline u8 dSv_getMaxBombNum(void) {
    dSv_player_c* p = dComIfGs_getPlayerSave();
    return p ? p->mItemMax.mBombNum : (u8)0;
}

/** [V] Set bombs carried, clamped to the bag. */
static __inline void dSv_setBombNum(u8 bombs) {
    dSv_player_c* p = dComIfGs_getPlayerSave();
    if (!p)
        return;
    if (bombs > p->mItemMax.mBombNum)
        bombs = p->mItemMax.mBombNum;
    p->mItemRecord.mBombNum = bombs;
}

/** [V] Refill both ammo pools to their capacities. Each is a no-op while its
 *  capacity is zero, so this never hands out ammo for an item Link lacks. */
static __inline void dSv_refillAmmo(void) {
    dSv_setArrowNum(dSv_getMaxArrowNum());
    dSv_setBombNum(dSv_getMaxBombNum());
}

/* --- Rupees -------------------------------------------------------------- */

/** [V] What the current wallet holds. 0 when the save is unavailable, which is
 *  distinguishable from a real capacity because the smallest wallet is 500. */
static __inline u16 dSv_getMaxRupee(void) {
    dSv_player_c* p = dComIfGs_getPlayerSave();
    return p ? dSv_walletCapacity(p->mPlayerStatusA.mWalletSize) : (u16)0;
}

/** [V] Fill the wallet. Goes through dSv_setRupeeExact so a pickup already in
 *  flight cannot land afterwards and push the total over the cap. */
static __inline void dSv_refillRupees(void) {
    u16 cap = dSv_getMaxRupee();
    if (cap)
        dSv_setRupeeExact(cap);
}

/** [V] Read an inventory slot, or 0xFF when unavailable. */
static __inline u8 dSv_getItem(int slot) {
    dSv_player_c* p = dComIfGs_getPlayerSave();
    if (!p || slot < 0 || slot >= WWHD_ITEM_SLOTS)
        return (u8)WWHD_ITEM_NONE;
    return p->mPlayerItem.mItems[slot];
}

/**
 * [V] Write an inventory slot. Returns non-zero on success.
 *
 * This is the raw slot write. It does NOT run the game's acquisition gate, so
 * it will happily place an id the game would have refused; that is the point of
 * a tool, but it also means an invalid id shows as a broken icon rather than a
 * crash. Item IDS are not enumerated here - they come from d_item_data, have
 * not been checked against WWHD, and WWHD is known to have renumbered at least
 * one id enum.
 */
static __inline int dSv_setItem(int slot, u8 itemId) {
    dSv_player_c* p = dComIfGs_getPlayerSave();
    if (!p || slot < 0 || slot >= WWHD_ITEM_SLOTS)
        return 0;
    p->mPlayerItem.mItems[slot] = itemId;
    return 1;
}

/** [V] The current stage's scratch memory bits. */
static __inline dSv_memBit_c* dSv_getCurrentMemBit(void) {
    dSv_info_c* s = dComIfGs_getSaveInfo();
    return s ? &s->mMemory.mMembit : (dSv_memBit_c*)0;
}

/** [V] One zone's bit bank, or NULL. zoneNo < dSv_ZONE_MAX. */
static __inline dSv_zoneBit_c* dSv_getZoneBit(int zoneNo) {
    dSv_info_c* s = dComIfGs_getSaveInfo();
    if (!s || zoneNo < 0 || zoneNo >= dSv_ZONE_MAX)
        return (dSv_zoneBit_c*)0;
    return &s->mZone[zoneNo].mZoneBit;
}

/* ========================================================================
 * Charts, the sea chart, collectables and event flags
 *
 * The 1-based layer. Everything below takes a chart id the way the game's own
 * dComIfGs_ wrappers take one - 1..61 - and subtracts the one before touching
 * the bank, exactly as they do. The 0-based bank helpers are in d_save_bits.h;
 * do not mix the two.
 * ===================================================================== */

/** [V] The chart bank, or NULL before a region is selected. */
static __inline dSv_player_map_c* dComIfGs_getMap(void) {
    dSv_player_c* p = dComIfGs_getPlayerSave();
    return p ? &p->mMap : (dSv_player_map_c*)0;
}

/** [V] The collectables bank, or NULL. */
static __inline dSv_player_collect_c* dComIfGs_getCollect(void) {
    dSv_player_c* p = dComIfGs_getPlayerSave();
    return p ? &p->mCollect : (dSv_player_collect_c*)0;
}

/** [V] The ocean salvage bank, or NULL. */
static __inline dSv_ocean_c* dComIfGs_getOcean(void) {
    dSv_info_c* s = dComIfGs_getSaveInfo();
    return s ? &s->mSavedata.mOcean : (dSv_ocean_c*)0;
}

/** [V] The persistent event bank, or NULL. */
static __inline dSv_event_c* dComIfGs_getEvent(void) {
    dSv_info_c* s = dComIfGs_getSaveInfo();
    return s ? &s->mSavedata.mEvent : (dSv_event_c*)0;
}

/** [V] The scratch event bank, or NULL. Same encoding, not written to the
 *  card - 164 call sites share the five routines with the persistent one. */
static __inline dSv_event_c* dComIfGs_getTmpEvent(void) {
    dSv_info_c* s = dComIfGs_getSaveInfo();
    return s ? &s->mTmp : (dSv_event_c*)0;
}

/**
 * [V] Chart id for an item number, or 0 when the item is not a chart.
 *
 * The item table stores the 61 chart give-stubs in descending order ending at
 * item 0xFE, so the relation is a subtraction: chart 1 is item 0xFE and chart
 * 61 is item 0xC2. Confirmed by the table geometry in all three regions, and
 * stated the same way in the GameCube source.
 */
static __inline int dSv_chartIdFromItemNo(u8 itemNo) {
    int id = 0xFF - (int)itemNo;
    return WWHD_CHART_ID_VALID(id) ? id : 0;
}

/** [V] Item number for a chart id, or 0xFF (none) when the id is out of range. */
static __inline u8 dSv_itemNoFromChartId(int chartId) {
    return WWHD_CHART_ID_VALID(chartId) ? (u8)(0xFF - chartId) : (u8)0xFF;
}

/** [V] Do you own chart 1..61. */
static __inline int dComIfGs_isGetCollectMap(int chartId) {
    return WWHD_CHART_ID_VALID(chartId) &&
           dSv_map_isGetMap(dComIfGs_getMap(), chartId - 1);
}

/** [V] Has chart 1..61 been opened. */
static __inline int dComIfGs_isOpenCollectMap(int chartId) {
    return WWHD_CHART_ID_VALID(chartId) &&
           dSv_map_isOpenMap(dComIfGs_getMap(), chartId - 1);
}

/** [V] Has chart 1..61's treasure been salvaged. */
static __inline int dComIfGs_isCompleteCollectMap(int chartId) {
    return WWHD_CHART_ID_VALID(chartId) &&
           dSv_map_isCompleteMap(dComIfGs_getMap(), chartId - 1);
}

/**
 * [V] Grant chart 1..61 exactly the way picking it up does: set Get, clear
 * Open and Complete. Returns non-zero on success.
 *
 * Clearing the other two is not tidiness - it is what the game's own chart
 * give-stub does, and skipping it leaves a fresh chart marked already
 * salvaged, so its dig site never appears.
 */
static __inline int dComIfGs_onGetCollectMap(int chartId) {
    dSv_player_map_c* m = dComIfGs_getMap();
    if (!m || !WWHD_CHART_ID_VALID(chartId))
        return 0;
    dSv_map_onGetMap(m, chartId - 1);
    dSv_map_offOpenMap(m, chartId - 1);
    dSv_map_offCompleteMap(m, chartId - 1);
    return 1;
}

/** [V] Mark chart 1..61 opened. Returns non-zero on success. */
static __inline int dComIfGs_onOpenCollectMap(int chartId) {
    dSv_player_map_c* m = dComIfGs_getMap();
    if (!m || !WWHD_CHART_ID_VALID(chartId))
        return 0;
    dSv_map_onOpenMap(m, chartId - 1);
    return 1;
}

/** [V] Mark chart 1..61's treasure salvaged. Returns non-zero on success. */
static __inline int dComIfGs_onCompleteCollectMap(int chartId) {
    dSv_player_map_c* m = dComIfGs_getMap();
    if (!m || !WWHD_CHART_ID_VALID(chartId))
        return 0;
    dSv_map_onCompleteMap(m, chartId - 1);
    return 1;
}

/** [V] Charts collected, 0..49. */
static __inline int dComIfGs_getCollectMapNum(void) {
    return dSv_map_getCollectMapNum(dComIfGs_getMap());
}

/** [V] Has Triforce Chart 1..8 been deciphered. */
static __inline int dComIfGs_isCollectMapTriforce(int no) {
    return no >= 1 && no <= WWHD_TRIFORCE_MAP_MAX &&
           dSv_map_isTriforce(dComIfGs_getMap(), no - 1);
}

/** [V] Decipher Triforce Chart 1..8. Returns non-zero on success. */
static __inline int dComIfGs_onCollectMapTriforce(int no) {
    dSv_player_map_c* m = dComIfGs_getMap();
    if (!m || no < 1 || no > WWHD_TRIFORCE_MAP_MAX)
        return 0;
    dSv_map_onTriforce(m, no - 1);
    return 1;
}

/** [V] Is Triforce SHARD 1..8 held. A shard is not a chart: the shard bits are
 *  in the collect bank, the chart bits in the map bank. */
static __inline int dComIfGs_isTriforce(int no) {
    return no >= 1 && no <= WWHD_TRIFORCE_MAP_MAX &&
           dSv_collect_isTriforce(dComIfGs_getCollect(), no - 1);
}

/** [V] Triforce shards held, 0..8. */
static __inline int dComIfGs_getTriforceNum(void) {
    return dSv_collect_getTriforceNum(dComIfGs_getCollect());
}

/* --- the sea chart itself ---------------------------------------------- */

/** [V] Is sea sector 0..48 drawn on the sea chart. Takes a grid number, NOT a
 *  chart id. "Arrive" is the GameCube name; the bit is the chart reveal. */
static __inline int dComIfGs_isSaveArriveGrid(int gridNo) {
    return dSv_map_isArriveGrid(dComIfGs_getMap(), gridNo);
}

/** [V] Draw sea sector 0..48 on the sea chart. Returns non-zero on success. */
static __inline int dComIfGs_onSaveArriveGrid(int gridNo) {
    dSv_player_map_c* m = dComIfGs_getMap();
    if (!m || !dMap_gridNoValid(gridNo))
        return 0;
    dSv_map_onArriveGrid(m, gridNo);
    return 1;
}

/** [V] Sectors drawn on the sea chart, 0..49. */
static __inline int dComIfGs_getArriveGridNum(void) {
    return dSv_map_getArriveGridNum(dComIfGs_getMap());
}

/** [V] Reveal the whole sea chart. Returns the number of sectors it opened. */
static __inline int dComIfGs_revealAllGrids(void) {
    dSv_player_map_c* m = dComIfGs_getMap();
    int i, n = 0;
    if (!m)
        return 0;
    for (i = 0; i < WWHD_SEA_GRID_MAX; i++) {
        if (!dSv_map_isArriveGrid(m, i)) {
            dSv_map_onArriveGrid(m, i);
            n++;
        }
    }
    return n;
}

/** [V] Has Link sailed into sea sector 0..48. The game records this and never
 *  shows it. */
static __inline int dComIfGs_isVisitGrid(int gridNo) {
    return dSv_map_isVisitGrid(dComIfGs_getMap(), gridNo);
}

/** [V] Sectors sailed into, 0..49. */
static __inline int dComIfGs_getVisitGridNum(void) {
    dSv_player_map_c* m = dComIfGs_getMap();
    int i, n = 0;
    if (!m)
        return 0;
    for (i = 0; i < WWHD_SEA_GRID_MAX; i++)
        if (dSv_map_isVisitGrid(m, i))
            n++;
    return n;
}

/** [V] Record every sector as sailed into. Returns how many it marked. */
static __inline int dComIfGs_visitAllGrids(void) {
    dSv_player_map_c* m = dComIfGs_getMap();
    int i, n = 0;
    if (!m)
        return 0;
    for (i = 0; i < WWHD_SEA_GRID_MAX; i++) {
        if (!dSv_map_isVisitGrid(m, i)) {
            dSv_map_onVisitGrid(m, i);
            n++;
        }
    }
    return n;
}

/* --- event flags -------------------------------------------------------- */

/** [V] Test a packed event id against the persistent bank. */
static __inline int dComIfGs_isEventBit(u16 id) {
    return dSv_event_isEventBit(dComIfGs_getEvent(), id);
}

/** [V] Set a packed event id in the persistent bank. */
static __inline void dComIfGs_onEventBit(u16 id) {
    dSv_event_onEventBit(dComIfGs_getEvent(), id);
}

/** [V] Clear a packed event id in the persistent bank. */
static __inline void dComIfGs_offEventBit(u16 id) {
    dSv_event_offEventBit(dComIfGs_getEvent(), id);
}

/** [V] Read an event register, masked rather than shifted down. */
static __inline u8 dComIfGs_getEventReg(u16 reg) {
    return dSv_event_getEventReg(dComIfGs_getEvent(), reg);
}

/** [V] Write an event register. value must already be shifted into the mask. */
static __inline void dComIfGs_setEventReg(u16 reg, u8 value) {
    dSv_event_setEventReg(dComIfGs_getEvent(), reg, value);
}

/**
 * [V] Fill `info` the way the file select's slot load (0x02721908) fills the
 * live block: dSv_info_c::init (0x025B9A18) - zeros, mDan at stage -1, every
 * zone at room -1 - then the slot number into mDataNum, the twenty packed runs
 * of card_to_memory (0x025BA7B0), the config byte that routine forces to 2,
 * and the HD Triforce preset that 0x02721880 re-applies after every load.
 * `slot` is one slot of a cking.sav image, from dSv_saveFileSlot(). The
 * return place inside the result is what dComIfGs_gameStart would warp to.
 */
static __inline void dSv_info_loadSlotImage(dSv_info_c* info, const u8* slot, int slotNo) {
    u8* bytes = (u8*)info;
    u32 i;
    if (!info || !slot)
        return;
    for (i = 0; i < (u32)sizeof(dSv_info_c); i++)
        bytes[i] = 0;
    info->mDan.mStageNo = (s8)-1;
    for (i = 0; i < dSv_ZONE_MAX; i++)
        info->mZone[i].mRoomNo = (s8)-1;
    info->mDataNum = (u8)slotNo;
    dSv_unpackSlot(slot, &info->mSavedata);
    info->mSavedata.mPlayer.mConfig[dSv_CONFIG_FORCED_INDEX] = dSv_CONFIG_FORCED_VALUE;
    info->mSavedata.mPlayer.mMap.mTriforceMap |= (u8)WWHD_TRIFORCE_MAP_PRESET;
}

#endif /* LIBWWHD_D_SAVE_ACCESS_H */
