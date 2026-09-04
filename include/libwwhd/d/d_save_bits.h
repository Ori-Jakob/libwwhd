#ifndef LIBWWHD_D_SAVE_BITS_H
#define LIBWWHD_D_SAVE_BITS_H

#include "libwwhd/wwhd_types.h"
#include "libwwhd/d/d_save.h"

/**
 * libwwhd - save bit-bank arithmetic (d_save.cpp)
 *
 * Every bank in the save block is addressed by a small integer through a
 * three-line shift-and-mask routine, and each of those routines was read out of
 * WWHD to get the bank offset that d_save.h records. Reimplementing them here
 * rather than calling them keeps the whole thing pure: no text delta, no
 * WWHD_ENABLE_GAME_CALLS, and the layout test can replay each one.
 *
 * These take the bank POINTER explicitly, so they need nothing from the game
 * info root. The helpers that go and find the bank are in d_save_access.h.
 *
 * The one bank that is called rather than reimplemented is the dSv_info_c-level
 * switch/item routing, which is not arithmetic - it picks between the memory,
 * dungeon and zone banks by id range and needs the room and zone tables to do
 * it. Those stay as map slots.
 */

/** [V] Quarter-hearts per heart container. */
#define WWHD_HEART_QUARTERS 4
/** [V] Inventory slot count. */
#define WWHD_ITEM_SLOTS 21
/** [V] Empty inventory slot. */
#define WWHD_ITEM_NONE  0xFF

/** [V] no < 128. Mirrors dSv_memBit_c::onSwitch. */
static __inline void dSv_memBit_onSwitch(dSv_memBit_c* b, int no) {
    if (b && no >= 0 && no < 128)
        b->mSwitch[no >> 5] |= (u32)1 << (no & 31);
}

/** [V] no < 128. */
static __inline void dSv_memBit_offSwitch(dSv_memBit_c* b, int no) {
    if (b && no >= 0 && no < 128)
        b->mSwitch[no >> 5] &= ~((u32)1 << (no & 31));
}

/** [V] no < 128. */
static __inline int dSv_memBit_isSwitch(const dSv_memBit_c* b, int no) {
    if (!b || no < 0 || no >= 128)
        return 0;
    return (b->mSwitch[no >> 5] & ((u32)1 << (no & 31))) != 0;
}

/** [V] no < 32. */
static __inline void dSv_memBit_onItem(dSv_memBit_c* b, int no) {
    if (b && no >= 0 && no < 32)
        b->mItem[0] |= (u32)1 << (no & 31);
}

/** [V] no < 32. */
static __inline int dSv_memBit_isItem(const dSv_memBit_c* b, int no) {
    if (!b || no < 0 || no >= 32)
        return 0;
    return (b->mItem[0] & ((u32)1 << (no & 31))) != 0;
}

/* ========================================================================
 * Charts, collectables, ocean and event banks
 *
 * The game's own routines index charts from ZERO; the dComIfGs_ wrappers that
 * callers actually use index them from ONE. libwwhd keeps both layers with the
 * same split, so a helper here takes what the WWHD routine takes and the
 * chart-id helpers in d_save_access.h take what the game's callers pass. Mixing
 * them silently reads the neighbouring chart.
 * ===================================================================== */

/** [V] Bits in each of the three chart banks. */
#define WWHD_CHART_BITS 128

/** [V] A valid 1-based chart id, as the dComIfGs_ layer numbers them. */
#define WWHD_CHART_ID_VALID(id) ((id) >= 1 && (id) <= dSv_CHART_MAX)

/**
 * [V] Charts 1..51 except 35 and 36 - the 49 that mark a salvage point, and
 * what getCollectMapNum counts. Charts 52..61 are the ones with no dig site.
 */
static __inline int dSv_map_chartIsSalvage(int chartId) {
    return chartId >= 1 && chartId <= 51 && chartId != 35 && chartId != 36;
}

/** [V] Triforce Charts, 1..8. */
#define WWHD_TRIFORCE_MAP_MAX 8

/**
 * [V] Triforce Chart n IS chart id n. Tingle's decipher loop at 0x022E9668
 * walks ids 1..8 against mTriforceMap bit n-1, and his two "anything left to
 * decipher" checks at 0x022EB10C and 0x022EB1AC walk the same eight. So the
 * Triforce Charts are the first eight rows of the chart banks, and the
 * treasure and special charts follow.
 */
