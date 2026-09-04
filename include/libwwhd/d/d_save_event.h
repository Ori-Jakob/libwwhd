#ifndef LIBWWHD_D_SAVE_EVENT_H
#define LIBWWHD_D_SAVE_EVENT_H

#include "libwwhd/wwhd_types.h"

/**
 * libwwhd - known event flag and register ids.
 *
 * These are save-format identifiers, not addresses: the same id means the same
 * bit in every region, because all three builds write the same save file. That
 * is why there is no map slot here and no porting to do - unlike everything
 * else in libwwhd, a value in this header is region-independent by
 * construction.
 *
 * The encoding is in d_save.h: byte = id >> 8, mask = id & 0xFF. A mask with
 * one bit set is a flag; a wider mask is a record field, and its value is
 * stored already shifted into place.
 *
 * COVERAGE IS DELIBERATELY THIN. WWHD reaches the event bank 1,910 times, but
 * the register ids are overwhelmingly passed in rather than written down: of
 * 292 getEventReg/setEventReg sites only 18 name a literal id, the other 274
 * taking it from a function parameter or a table. So a per-minigame register
 * map is per-actor reverse engineering, not one scan. What is here is what
 * could be attributed to a source file by an assert anchor in the same
 * function - which is the two-sighting rule, since the GameCube source uses
 * the same id in the same file.
 *
 * docs/SUBSYSTEMS.md (Minigames and records) has the method for extending it.
 */

/**
 * [V] Windfall auction. Read and written by the anchored function at
 * d_a_auction.cpp:3327, and by nothing else in the binary.
 */
#define WWHD_EVREG_AUCTION      0x790Fu

/**
 * [V] Salvatore's battleship game. Read and written inside d_a_npc_bs1.cpp,
 * confirmed by an anchor at :1493 in one of the two functions that use it.
 */
#define WWHD_EVREG_BATTLESHIP   0x7F0Fu

/**
 * [V] A shared full-byte register, NOT a minigame score. Six unrelated actors
 * read it - d_a_oq.cpp among them - and dSv_event_c::init presets it, which is
 * what rules out a per-minigame meaning. Named for what is known.
 */
#define WWHD_EVREG_SHARED_7E    0x7EFFu

/** [V] The two registers dSv_event_c::init presets, and their values. */
#define WWHD_EVREG_INIT_7E      0x7EFFu
#define WWHD_EVREG_INIT_7E_VAL  0x0Eu
#define WWHD_EVREG_INIT_BE      0xBEFFu
#define WWHD_EVREG_INIT_BE_VAL  0x14u

/* ========================================================================
 * Figurine pedestals - d_a_dai_item.cpp
 *
 * [V] Forty full-byte registers, one per pedestal, holding which figurine is
 * displayed on it. They sit in a table of exactly 40 entries running strictly
 * downward from event byte 0xF8 to 0xD1 and terminated by a zero word, indexed
 * by five functions immediately before the d_a_dai_item.cpp:569 anchor.
 *
 * Because the run is contiguous and descending, the table is arithmetic and
 * libwwhd does not need its address.
 * ===================================================================== */

/** [V] Pedestals with a register. */
#define WWHD_PEDESTAL_MAX 40

/** [V] Register for pedestal 0..39, or 0 when the index is out of range. */
static __inline u16 dSv_event_pedestalReg(int pedestal) {
    if (pedestal < 0 || pedestal >= WWHD_PEDESTAL_MAX)
        return 0;
    return (u16)(0xF8FFu - ((unsigned)pedestal << 8));
}

/* ========================================================================
 * Salvage rewards - d_salvage.cpp
 *
 * [V] Sixteen single-bit event flags, indexed by salvage id, read by the
 * salvage scanner and set on completion when the reward is neither a chart nor
 * an ocean bit. The table is anchored at d_salvage.cpp:139.
 *
 * Unlike the pedestal run these ids are irregular, so the values are carried
 * rather than computed. They are save ids, so the array is region-independent.
 * ===================================================================== */

/** [V] Entries in the salvage flag table. */
#define WWHD_SALVAGE_FLAG_MAX 16

/** [V] The salvage reward flags, in table order. */
static const u16 wwhd_salvageFlags[WWHD_SALVAGE_FLAG_MAX] = {
    0x2080u, 0x2004u, 0x2002u, 0x2804u, 0x2802u, 0x2801u, 0x2980u, 0x2940u,
    0x3B01u, 0x3C80u, 0x3C40u, 0x3C20u, 0x3C10u, 0x3C08u, 0x3C04u, 0x3C02u
};

/** [V] Salvage reward flag for a salvage id, or 0 when out of range. */
static __inline u16 dSv_event_salvageFlag(int salvageId) {
    if (salvageId < 0 || salvageId >= WWHD_SALVAGE_FLAG_MAX)
        return 0;
    return wwhd_salvageFlags[salvageId];
}

/** [V] Set on completing a salvage whose reward is a chart's treasure.
 *  The one literal the salvage completion path writes. */
#define WWHD_EVFLAG_SALVAGE_CHART 0x3E02u

#endif /* LIBWWHD_D_SAVE_EVENT_H */
