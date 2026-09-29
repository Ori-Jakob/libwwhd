#ifndef LIBWWHD_D_A_HOOKSHOT_H
#define LIBWWHD_D_A_HOOKSHOT_H

#include "libwwhd/wwhd_types.h"
#include "libwwhd/wwhd_map.h"
#include "libwwhd/wwhd_region.h"

/**
 * libwwhd - the Hookshot (d_a_hookshot.cpp) and the Grappling Hook (d_a_himo2)
 *
 * [V] Hookshot: every tuning float lives in a pool only hookshot code reads
 * (xrefs and a whole-program displacement scan, all three builds), at the same
 * .rodata address in each build. procWait (USA/EUR 0x021789B4) aims mMoveVec
 * at pitch * direction; procShot (0x02178DDC) steps it `shotSpeed` times a
 * frame and turns back once past `range`; procReturn (0x02179B98) moves
 * `returnSpeed` a frame and snaps home within 10 pitches; procPlayerPull
 * (0x02179888) pulls Link min(links, 9) pitches a frame. execute clamps the
 * chain to 300 links (the draw buffer), so keep range + pitch * shotSpeed
 * within 300 pitches and returnSpeed under 20 pitches.
 *
 * [V] Grappling Hook: l_himo2HIO, a .bss block filled once by its TU static
 * initialiser (USA/EUR 0x0216F6D8) and read only by setTargetPos
 * (0x0216EEB8): the horizontal search radius, the vertical window and the
 * aim-cone width. Same address in all three builds.
 */
#define WWHD_HOOKSHOT_OFF_PITCH        0x00   /* [V] f32, 7.0 */
#define WWHD_HOOKSHOT_OFF_PITCH_Y      0x04   /* [V] f32, -7.0, draw only */
#define WWHD_HOOKSHOT_OFF_RANGE        0x64   /* [V] f32, 1500.0 */
#define WWHD_HOOKSHOT_OFF_SHOT_SPEED   0x6C   /* [V] f32, 15.0 pitches a frame */
#define WWHD_HOOKSHOT_OFF_RETURN_SPEED 0x80   /* [V] f32, 63.0 units a frame */

#define WWHD_GRAPPLE_OFF_AIM_WIDTH     0x08   /* [V] f32, 60.0 */
#define WWHD_GRAPPLE_OFF_VERTICAL      0x0C   /* [V] f32, 1500.0 */
#define WWHD_GRAPPLE_OFF_RANGE         0x10   /* [V] f32, 1000.0 */

/** [V] A hookshot tuning float by its offset in the pool, or NULL. */
static __inline f32* daHookshot_param(u32 ofs) {
    if (!wwhd_regionResolved)
        return (f32*)0;
    return WWHD_AT_DATA(f32, wwhd_map->hookshotParams + ofs);
}

/** [V] A grappling hook search float by its offset in l_himo2HIO, or NULL. */
static __inline f32* daHimo2_param(u32 ofs) {
    if (!wwhd_regionResolved)
        return (f32*)0;
    return WWHD_AT_DATA(f32, wwhd_map->himo2HIO + ofs);
}

#endif /* LIBWWHD_D_A_HOOKSHOT_H */