static __inline int dSv_map_chartIsTriforce(int chartId) {
    return chartId >= 1 && chartId <= WWHD_TRIFORCE_MAP_MAX;
}

/**
 * [P] Chart names by chart id 1..61, from the GameCube item table: the chart
 * give stubs sit on items 0xC2..0xFE with chartId = 0xFF - itemNo, so the
 * names follow the item numbers. The binary has no chart-name strings, which
 * is why these are [P]; two verified facts pin the list down. Triforce Charts
 * 1..8 are ids 1..8 (dSv_map_chartIsTriforce above), and the twelve ids the
 * game's own salvage predicate excludes - 35, 36 and 52..61 - are exactly the
 * twelve special charts here. In HD only Triforce Charts 1, 3 and 5 exist as
 * items; 2, 4, 6, 7 and 8 are found as shards and start out deciphered
 * (WWHD_TRIFORCE_MAP_PRESET).
 */
static const char* const wwhd_chartNames[dSv_CHART_MAX] = {
    "Triforce Chart 1",  "Triforce Chart 2",  "Triforce Chart 3",  "Triforce Chart 4",
    "Triforce Chart 5",  "Triforce Chart 6",  "Triforce Chart 7",  "Triforce Chart 8",
    "Treasure Chart 11", "Treasure Chart 15", "Treasure Chart 30", "Treasure Chart 20",
    "Treasure Chart 5",  "Treasure Chart 23", "Treasure Chart 31", "Treasure Chart 33",
    "Treasure Chart 2",  "Treasure Chart 38", "Treasure Chart 39", "Treasure Chart 24",
    "Treasure Chart 6",  "Treasure Chart 12", "Treasure Chart 35", "Treasure Chart 1",
    "Treasure Chart 29", "Treasure Chart 34", "Treasure Chart 18", "Treasure Chart 16",
    "Treasure Chart 28", "Treasure Chart 4",  "Treasure Chart 3",  "Treasure Chart 40",
    "Treasure Chart 10", "Treasure Chart 14",
    "Tingle's Chart",    "Ghost Ship Chart",
    "Treasure Chart 9",  "Treasure Chart 22", "Treasure Chart 36", "Treasure Chart 17",
    "Treasure Chart 25", "Treasure Chart 37", "Treasure Chart 8",  "Treasure Chart 26",
    "Treasure Chart 41", "Treasure Chart 19", "Treasure Chart 32", "Treasure Chart 13",
    "Treasure Chart 21", "Treasure Chart 27", "Treasure Chart 7",
    "IN-credible Chart", "Octo Chart",        "Great Fairy Chart", "Island Hearts Chart",
    "Sea Hearts Chart",  "Secret Cave Chart", "Light Ring Chart",  "Platform Chart",
    "Beedle's Chart",    "Submarine Chart",
};

/** [P] The name of chart 1..61, or "?" for anything else. */
static __inline const char* wwhd_chartName(int chartId) {
    return WWHD_CHART_ID_VALID(chartId) ? wwhd_chartNames[chartId - 1] : "?";
}

/**
 * [V] WWHD starts a file with five Triforce Charts already deciphered:
 * 0xEA is bits 1,3,5,6,7, i.e. charts 2,4,6,7 and 8. Only charts 1, 3 and 5
 * still need Tingle, which is why the chart list bounds its Triforce category
 * at three. GameCube starts this byte at zero.
 */
#define WWHD_TRIFORCE_MAP_PRESET 0xEAu

/** [V] no < 128, 0-based. Mirrors dSv_player_map_c::onGetMap. */
static __inline void dSv_map_onGetMap(dSv_player_map_c* m, int no) {
    if (m && no >= 0 && no < WWHD_CHART_BITS)
        m->mGetMap[no >> 5] |= (u32)1 << (no & 31);
}

/** [V] no < 128, 0-based. The game never clears this bit - picking a chart up
 *  is permanent - so this exists for an editor, mirroring the Open/Complete
 *  off routines it does have. */
static __inline void dSv_map_offGetMap(dSv_player_map_c* m, int no) {
    if (m && no >= 0 && no < WWHD_CHART_BITS)
        m->mGetMap[no >> 5] &= ~((u32)1 << (no & 31));
}

