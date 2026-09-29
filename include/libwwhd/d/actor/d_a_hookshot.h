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
/*
 * [V] Code sites (words the same in all three builds):
 *   hsStickSite      dBgS::ChkPolyHSStick (USA 0x024EF398) `rlwinm r3,r9,0,27,27`
 *                    0x552306F6: the polygon's hookshot-stick bit (info word 3,
 *                    0x10). Its only callers are the hookshot and its reticle.
 *                    r12 points at the 0x10-byte info entry; r0, r4-r8, r10,
 *                    r11 and cr0 are free, and r9 is dead after. Attribute
 *                    (info word 1 >> 16) & 0x1F: 6 lava, 8 void.
 *   hsSightLisSite / hsSightLfsSite  setHookshotSight's reticle range,
 *                    `lis r8,0x1003` 0x3D001003 + `lfs f1,0x59B8(r8)` 0xC02859B8
 *                    (the player's own 1500); r8 is dead after. Pointing them
 *                    at the hookshot's range makes the reticle follow it; both
 *                    are 1500 in stock.
 *   hsChainClampSite execute's `li r12,0x12C` 0x3980012C: the 300-link clamp.
 *                    Clamped, a longer chain draws short and the first pull
 *                    frame teleports Link by the excess. Stock never passes 229.
 *   hsDrawPitchSite  the chain draw's `lfs f11,0x16C0(r7)` 0xC16716C0 (the
 *                    pitch); the draw lays r31 links (count) at that step along
 *                    the chain and f12 (= 0x100116C4) up it, into 300 records.
 *                    r8, r9, f0, f8-f10, f13, cr0 and 0x18/0x1C(r1) are free;
 *                    CTR is unused by the draw.
 */
#define WWHD_HS_STICK_WORD       0x552306F6u
#define WWHD_HS_SIGHT_LIS_WORD   0x3D001003u
#define WWHD_HS_SIGHT_LFS_WORD   0xC02859B8u
#define WWHD_HS_CLAMP_WORD       0x3980012Cu
#define WWHD_HS_DRAW_PITCH_WORD  0xC16716C0u
#define WWHD_HS_CHAIN_MAX        300

/* [V] The hookshot actor, Link's mActorKeepEquip (+0x6594) while he holds it. */
#define daPy_OFF_equipActor      0x6594
#define daHookshot_OFF_mode      0x0B0   /* [V] s32: 0 wait, 1 shot, 2 return, 3 pull */
#define daHookshot_OFF_chainCnt  0xD504  /* [V] s32 */
#define WWHD_HOOKSHOT_MODE_RETURN 2
#define WWHD_HOOKSHOT_MODE_PULL   3

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
