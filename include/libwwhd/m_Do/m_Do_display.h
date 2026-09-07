#ifndef LIBWWHD_M_DO_DISPLAY_H
#define LIBWWHD_M_DO_DISPLAY_H

#include "libwwhd/wwhd_types.h"
#include "libwwhd/wwhd_map.h"
#include "libwwhd/wwhd_region.h"

/**
 * Which screen the game is playing on.
 *
 * The Wii U build lets the player move the game between the TV and the GamePad,
 * and the choice is a single word in the display manager - see wwhd_map
 * uiDisplayMgr for how it was found and what else lives in that object.
 *
 * 0 and 3 are two further targets the setter accepts and were not chased, so
 * they are reported as-is rather than folded into TV or DRC. Anything outside
 * 0..3 is reported as WWHD_DISPLAY_MODE_UNKNOWN, which is also what a caller
 * gets before the manager exists.
 *
 * IMPORTANT, from hardware: normal play on the TV reads **3**, not 1. The 1 and
 * 2 written by ::StateID_ChangeToTvMode and ::StateID_ChangeToDrcMode are what
 * those transitions pass; 3 is what the field settles at in ordinary play. So
 * test for DRC rather than for TV - which is what wwhd_isTvShowingGame() does -
 * and do not assume 1 means "on the TV right now".
 *
 * Off-TV play does park the field at 2 rather than only passing through it:
 * screen routing that keys on this behaves correctly in use, which it could not
 * if 2 were transient. That is behavioural evidence, not an instrumented read.
 */
#define WWHD_DISPLAY_MODE_UNKNOWN 0xFFFFFFFFu
#define WWHD_DISPLAY_MODE_TV      1u  /* [V] playing on the TV      */
#define WWHD_DISPLAY_MODE_DRC     2u  /* [V] playing on the GamePad */

/** [V] Display mode word, relative to the manager. USA and EUR agree. */
#define WWHD_DISPLAY_MODE_OFS 0x1D0

/** [V] The display manager, or NULL before it exists. */
static __inline void* wwhd_getDisplayMgr(void) {
    u32 p;
    if (!wwhd_regionResolved)
        return (void*)0;
    p = *WWHD_AT_DATA(u32, wwhd_map->uiDisplayMgr);
    return p ? WWHD_AT(void, p) : (void*)0;
}

/** [V] WWHD_DISPLAY_MODE_*, or WWHD_DISPLAY_MODE_UNKNOWN if it cannot be read. */
static __inline u32 wwhd_getDisplayMode(void) {
    const void* mgr = wwhd_getDisplayMgr();
    u32 mode;
    if (!mgr)
        return WWHD_DISPLAY_MODE_UNKNOWN;
    mode = *(const u32*)((const u8*)mgr + WWHD_DISPLAY_MODE_OFS);
    return mode <= 3u ? mode : WWHD_DISPLAY_MODE_UNKNOWN;
}

/** [V] Is the game being played on the GamePad rather than the TV? */
static __inline int wwhd_isPlayingOnGamePad(void) {
    return wwhd_getDisplayMode() == WWHD_DISPLAY_MODE_DRC;
}

/**
 * [V] Is the TV worth drawing on?
 *
 * Deliberately true when the mode cannot be read, so a caller that gates
 * drawing on this keeps drawing rather than going blank on an unknown build.
 */
static __inline int wwhd_isTvShowingGame(void) {
    return wwhd_getDisplayMode() != WWHD_DISPLAY_MODE_DRC;
}

#endif /* LIBWWHD_M_DO_DISPLAY_H */