/** [V] no < 128, 0-based. */
static __inline int dSv_map_isGetMap(const dSv_player_map_c* m, int no) {
    if (!m || no < 0 || no >= WWHD_CHART_BITS)
        return 0;
    return (m->mGetMap[no >> 5] & ((u32)1 << (no & 31))) != 0;
}

/** [V] no < 128, 0-based. */
static __inline void dSv_map_onOpenMap(dSv_player_map_c* m, int no) {
    if (m && no >= 0 && no < WWHD_CHART_BITS)
        m->mOpenMap[no >> 5] |= (u32)1 << (no & 31);
}

/** [V] no < 128, 0-based. */
static __inline void dSv_map_offOpenMap(dSv_player_map_c* m, int no) {
    if (m && no >= 0 && no < WWHD_CHART_BITS)
        m->mOpenMap[no >> 5] &= ~((u32)1 << (no & 31));
}

/** [V] no < 128, 0-based. */
static __inline int dSv_map_isOpenMap(const dSv_player_map_c* m, int no) {
    if (!m || no < 0 || no >= WWHD_CHART_BITS)
        return 0;
    return (m->mOpenMap[no >> 5] & ((u32)1 << (no & 31))) != 0;
}

/** [V] no < 128, 0-based. */
static __inline void dSv_map_onCompleteMap(dSv_player_map_c* m, int no) {
    if (m && no >= 0 && no < WWHD_CHART_BITS)
        m->mCompleteMap[no >> 5] |= (u32)1 << (no & 31);
}

/** [V] no < 128, 0-based. */
static __inline void dSv_map_offCompleteMap(dSv_player_map_c* m, int no) {
    if (m && no >= 0 && no < WWHD_CHART_BITS)
        m->mCompleteMap[no >> 5] &= ~((u32)1 << (no & 31));
}

/** [V] no < 128, 0-based. */
static __inline int dSv_map_isCompleteMap(const dSv_player_map_c* m, int no) {
    if (!m || no < 0 || no >= WWHD_CHART_BITS)
        return 0;
    return (m->mCompleteMap[no >> 5] & ((u32)1 << (no & 31))) != 0;
}

/** [V] Triforce Chart deciphered. no < 8, 0-based. */
static __inline void dSv_map_onTriforce(dSv_player_map_c* m, int no) {
    if (m && no >= 0 && no < WWHD_TRIFORCE_MAP_MAX)
        m->mTriforceMap |= (u8)(1 << no);
}

/** [V] no < 8, 0-based. */
static __inline void dSv_map_offTriforce(dSv_player_map_c* m, int no) {
    if (m && no >= 0 && no < WWHD_TRIFORCE_MAP_MAX)
        m->mTriforceMap &= (u8)~(1 << no);
}

/** [V] no < 8, 0-based. */
static __inline int dSv_map_isTriforce(const dSv_player_map_c* m, int no) {
    if (!m || no < 0 || no >= WWHD_TRIFORCE_MAP_MAX)
        return 0;
    return (m->mTriforceMap & (u8)(1 << no)) != 0;
}

/**
 * [V] Charts collected, counted the way the game counts them: 1..51 except 35
 * and 36, so the total tops out at 49 rather than dSv_CHART_MAX.
 */
static __inline int dSv_map_getCollectMapNum(const dSv_player_map_c* m) {
    int id, n = 0;
    if (!m)
        return 0;
    for (id = 1; id <= 51; id++)
        if (dSv_map_chartIsSalvage(id) && dSv_map_isGetMap(m, id - 1))
            n++;
    return n;
}

/** [V] Sea-square bit. grid < 49, bit < 8. Mirrors onFmapBit. */
static __inline void dSv_map_onFmapBit(dSv_player_map_c* m, int grid, int bit) {
    if (m && grid >= 0 && grid < dSv_FMAP_GRID_MAX && bit >= 0 && bit < 8)
        m->mFmapBits[grid] |= (u8)(1 << bit);
}

/** [V] grid < 49, bit < 8. */
static __inline int dSv_map_isFmapBit(const dSv_player_map_c* m, int grid, int bit) {
    if (!m || grid < 0 || grid >= dSv_FMAP_GRID_MAX || bit < 0 || bit >= 8)
        return 0;
    return (m->mFmapBits[grid] & (u8)(1 << bit)) != 0;
}

