#ifndef LIBWWHD_F_OP_ACTOR_H
#define LIBWWHD_F_OP_ACTOR_H

#include "libwwhd/wwhd_types.h"
#include "libwwhd/wwhd_region.h"

/**
 * libwwhd - actor base class (f_op_actor.cpp / f_op_actor_mng.cpp)
 *
 * WWHD's fopAc_ac_c IS NOT THE GAMECUBE LAYOUT. The prefix runs +0x4 through
 * the event-info block and +0x11C from `current` onward; past that the GameCube
 * field deltas hold again. The extra 0x118 is dKy_tevstr_c growing from 0xB0 to
 * 0x1C8, which d_stage.h confirms independently. docs/SUBSYSTEMS.md (Actors) has the table.
 */

/** [V] Position, facing and room, as stored three times per actor. */
typedef struct actor_place {
    /* 0x00 */ cXyz  pos;     /* [V] */
    /* 0x0C */ csXyz angle;   /* [V] */
    /* 0x12 */ s8    roomNo;  /* [V] passed as the switch router roomNo */
    /* 0x13 */ u8    _unk_13; /* [?] */
} actor_place;
WWHD_ASSERT_SIZE(actor_place, 0x14);

/**
 * fopAc_ac_c - the actor base.
 *
 * Size 0x3AC is [P]: GameCube 0x290 plus the measured +0x11C, corroborated by
 * the actor at 0x0218A6F8 placing its own subclass fields from 0x3B4 up.
 */
typedef struct fopAc_ac_c {
    /* 0x000 */ u8          _unk_000[0x0F4];        /* [?] leafdraw_class base,
                                                     *     actor_type, tags */
    /* 0x0F4 */ wwhd_gptr_t heap;                   /* [V] JKRSolidHeap*;
                                                     *     asserted == 0 */
    /* 0x0F8 */ u8          _unk_0F8[0x110 - 0x0F8];/* [P] dEvt_info_c */
    /* 0x110 */ u8          tevStr[0x2EC - 0x110];  /* [V] offset; the draw call
                                                     *     in daShip_c::draw
                                                     *     takes it as tevstr.
                                                     *     Tail also holds setID,
                                                     *     group, cullType and
                                                     *     actor_status. [?] */
    /* 0x2EC */ actor_place home;                   /* [P] current - 0x28 */
    /* 0x300 */ actor_place old;                    /* [V] the actor update at
                                                     *     0x025D475C copies
                                                     *     0x314..0x324 here */
    /* 0x314 */ actor_place current;                /* [V] two unrelated actors */
    /* 0x328 */ csXyz       shape_angle;            /* [V] the facing you
                                                     *     usually want */
    /* 0x32E */ u8          _unk_32E[0x330 - 0x32E];/* [?] */
    /* 0x330 */ cXyz        scale;                  /* [P] */
    /* 0x33C */ cXyz        speed;                  /* [V] per-axis momentum */
    /* 0x348 */ wwhd_gptr_t cullMtx;                /* [P] */
    /* 0x34C */ u8          _unk_34C[0x364 - 0x34C];/* [P] cull box/sphere */
    /* 0x364 */ f32         cullSizeFar;            /* [P] */
    /* 0x368 */ wwhd_gptr_t model;                  /* [P] J3DModel* */
    /* 0x36C */ wwhd_gptr_t jntHit;                 /* [P] */
    /* 0x370 */ f32         speedF;                 /* [V] scalar ground speed */
    /* 0x374 */ f32         gravity;                /* [V] */
    /* 0x378 */ f32         maxFallSpeed;           /* [P] */
    /* 0x37C */ cXyz        eyePos;                 /* [V] */
    /* 0x388 */ u8          _unk_388[0x3A0 - 0x388];/* [P] attention_info */
    /* 0x3A0 */ s8          max_health;             /* [P] */
    /* 0x3A1 */ s8          health;                 /* [P] */
    /* 0x3A2 */ u8          _unk_3A2[0x3A4 - 0x3A2];/* [?] */
    /* 0x3A4 */ s32         itemTableIdx;           /* [P] */
    /* 0x3A8 */ u8          stealItemBitNo;         /* [P] */
    /* 0x3A9 */ s8          stealItemLeft;          /* [P] */
    /* 0x3AA */ u8          _unk_3AA[0x3AC - 0x3AA];/* [?] */
} fopAc_ac_c;
WWHD_ASSERT_OFFSET(fopAc_ac_c, heap,        0x0F4);
WWHD_ASSERT_OFFSET(fopAc_ac_c, tevStr,      0x110);
WWHD_ASSERT_OFFSET(fopAc_ac_c, home,        0x2EC);
WWHD_ASSERT_OFFSET(fopAc_ac_c, old,         0x300);
WWHD_ASSERT_OFFSET(fopAc_ac_c, current,     0x314);
WWHD_ASSERT_OFFSET(fopAc_ac_c, shape_angle, 0x328);
WWHD_ASSERT_OFFSET(fopAc_ac_c, scale,       0x330);
WWHD_ASSERT_OFFSET(fopAc_ac_c, speed,       0x33C);
WWHD_ASSERT_OFFSET(fopAc_ac_c, model,       0x368);
WWHD_ASSERT_OFFSET(fopAc_ac_c, speedF,      0x370);
WWHD_ASSERT_OFFSET(fopAc_ac_c, gravity,     0x374);
WWHD_ASSERT_OFFSET(fopAc_ac_c, eyePos,      0x37C);
WWHD_ASSERT_SIZE  (fopAc_ac_c,              0x3AC);

