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

/**
 * [V] The `bl checkHeavyStateOn` calls that make the Iron Boots walk heavy:
 * setBlendMoveAnime's walk rates, animation choice (WALKHBOOTS), run branch
 * and foot flag; setBlendAtnMoveAnime's strafe rate; setStickData's halving;
 * setSpeedAndAngleNormal's x4 and threshold; setNormalSpeedF's target;
 * setDoStatusBasic. Each is followed by `cmpwi r3,0`, so every volatile
 * register is dead there. Answering "not heavy" at these alone, while the
 * boots are the only reason, gives the normal walk and run and keeps the
 * boots' weight (gravity, wind, switches, sinking), which is read elsewhere.
 * The call words are the same in all three builds.
 */
typedef struct wwhd_codeSite_t {
    wwhd_addr_t usa, eur, jap;
    u32         word;
} wwhd_codeSite_t;

#define WWHD_HEAVY_WALK_SITES 10
static const wwhd_codeSite_t wwhd_heavyWalkSites[WWHD_HEAVY_WALK_SITES] = {
    { 0x023E15E0u, 0x023E15E4u, 0x023E15E8u, 0x4BFFA645u },
    { 0x023E16C4u, 0x023E16C8u, 0x023E16CCu, 0x4BFFA561u },
    { 0x023E1C40u, 0x023E1C44u, 0x023E1C48u, 0x4BFF9FE5u },
    { 0x023E1C50u, 0x023E1C54u, 0x023E1C58u, 0x4BFF9FD5u },
    { 0x023E8518u, 0x023E851Cu, 0x023E8520u, 0x4BFF370Du },
    { 0x023F41C0u, 0x023F41C4u, 0x023F41C8u, 0x4BFE7A65u },
    { 0x024165ACu, 0x024165B0u, 0x024165B4u, 0x4BFC5679u },
    { 0x0241684Cu, 0x02416850u, 0x02416854u, 0x4BFC53D9u },
    { 0x024162B8u, 0x024162BCu, 0x024162C0u, 0x4BFC596Du },
    { 0x023EA6B4u, 0x023EA6B8u, 0x023EA6BCu, 0x4BFF1571u },
};

/** A code site's link-time address in the running build, 0 before one is selected. */
static __inline wwhd_addr_t wwhd_codeSiteAddr(const wwhd_codeSite_t* s) {
    if (!s || !wwhd_regionResolved || !wwhd_regionInfo_p)
        return 0;
    switch (wwhd_regionInfo_p->region) {
    case WWHD_REGION_USA:
    case WWHD_REGION_RANDO: return s->usa;
    case WWHD_REGION_EUR:   return s->eur;
    case WWHD_REGION_JAP:   return s->jap;
    default:                return 0;
    }
}

/** [V] Link's u32 at a byte offset, or NULL without Link. */
static __inline u32* daPy_u32At(daPy_lk_c* link, u32 ofs) {
    return link ? (u32*)((u8*)link + ofs) : (u32*)0;
}

static __inline int daPy_hasIronBootsOn(daPy_lk_c* link) {
    const u32* f = daPy_u32At(link, WWHD_DAPY_OFF_FLAGS0);
    return f && (*f & WWHD_DAPY_FLAGS0_IRON_BOOTS) != 0;
}

#endif /* LIBWWHD_D_A_PLAYER_EQUIP_H */
