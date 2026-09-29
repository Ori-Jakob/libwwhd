#ifndef LIBWWHD_D_A_PLAYER_EQUIP_H
#define LIBWWHD_D_A_PLAYER_EQUIP_H

#include "libwwhd/wwhd_types.h"
#include "libwwhd/d/actor/d_a_player.h"

/**
 * libwwhd - Link's equipment state (d_a_player_*.inc)
 *
 * Offsets are identical in all three builds; the daPy_py_c fields sit at the
 * GameCube offset + 0x11C.
 *
 * [V] Iron Boots: checkHeavyStateOn (USA 0x023DBC24) tests flags0 for
 * 0x02000000 (boots, toggled by procBootsEquip 0x0241F618) or 0x40000000
 * (Morth heavy state). The slowdown is setStickData (0x023F32E4) halving
 * mStickDistance by the pooled 0.5f at 0x100353A0 (about 130 readers, never
 * write it); setNormalSpeedF then aims mNormalSpeed at mMaxNormalSpeed * stick.
 *
 * [V] Magic Armor: flags1 bit 0. HD charges rupees per hit instead of draining
 * magic: the rupee-loss routine (0x023F4CB0) costs damage * 20 at the
 * `mulli r0,r0,0x14` below and scatters up to 200 rupees as pickups.
 *
 * [V] Magic arrows: flags1 bit 0x2000 (USE_ARROW_EFFECT), set by the arrow's
 * light effect while the arrow flies and ~60 frames after; the bow's reload
 * gate (checkNextActionBowReady 0x023EB208) waits on it. Nothing else reads it.
 *
 * [V] Hurricane Spin: procCutTurnMove_init (0x02442190) sets the counter to 47
 * only with event 0x0B20 (or tmp 0x0402), the sword in hand (0x103), 2+ magic
 * and no minigame type 2/6, else -1; procCutTurnMove (0x02442470) counts it
 * down and a release at 0 starts the Hurricane Spin, which spends 2 magic.
 *
 * [V] Rope climb: procRopeUp (0x02436E6C) moves y 5 per frame toward the
 * target at +0x69F8 while the climb animation runs; procRopeDown (0x02437000)
 * grows the step at +0x69F8 to 27 and moves toward +0x69FC.
 */
#define WWHD_DAPY_OFF_FLAGS0           0x3B8  /* [V] u32 */
#define WWHD_DAPY_FLAGS0_IRON_BOOTS    0x02000000u
#define WWHD_DAPY_OFF_FLAGS1           0x3BC  /* [V] u32 mNoResetFlg1 */
#define WWHD_DAPY_FLAGS1_MAGIC_ARMOR   0x00000001u
#define WWHD_DAPY_FLAGS1_ARROW_EFFECT  0x00002000u
#define WWHD_DAPY_OFF_SPIN_CHARGE      0x6916 /* [V] s16, -1 none, 47 fresh, 0 ready */
#define WWHD_DAPY_OFF_EQUIP_ITEM       0x69B0 /* [V] u16 mEquipItem */
#define WWHD_DAPY_OFF_STICK_DISTANCE   0x6A08 /* [V] f32 */
#define WWHD_DAPY_OFF_ROPE_TARGET      0x69F8 /* [V] f32, up target y or down step */
#define WWHD_DAPY_OFF_ROPE_BOTTOM      0x69FC /* [V] f32, down target y */
#define WWHD_DAPY_OFF_LOWER_ANM_RATE   0x5898 /* [V] f32, lower-body frame rate */

#define WWHD_ITEM_EQUIP_SWORD          0x103  /* [V] mEquipItem while the sword is out */
#define WWHD_SPIN_CHARGE_FRAMES        47     /* [V] procCutTurnMove_init */
#define WWHD_IRON_BOOTS_STICK_SCALE    0.5f   /* [V] the halving in setStickData */
#define WWHD_ROPE_UP_STEP              5.0f   /* [V] procRopeUp */

/** [V] Link's u32 at a byte offset, or NULL without Link. */
static __inline u32* daPy_u32At(daPy_lk_c* link, u32 ofs) {
    return link ? (u32*)((u8*)link + ofs) : (u32*)0;
}

static __inline int daPy_hasIronBootsOn(daPy_lk_c* link) {
    const u32* f = daPy_u32At(link, WWHD_DAPY_OFF_FLAGS0);
    return f && (*f & WWHD_DAPY_FLAGS0_IRON_BOOTS) != 0;
}

#endif /* LIBWWHD_D_A_PLAYER_EQUIP_H */