/**
 * [V] mFmapBits bit 0: the sector is DRAWN on the sea chart. It is what the
 * chart menu reads - 0x02681400 walks all 49 sectors, shows each pane on this
 * bit and counts them - and what the sea-chart UI sets through 0x0269A968 ->
 * 0x0269A91C -> onFmapBit(grid, 0) at 0x025B8484, besides the three presets
 * the initialiser makes. The GameCube name onArriveGrid survives in the
 * helpers below; do not read "arrive" as "sailed into", which is bit 1.
 */
#define WWHD_FMAP_BIT_CHART 0
/**
 * [V] mFmapBits bit 1: Link has SAILED INTO the sector. Set by the sea-stage
 * position tracker at 0x02590004 whenever his grid square changes, through
 * the routine the GameCube called onArriveGridForAgb (0x025B85CC). Nothing in
 * this build reads it back: a record the game keeps and never shows.
 */
#define WWHD_FMAP_BIT_VISITED 1
/* The earlier names. Same bits; kept so nothing else moves. */
#define WWHD_FMAP_BIT_ARRIVE     WWHD_FMAP_BIT_CHART
#define WWHD_FMAP_BIT_ARRIVE_AGB WWHD_FMAP_BIT_VISITED

/** [V] grid < 49, bit < 8. The game never clears a sector bit; this is for an
 *  editor, and mirrors onFmapBit with the mask inverted. */
static __inline void dSv_map_offFmapBit(dSv_player_map_c* m, int grid, int bit) {
    if (m && grid >= 0 && grid < dSv_FMAP_GRID_MAX && bit >= 0 && bit < 8)
        m->mFmapBits[grid] &= (u8)~(1 << bit);
}

/** [V] Draw this sector on the sea chart. */
static __inline void dSv_map_onArriveGrid(dSv_player_map_c* m, int grid) {
    dSv_map_onFmapBit(m, grid, WWHD_FMAP_BIT_CHART);
}

/** [V] Take this sector off the sea chart again. */
static __inline void dSv_map_offArriveGrid(dSv_player_map_c* m, int grid) {
    dSv_map_offFmapBit(m, grid, WWHD_FMAP_BIT_CHART);
}

/** [V] Is this sector drawn on the sea chart. */
static __inline int dSv_map_isArriveGrid(const dSv_player_map_c* m, int grid) {
    return dSv_map_isFmapBit(m, grid, WWHD_FMAP_BIT_CHART);
}

/** [V] Has Link sailed into this sector. */
static __inline int dSv_map_isVisitGrid(const dSv_player_map_c* m, int grid) {
    return dSv_map_isFmapBit(m, grid, WWHD_FMAP_BIT_VISITED);
}

/** [V] Record that Link sailed into this sector. */
static __inline void dSv_map_onVisitGrid(dSv_player_map_c* m, int grid) {
    dSv_map_onFmapBit(m, grid, WWHD_FMAP_BIT_VISITED);
}

/** [V] Forget that Link sailed into this sector. */
static __inline void dSv_map_offVisitGrid(dSv_player_map_c* m, int grid) {
    dSv_map_offFmapBit(m, grid, WWHD_FMAP_BIT_VISITED);
}

/** [V] Sectors drawn on the sea chart, 0..49. */
static __inline int dSv_map_getArriveGridNum(const dSv_player_map_c* m) {
    int i, n = 0;
    for (i = 0; i < dSv_FMAP_GRID_MAX; i++)
        if (dSv_map_isArriveGrid(m, i))
            n++;
    return n;
}

/* --- collect ----------------------------------------------------------- */

/** [V] idx indexes the byte, bit < 8. Mirrors onCollect. */
static __inline void dSv_collect_onCollect(dSv_player_collect_c* c, int idx, int bit) {
    if (c && idx >= 0 && idx < 8 && bit >= 0 && bit < 8)
        c->mCollect[idx] |= (u8)(1 << bit);
}

/** [V] idx < 8, bit < 8. */
static __inline int dSv_collect_isCollect(const dSv_player_collect_c* c, int idx, int bit) {
    if (!c || idx < 0 || idx >= 8 || bit < 0 || bit >= 8)
        return 0;
    return (c->mCollect[idx] & (u8)(1 << bit)) != 0;
}

