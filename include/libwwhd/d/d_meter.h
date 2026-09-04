#ifndef LIBWWHD_D_METER_H
#define LIBWWHD_D_METER_H

#include "libwwhd/wwhd_types.h"
#include "libwwhd/wwhd_region.h"
#include "libwwhd/f_op/f_op_actor.h"
#include "libwwhd/d/d_com_inf_game.h"

/**
 * libwwhd - the HUD meter process (d_meter.cpp)
 *
 * One process, name WWHD_PROC_METER, owns the whole HUD: hearts, magic, rupees,
 * the item icons. Everything below was read from its create (0x02595C6C, the
 * function asserting "i_Meter->heap != (0)") and its draw (0x02593B10, where
 * this game does the meter's per-frame work; the rupee part is the routine at
 * 0x0259C454 it calls). EUR create is 0x02595C7C, JAP 0x02595C70.
 *
 * THE RUPEE COUNT ON SCREEN LIVES HERE, NOT IN THE SAVE. The digits are drawn
 * from mRupeeDisp. The rupee routine moves it only when a pending delta arrives
 * (dComIfGp_getItemRupeeCount): it folds the delta into the save value, sets
 * mRupeeCountRemain = total - mRupeeDisp, and then steps mRupeeDisp toward the
 * total by 1, 10 or 100 a frame (0x025931A8 picks the step). A save value
 * written directly, with no delta, is therefore not shown until the next pickup
 * or purchase. docs/SUBSYSTEMS.md, "Making a modifier stick", shape 4.
 *
 * REACHING IT. The meter is a process, not a global: nothing in the game keeps
 * a pointer to it, and its create publishes only its screen objects. The game
 * finds it the way the RIDE camera finds the boat - a process-name search - and
 * dMeter_searchByProc() below is that search, under WWHD_ENABLE_GAME_CALLS.
 */

/* 0x3044 bytes: the size in the profile record (USA 0x101EA114) and the last
 * field the draw touches, at +0x3040, agree. What follows is a verified PREFIX
 * with every unknown region left explicit. */
typedef struct dMeter_c {
    /* 0x0000 */ u8          _unk_0000[0x100];          /* [?] process header  */
    /* 0x0100 */ wwhd_gptr_t mpHeap;                    /* [V] "i_Meter->heap":
                                                         *     asserted by create,
                                                         *     made current by
                                                         *     draw and delete  */
    /* 0x0104 */ u8          _unk_0104[0x1A62 - 0x104]; /* [?] pane and texture
                                                         *     state             */
    /* 0x1A62 */ u16         mWalletSizeShown;          /* [V] the wallet the
                                                         *     digits are laid
                                                         *     out for. Create
                                                         *     seeds it from
                                                         *     mWalletSize; the
                                                         *     rupee routine
                                                         *     re-lays the digits
                                                         *     when it differs  */
    /* 0x1A64 */ u8          _unk_1A64[0x3000 - 0x1A64];/* [?] */
    /* 0x3000 */ s32         mRupeeCountRemain;         /* [P] rupees still to be
                                                         *     counted onto the
                                                         *     display, signed; 0
                                                         *     while idle. One
                                                         *     function touches
                                                         *     it (0x0259C454),
                                                         *     but its whole
                                                         *     lifecycle is there */
    /* 0x3004 */ u32         _unk_3004;                 /* [?] */
    /* 0x3008 */ u32         mFlags;                    /* [V] per-frame state
                                                         *     bits: draw rebuilds
                                                         *     them, the rupee
                                                         *     routine tests
                                                         *     0x40 and 0x4000  */
    /* 0x300C */ u16         mRupeeDisp;                /* [V] the count on
                                                         *     screen. Create
                                                         *     seeds it from
                                                         *     mRupee, the rupee
                                                         *     routine steps it,
                                                         *     and both mirror it
                                                         *     to play.mRupeeDisp */
    /* 0x300E */ u8          _unk_300E[0x3044 - 0x300E];/* [?] the life and magic
                                                         *     display counters
                                                         *     are in here      */
} dMeter_c;
WWHD_ASSERT_OFFSET(dMeter_c, mpHeap,            0x0100);
WWHD_ASSERT_OFFSET(dMeter_c, mWalletSizeShown,  0x1A62);
WWHD_ASSERT_OFFSET(dMeter_c, mRupeeCountRemain, 0x3000);
WWHD_ASSERT_OFFSET(dMeter_c, mFlags,            0x3008);
WWHD_ASSERT_OFFSET(dMeter_c, mRupeeDisp,        0x300C);
WWHD_ASSERT_SIZE  (dMeter_c, 0x3044);

/**
 * [V] Put a rupee count on screen now: the meter's own digits, its count-up
 * remainder (so no animation runs back toward what it was showing), and the
 * copy in play that shops and the "count settled" test at 0x02263234 read.
 * Does not touch the save value; pair it with dSv_setRupee(). Either pointer
 * may be NULL, in which case that half is skipped.
 */
static __inline void dMeter_setRupeeDisplay(dMeter_c* meter, u16 rupees) {
    u16* copy = dComIfGp_getRupeeDisp();
    if (meter) {
        meter->mRupeeDisp = rupees;
        meter->mRupeeCountRemain = 0;
    }
    if (copy)
        *copy = rupees;
}

#ifdef WWHD_ENABLE_GAME_CALLS
/**
 * [V] The meter process, or NULL when there is none - between stages, on the
 * title screen - or before the text delta is resolved. The same call the meter
 * itself makes every frame to find another process (0x02593B10 searches 0x76
 * with the same tag). It walks the process list, so it belongs on the thread
 * the game runs on, which is where the present hook this tool ticks from sits;
 * do not call it from a thread of your own.
 */
static __inline dMeter_c* dMeter_searchByProc(void) {
    typedef void* (*fopAcM_searchByID_t)(void* tag, u16* procName);
    fopAcM_searchByID_t search;
    u16 key;
    if (!wwhd_textResolved || !wwhd_regionResolved)
        return (dMeter_c*)0;
    search = WWHD_FN(fopAcM_searchByID_t, wwhd_map->fopAcM_searchByID);
    key = WWHD_PROC_METER;
    return (dMeter_c*)search(WWHD_AT_TEXT(void, wwhd_map->tagProcSearch), &key);
}
#endif /* WWHD_ENABLE_GAME_CALLS */

#endif /* LIBWWHD_D_METER_H */
