#ifndef LIBWWHD_D_A_BOOMERANG_H
#define LIBWWHD_D_A_BOOMERANG_H

#include "libwwhd/wwhd_types.h"
#include "libwwhd/wwhd_map.h"
#include "libwwhd/wwhd_region.h"
#include "libwwhd/f_op/f_op_actor.h"

/**
 * libwwhd - the boomerang (d_a_boomerang.cpp)
 *
 * [V] Speed: procWait (USA 0x020CF4CC) sets speedF to 60 on the throw frame and
 * procMove (0x020CEE10) reads it every frame without writing it, for the step,
 * the reach test (dist < speedF) and the catch test (dist < 2*speedF). The
 * thrown boomerang is the player's mActorKeepThrow at +0x659C (the ship code
 * at 0x02431A1C loads it there); +0xB0 is 1 while it is thrown.
 *
 * [V] Range: getFlyMax (0x020CECF0) returns 2500 on foot and 5000 on the boat
 * or in GanonK, read from the .rodata floats below (only reader, same address
 * in all builds). The player uses it for the aim line too, so it also sets
 * the lock-on reach.
 *
 * [V] The lock-on count (5) is not raisable: every lock array holds exactly 5
 * (+0x26244 ids, +0x26258 actors, the sight packet, the se_flg table).
 */
#define WWHD_PROC_BOOMERANG        0x1B0

#define daPy_OFF_throwActor        0x659C /* [V] daBoomerang_c* in flight */

#define daBoomerang_OFF_mode       0x0B0  /* [V] s32, 1 while thrown */
#define daBoomerang_STOCK_SPEED    60.0f
#define daBoomerang_STOCK_FLY_MAX  2500.0f
#define daBoomerang_STOCK_FLY_MAX_FAR 5000.0f

/** [V] The boomerang in flight, or NULL. Found in the live actor list, since
 *  the player's keep pointer can outlive the actor. */
static __inline fopAc_ac_c* daBoomerang_findThrown(void) {
    wwhd_gptr_t node;
    for (node = fopAcTg_firstNode(); node; node = fopAcTg_nextNode(node)) {
        fopAc_ac_c* b = fopAcTg_nodeActor(node);
        if (fopAcM_getProcName(b) == WWHD_PROC_BOOMERANG &&
            *(const s32*)((const u8*)b + daBoomerang_OFF_mode) == 1)
            return b;
    }
    return (fopAc_ac_c*)0;
}

/** [V] Free-throw distance on foot / on the boat and in GanonK. */
static __inline f32* daBoomerang_getFlyMaxPtr(int far) {
    if (!wwhd_regionResolved)
        return (f32*)0;
    return WWHD_AT_DATA(f32, far ? wwhd_map->boomerangFlyMaxFar : wwhd_map->boomerangFlyMax);
}

#endif /* LIBWWHD_D_A_BOOMERANG_H */