/** [V] Triforce SHARDS held. no < 8, 0-based. */
static __inline void dSv_collect_onTriforce(dSv_player_collect_c* c, int no) {
    if (c && no >= 0 && no < WWHD_TRIFORCE_MAP_MAX)
        c->mTriforce |= (u8)(1 << no);
}

/** [V] no < 8, 0-based. */
static __inline int dSv_collect_isTriforce(const dSv_player_collect_c* c, int no) {
    if (!c || no < 0 || no >= WWHD_TRIFORCE_MAP_MAX)
        return 0;
    return (c->mTriforce & (u8)(1 << no)) != 0;
}

/** [V] Shards held, 0..8. Mirrors getTriforceNum. */
static __inline int dSv_collect_getTriforceNum(const dSv_player_collect_c* c) {
    int i, n = 0;
    for (i = 0; i < WWHD_TRIFORCE_MAP_MAX; i++)
        if (dSv_collect_isTriforce(c, i))
            n++;
    return n;
}

/* --- ocean salvage bits ------------------------------------------------ */

/** [V] grid < 50, bit < 16. Mirrors dSv_ocean_c::onOceanSvBit. */
static __inline void dSv_ocean_onSvBit(dSv_ocean_c* o, int grid, int bit) {
    if (o && grid >= 0 && grid < dSv_OCEAN_GRID_MAX && bit >= 0 && bit < dSv_OCEAN_BIT_MAX)
        o->mSvBits[grid] |= (u16)(1 << bit);
}

/** [V] grid < 50, bit < 16. */
static __inline int dSv_ocean_isSvBit(const dSv_ocean_c* o, int grid, int bit) {
    if (!o || grid < 0 || grid >= dSv_OCEAN_GRID_MAX || bit < 0 || bit >= dSv_OCEAN_BIT_MAX)
        return 0;
    return (o->mSvBits[grid] & (u16)(1 << bit)) != 0;
}

/* --- event flags and registers ----------------------------------------- */

/**
 * Event ids are packed, not indexed: byte = id >> 8, mask = id & 0xFF. There is
 * no bounds check in the game and none here - every u16 is a legal id, because
 * the byte selector is already limited to 0..255 by the shift.
 */

/** [V] Byte an event id addresses. */
#define WWHD_EVENT_BYTE(id) (((u32)(id) >> 8) & 0xFFu)
/** [V] Mask an event id carries. */
#define WWHD_EVENT_MASK(id) ((u8)((id) & 0xFF))

/** [V] Mirrors dSv_event_c::onEventBit. */
static __inline void dSv_event_onEventBit(dSv_event_c* e, u16 id) {
    if (e)
        e->mFlags[WWHD_EVENT_BYTE(id)] |= WWHD_EVENT_MASK(id);
}

/** [V] Mirrors offEventBit. */
static __inline void dSv_event_offEventBit(dSv_event_c* e, u16 id) {
    if (e)
        e->mFlags[WWHD_EVENT_BYTE(id)] &= (u8)~WWHD_EVENT_MASK(id);
}

/** [V] Mirrors isEventBit. Non-zero when EVERY bit in the mask is set. */
static __inline int dSv_event_isEventBit(const dSv_event_c* e, u16 id) {
    if (!e)
        return 0;
    return (e->mFlags[WWHD_EVENT_BYTE(id)] & WWHD_EVENT_MASK(id)) != 0;
}

/**
 * [V] Mirrors getEventReg. The result is MASKED, not shifted down: the game
 * compares it against equally pre-shifted constants, so shifting it here would
 * break every comparison a caller ports over from the game.
 */
static __inline u8 dSv_event_getEventReg(const dSv_event_c* e, u16 reg) {
    if (!e)
        return 0;
    return (u8)(e->mFlags[WWHD_EVENT_BYTE(reg)] & WWHD_EVENT_MASK(reg));
}

/** [V] Mirrors setEventReg. value must already be shifted into the mask. */
static __inline void dSv_event_setEventReg(dSv_event_c* e, u16 reg, u8 value) {
    if (e)
        e->mFlags[WWHD_EVENT_BYTE(reg)] =
            (u8)((e->mFlags[WWHD_EVENT_BYTE(reg)] & ~WWHD_EVENT_MASK(reg)) | value);
}

#endif /* LIBWWHD_D_SAVE_BITS_H */
