#ifndef LIBWWHD_D_A_TITLE_H
#define LIBWWHD_D_A_TITLE_H

#include "libwwhd/wwhd_types.h"
#include "libwwhd/wwhd_region.h"
#include "libwwhd/f_op/f_op_actor.h"

/**
 * libwwhd - the title logo and PRESS START prompt (d_a_title.cpp)
 *
 * An actor inside the opening scene, not a scene of its own. Its profile is
 * at 0x101D2418 in USA and EUR and 0x101D2438 in JAP, identical apart from
 * the relocated pointers: process name 0x01BC at +0x08, object size 0x3BC at
 * +0x10 and the method table at +0x24 (USA: create 0x024B8F44, delete
 * 0x024B895C, execute 0x024B87A4, isDelete 0x024B9130, draw 0x024B7CBC).
 *
 * [V] create requests the "Tlogo" archive through the per-actor resource
 * phase (0x02520460) and returns that status until it reads complete (4);
 * only then does it allocate the 0x3F0-byte logo object it keeps at +0x3B4
 * and clear the "started" byte at +0x3B8.
 *
 * [V] execute reads the logo object's state word at +0x6C:
 *   1  the prompt is up and Start is accepted - the pad word is tested here
 *      and 0x024B7CE4 moves the state to 2 on a press;
 *   3  it asks the stage scene for scene 9, the file select, and sets +0x3B8.
 * Anything below 1 is the logo still loading or animating. The game accepts
 * Start in state 1 and nowhere else, which makes that state the safe moment
 * to start a game from the title by other means: before it the loader thread
 * is still fetching the title's own archives, and a play scene created then
 * asserts d_stage.cpp:4871 stageRsrc != 0 in its create-phase slot 4.
 */
#define WWHD_PROC_TITLE             0x01BC
#define WWHD_DATITLE_OFF_LOGO       0x3B4
#define WWHD_DATITLE_OFF_STARTED    0x3B8
#define WWHD_TLOGO_OFF_STATE        0x6C
#define WWHD_TLOGO_STATE_WAIT_START 1

#ifdef WWHD_ENABLE_GAME_CALLS
/** [V] The title actor, or NULL when it is not around. A process search,
 *  with the same caveats as daShip_searchByProc(). */
static __inline fopAc_ac_c* daTitle_searchByProc(void) {
    typedef void* (*fopAcM_searchByID_t)(void* tag, u16* procName);
    fopAcM_searchByID_t search;
    u16 key;
    if (!wwhd_textResolved || !wwhd_regionResolved)
        return (fopAc_ac_c*)0;
    search = WWHD_FN(fopAcM_searchByID_t, wwhd_map->fopAcM_searchByID);
    key = WWHD_PROC_TITLE;
    return (fopAc_ac_c*)search(WWHD_AT_TEXT(void, wwhd_map->tagProcSearch), &key);
}

/** [V] Non-zero while the title screen shows PRESS START and would accept it. */
static __inline int daTitle_isWaitingForStart(void) {
    fopAc_ac_c* title = daTitle_searchByProc();
    wwhd_gptr_t logo;
    if (!title)
        return 0;
    logo = *(wwhd_gptr_t*)((u8*)title + WWHD_DATITLE_OFF_LOGO);
    if (!logo)
        return 0;
    return *WWHD_AT(s32, logo + WWHD_TLOGO_OFF_STATE) == WWHD_TLOGO_STATE_WAIT_START;
}
#endif /* WWHD_ENABLE_GAME_CALLS */

#endif /* LIBWWHD_D_A_TITLE_H */