/**
 * [V] Process name of the King of Red Lions.
 *
 * Read directly out of the RIDE camera, which searches for the boat at five
 * separate points and loads the same immediate at every one:
 *
 *     li   r9, 0xa5
 *     addi r3, r3, 0x121c      ; tagProcSearch
 *     addi r4, r1, 0x50
 *     sth  r9, 0x50(r1)        ; the key is a u16
 *     bl   fopAcM_searchByID
 *
 * The GameCube decomp has fpcNm_SHIP_e = 0x00A7; that value does NOT apply to
 * WWHD, whose process-name enum was renumbered. This is a case where trusting
 * the decomp would have been wrong.
 *
 * NOTE: a live get_daShip() on console still returned NULL at sea. The constant
 * and the search mechanism are both confirmed, so that is a separate question -
 * most likely the boat is not a searchable process at the moment sampled, or
 * the text delta was unresolved. Do not "fix" this constant.
 */
#define WWHD_PROC_SHIP 0xA5

/** [V] A process name confirmed by reading the immediate out of FUN_0216B8B8.
 *  Kept as the worked example of what a correct key looks like. */
#define WWHD_PROC_UNKNOWN_E5 0xE5

/**
 * [V] The HUD meter process (d_meter.cpp, see d/d_meter.h). Two independent
 * sightings: the stage constructor creates it with `li r4, 0x1E1` at
 * 0x025C1A9C straight into the process-create call, and the meter's own
 * profile record carries 0x01E1 at +0x08 alongside the object size 0x3044 at
 * +0x10 and its method table at +0x24 (create 0x02595C6C, draw 0x02593B10).
 * The record sits at 0x101EA104 in USA and EUR and 0x101EA124 in JAP, with the
 * same name in all three.
 *
 * The record layout is pinned by the profile list at 0x101F3EE0 (USA), which
 * is indexed by process name: entry 0x1E1 points at the meter record above,
 * and entry 0xA5 points at a record carrying 0x00A5 at the same +0x08 - the
 * boat, WWHD_PROC_SHIP, confirmed independently at the top of this file.
 */
#define WWHD_PROC_METER 0x1E1

/** [V] One entry of the interleaved sin/cos table, indexed (angle >> 3). */
typedef struct cM_sinCos_c {
    /* 0x0 */ f32 sin;  /* [V] */
    /* 0x4 */ f32 cos;  /* [V] */
} cM_sinCos_c;
WWHD_ASSERT_SIZE(cM_sinCos_c, 0x8);

/** [V] The sin/cos table, or NULL before a region is selected. */
static __inline cM_sinCos_c* cM_getSinCosTable(void) {
    if (!wwhd_regionResolved)
        return (cM_sinCos_c*)0;
    return WWHD_AT_DATA(cM_sinCos_c, wwhd_map->sinCosTable);
}

/**
 * [V] The room an actor is currently in, or -1 if it has none.
 *
 * This is the LIVE room and it changes as the actor moves between them, unlike
 * the room recorded in the stage record. Independently corroborated: dBgS's
 * registration routine reads this exact offset (+0x326) off the actor it is
 * given, alongside shape_angle at +0x32A.
 */
static __inline s8 fopAcM_getRoomNo(const fopAc_ac_c* ac) {
    return ac ? ac->current.roomNo : (s8)-1;
}

/**
 * [V] The room an actor BELONGS to, or -1.
 *
 * Distinct from fopAcM_getRoomNo(): this is where the actor was placed, and it
 * is what the save switch/item routers are handed most often (346 call sites
 * against 189 for the live room). A flag belongs to the room the object sits
 * in, not to wherever it has since drifted.
 */
static __inline s8 fopAcM_getHomeRoomNo(const fopAc_ac_c* ac) {
    return ac ? ac->home.roomNo : (s8)-1;
}

/** [V] Read an actor's position. Returns non-zero on success. */
static __inline int fopAcM_getPosition(const fopAc_ac_c* ac, cXyz* out) {
    if (!ac || !out)
        return 0;
    *out = ac->current.pos;
    return 1;
}

/**
 * [V] Set an actor's position - the RAW write, `current` only.
 *
 * Use this to nudge an actor along its own motion. For a teleport use
 * fopAcM_warpPosition(): leaving `old` behind makes the gap between the two
 * read as one frame of travel, and anything that differences them - collision
 * sweeps, speed derivation - sees the actor cross the map in a frame.
 */
static __inline void fopAcM_setPosition(fopAc_ac_c* ac, f32 x, f32 y, f32 z) {
    if (!ac)
        return;
    ac->current.pos.x = x;
    ac->current.pos.y = y;
    ac->current.pos.z = z;
}

/**
 * [V] Stop an actor dead: clear per-axis momentum and scalar speed.
 *
 * This is the idiom at 0x0218A6F8, which zeroes the three floats at 0x33C plus
 * the scalar at 0x370. Gravity is left alone - zeroing it makes the actor float
 * rather than halt.
 */
static __inline void fopAcM_stop(fopAc_ac_c* ac) {
    if (!ac)
        return;
    ac->speed.x = 0.0f;
    ac->speed.y = 0.0f;
    ac->speed.z = 0.0f;
    ac->speedF  = 0.0f;
}

/**
 * [V] Teleport an actor: write `current` AND `old`, then stop it.
 *
 * `old` goes with `current` because the actor update copies one into the other
 * before running the actor. Momentum is cleared for the same reason a teleport
 * should not fling you on arrival. This is the shape daPy_setPosition() uses
 * for Link, generalised.
 *
 * Returns non-zero on success.
 */
static __inline int fopAcM_warpPosition(fopAc_ac_c* ac, const cXyz* pos) {
    if (!ac || !pos)
        return 0;
    ac->current.pos = *pos;
    ac->old.pos     = *pos;
    fopAcM_stop(ac);
    return 1;
}

#endif /* LIBWWHD_F_OP_ACTOR_H */
