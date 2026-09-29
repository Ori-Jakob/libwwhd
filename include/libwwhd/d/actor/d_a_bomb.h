#ifndef LIBWWHD_D_A_BOMB_H
#define LIBWWHD_D_A_BOMB_H

#include "libwwhd/wwhd_types.h"
#include "libwwhd/f_op/f_op_actor.h"

/**
 * libwwhd - bombs (d_a_bomb.cpp, daBomb_c)
 *
 * Link's bombs, cannon and Tingle Tuner bombs are all daBomb_c, process name
 * 0x126 (profile at USA/EUR 0x101922F4, JAP 0x10192314: name +0x08, size 0xB50
 * +0x10). Bomb flowers are daBomb2 and never match. Only create state 3 is
 * Link's: create_init (USA 0x020C6408) sets +0xA70 for it alone and
 * procExplode_init (0x020C5EF8) clears it again, so +0xA70 reads "Link's bomb,
 * still live".
 *
 * [V] The fuse: create_init sets the s16 at +0xA7C to 150 (an immediate). The
 * wait and carry procs run checkExplodeTimer (0x020C6CC4), which explodes the
 * bomb through procExplode_init the frame --rest reaches 0, unless the state's
 * attribute has 0x200 (cannon bombs). Writing 1 makes the game explode it on
 * its next execute through its own path; the Armos Knights do exactly that
 * (0x0204AF3C) after claiming a bomb with +0xA71.
 *
 * [V] Link's live-bomb count is the u8 at player +0x690C: raised when he pulls
 * one (makeItemType 0x023DF600), lowered by procExplode_init and bombDelete
 * only when non-zero, and refused at 3 by checkNewItemChange (0x023EF7CC,
 * `cmplwi r0,3` at 0x023EFAF8). Nothing else reads it.
 *
 * Offsets are identical in all three builds.
 */
#define WWHD_PROC_BOMB          0x126

#define daBomb_OFF_linkLive     0xA70  /* [V] u8, Link's bomb and not yet exploded */
#define daBomb_OFF_claimed      0xA71  /* [V] u8, an enemy (Armos, Gohdan) owns the fuse */
#define daBomb_OFF_restTime     0xA7C  /* [V] s16, frames until it explodes */
#define daBomb_OFF_deleting     0xB05  /* [V] u8, delete requested */
#define daBomb_FUSE             150    /* [V] create_init's rest time */

#define fopAc_OFF_status        0x2E0  /* [V] u32 actor status */
#define fopAc_STATUS_CARRY      0x2000 /* [V] being carried (fopAcM_setCarryNow) */

#define daPy_OFF_activeBombs    0x690C /* [V] u8, Link's bombs alive */

/** [V] A live bomb of Link's that no enemy has claimed, else NULL. */
static __inline u8* daBomb_asLinkBomb(fopAc_ac_c* ac) {
    u8* b = (u8*)ac;
    if (!ac || fopAcM_getProcName(ac) != WWHD_PROC_BOMB)
        return (u8*)0;
    if (!b[daBomb_OFF_linkLive] || b[daBomb_OFF_deleting] || b[daBomb_OFF_claimed])
        return (u8*)0;
    return b;
}

static __inline s16* daBomb_restTime(u8* bomb) {
    return (s16*)(bomb + daBomb_OFF_restTime);
}

static __inline int daBomb_isCarried(const u8* bomb) {
    return (*(const u32*)(bomb + fopAc_OFF_status) & fopAc_STATUS_CARRY) != 0;
}

#endif /* LIBWWHD_D_A_BOMB_H */
