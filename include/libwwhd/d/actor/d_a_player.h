#ifndef LIBWWHD_D_A_PLAYER_H
#define LIBWWHD_D_A_PLAYER_H

#include "libwwhd/wwhd_types.h"
#include "libwwhd/wwhd_region.h"
#include "libwwhd/f_op/f_op_actor.h"
#include "libwwhd/d/actor/d_a_player_demo.h"
#include "libwwhd/d/actor/d_a_player_proc.h"
#include "libwwhd/d/d_save.h"
#include "libwwhd/d/d_com_inf_game.h"
#include "libwwhd/d/d_stage.h"

/**
 * libwwhd - Link (d_a_player_main.cpp and its .inc siblings)
 *
 * Link's state is split across THREE places, and mixing them up is the easiest
 * way to waste a day:
 *
 *   1. the transform store    - three globals. Where Link actually is.
 *   2. daPy_lk_c (the actor)  - reloaded from 1 at the top of every frame,
 *                               plus momentum and everything else.
 *   3. dSv_player_c (the save)- items, equipment, hearts, rupees.
 *
 * Use daPy_setPosition() rather than writing the actor: see the note on the
 * transform store below for why writing the actor alone does nothing.
 *
 * WWHD's d_a_player_main.cpp is far larger than the GameCube original - asserts
 * fire from line 24239 where the decomp's reconstruction is 13168 lines total.
 * The object is correspondingly huge: mCurProc alone sits at +0x65F0. Only what
 * has been read out of code is named and the size is not asserted, so treat
 * sizeof(daPy_lk_c) as a lower bound and do not allocate one. Casting works.
 */
typedef struct daPy_lk_c {
    /* 0x000 */ fopAc_ac_c base;                    /* [V] see f_op_actor.h */
    /* 0x3AC */ u8         _unk_3AC[0x3B8 - 0x3AC]; /* [?] */
    /* 0x3B8 */ u32        mFlags;                  /* [V] offset; name [P].
                                                     *     Tested as mFlags &
                                                     *     0x40 at 0x02249058 */
    /* 0x3BC */ u8         _unk_3BC[0x3C4 - 0x3BC]; /* [?] */
    /* 0x3C4 */ f32        mMaxNormalSpeed;         /* [V] the ceiling the
                                                     *     current move ramps
                                                     *     mNormalSpeed toward;
                                                     *     see the accessor */
    /* 0x3C8 */ u8         _unk_3C8[0x3D0 - 0x3C8]; /* [?] */
    /* 0x3D0 */ csXyz      mBodyAngle;              /* [V] y read at +0x3D2
                                                     *     throughout the
                                                     *     player code */
    /* 0x3D6 */ u8         _unk_3D6[0x420 - 0x3D6]; /* [?] */
    /* 0x420 */ daPy_demo_c mDemo;                   /* [V] scripted animation
                                                      *     state; see
                                                      *     d_a_player_demo.h */
    /* 0x438 */ u8         _unk_438[0x65F0 - 0x438];/* [?] */
    /* 0x65F0 */ s32       mCurProc;                /* [V] the current player
                                                     *     procedure id */
    /* 0x65F4 */ u8        mCurProcFunc[12];        /* [V] pointer-to-member,
                                                     *     written with
                                                     *     mCurProc by the
                                                     *     setter */
} daPy_lk_c;
WWHD_ASSERT_OFFSET(daPy_lk_c, base,         0x000);
WWHD_ASSERT_OFFSET(daPy_lk_c, mFlags,       0x3B8);
WWHD_ASSERT_OFFSET(daPy_lk_c, mMaxNormalSpeed, 0x3C4);
WWHD_ASSERT_OFFSET(daPy_lk_c, mBodyAngle,   0x3D0);
WWHD_ASSERT_OFFSET(daPy_lk_c, mDemo,        WWHD_DAPY_OFF_DEMO);
WWHD_ASSERT_OFFSET(daPy_lk_c, mCurProc,     0x65F0);
WWHD_ASSERT_OFFSET(daPy_lk_c, mCurProcFunc, 0x65F4);

/* ========================================================================
 * Link's transform store - READ THIS BEFORE MOVING LINK.
 *
 * THIS IS NOT A STRUCT. It is three separate .bss globals that happen to sit
 * 0x08 and 0x40 apart, and libwwhd addresses them from one anchor slot only
 * because a single slot is cheaper than three. The disassembly settles it: each
 * is built independently as `lis rN, 0x1047` plus its own negative
 * displacement - -0x32F8, -0x32F0, -0x32B8 - and the five functions that use
 * them set up three separate base registers rather than one base plus member
 * offsets. The 0x32 bytes between shape angle and position hold other globals
 * with different owners, so a struct here would have to invent padding over
 * live state belonging to someone else.
 *
 * DIRECTION. fopAc_ac_c::current.pos is NOT authoritative for the player. The
 * player's actor execute reloads the actor FROM these globals at the top of
 * every frame and writes them back at the bottom, so writing the actor alone is
 * undone on the next frame and Link snaps back. Write both, which is what
 * daPy_setPosition() does.
 *
 * The execute is 0x0240CDD0, identified from the actor profile at 0x101CEC74:
 * its five words are create / delete / execute / isDelete / draw, the execute
 * slot holds the one-instruction thunk 0x0240EBB0 (`b 0x0240CDD0`), and the
 * slot after it is `li r3,1; blr` - an isDelete returning TRUE, which is what
 * makes the table's identity unambiguous.
 *
 * Both directions are visible inside it, with two early-outs that skip both:
 *
 *     restore, 0x0240D130:  lwzu r7, -0x32B8(r28)   ; r28 = 0x1046CD48
 *                           stw  r7, 0x314(r31)     ; store -> actor
 *     save,    0x0240EB00:  lwz  r11, 0x314(r31)
 *                           stw  r11, 0x0(r28)      ; actor -> store
 *
 * The mapping the save half makes explicit:
 *
 *     0x1046CD08..0C  <->  actor +0x320..0x324   current.angle
 *     0x1046CD10..14  <->  actor +0x328..0x32C   shape_angle
 *     0x1046CD48..50  <->  actor +0x314..0x31C   current.pos
 *
 * Corroborated by a working Cemu teleport cheat that writes the actor and these
 * globals together, and whose own label for 0x1046CD12 ("linkFacingAddress") is
 * shape_angle.y and agrees with the offsets here.
 *
 * All three addresses are identical in all three regions, carrying the same
 * five referencing functions and the same load/store profile in each.
 * ===================================================================== */

/** [V] Offsets of the three globals from the wwhd_map->lkTransformStore
 *  anchor. Displacements, not struct members - see above. */
#define WWHD_LK_STORE_ANGLE      0x00
#define WWHD_LK_STORE_SHAPEANGLE 0x08
#define WWHD_LK_STORE_POS        0x40

/** [V] The stored current.angle, or NULL before a region is selected. */
static __inline csXyz* daPy_getStoreAngle(void) {
    if (!wwhd_regionResolved)
        return (csXyz*)0;
    return WWHD_AT_DATA(csXyz, wwhd_map->lkTransformStore + WWHD_LK_STORE_ANGLE);
}

/** [V] The stored shape_angle; .y is the facing angle. */
static __inline csXyz* daPy_getStoreShapeAngle(void) {
    if (!wwhd_regionResolved)
        return (csXyz*)0;
    return WWHD_AT_DATA(csXyz, wwhd_map->lkTransformStore + WWHD_LK_STORE_SHAPEANGLE);
}

/** [V] The stored position - where Link actually is. */
static __inline cXyz* daPy_getStorePos(void) {
    if (!wwhd_regionResolved)
        return (cXyz*)0;
    return WWHD_AT_DATA(cXyz, wwhd_map->lkTransformStore + WWHD_LK_STORE_POS);
}

/** [V] Link's actor, or NULL if he is not spawned. */
static __inline daPy_lk_c* daPy_lk_c_getPlayer(void) {
    return (daPy_lk_c*)dComIfGp_getPlayer();
}

/**
 * [V] daPy_lk_c::mNormalSpeed (GameCube +0x35BC): the forward speed the
 * current move asked for, before the ground had its say.
 *
 * posMoveFromFootPos (0x023FCB9C) is the GameCube function line for line:
 * it scales this by (1 - slope factor at +0x69F0), adds the foot-position
 * term, multiplies by the cosine of the ground angle and, when uphill, by
 * 0.85, then stores the result at +0x370 (speedF) or zeroes it under 0.05.
 * The same function checks mCurProc against 55 (SWIM_MOVE) as the decomp
 * does. Two more sightings agree: the proc setter at 0x0241A05C writes 0.0
 * here on entry, and the jump check at 0x023F81A4 compares it against a
 * threshold. (An earlier value of 0x6938 was a pointer-sized member the
 * player takes the address of at 0x0241066C - not this.)
 *
 * So for a roll the field holds the full roll speed while speedF holds what
 * survived the slope; the two agree on flat ground.
 */
#define WWHD_DAPY_OFF_NORMAL_SPEED 0x6A14

static __inline f32* daPy_getNormalSpeedPtr(void) {
    daPy_lk_c* link = daPy_lk_c_getPlayer();
    return link ? (f32*)((u8*)link + WWHD_DAPY_OFF_NORMAL_SPEED) : (f32*)0;
}

/**
 * [V] daPy_lk_c::mMaxNormalSpeed: what mNormalSpeed is ramped toward.
 *
 * Three functions agree on +0x3C4. setNormalSpeedF (0x02416230, the GameCube
 * function line for line) forms its target as this times the stick distance
 * at +0x6A08 - squared unless an event is running or Link is carrying
 * something - and moves +0x6A14 toward it, by the accel it was handed or by
 * cLib_addCalc. The swim speed calc (0x0242ECFC) writes the 18.0 constant
 * behind daPy_getSwimSpeedPtr() here every frame, scaled down as momentum
 * swings away from the facing. The swim move (0x0242F70C) divides
 * |mNormalSpeed| by it for the stroke rate.
 *
 * So it is "as fast as this move goes right now": with the swim boost on it is
 * the boosted constant under the same scale. Read it; the move rewrites it.
 */
static __inline f32* daPy_getMaxNormalSpeedPtr(void) {
    daPy_lk_c* link = daPy_lk_c_getPlayer();
    return link ? &link->mMaxNormalSpeed : (f32*)0;
}

/**
 * [V] Link's current procedure id, or -1 when he is not spawned.
 *
 * This is the "what is Link doing" state. Found by shape: +0x65F0 is compared
 * against 54 distinct small constants across 62 player functions, which is what
 * `mCurProc == daPyProc_*_e` compiles to and is far and away the densest such
 * member. Corroborated by it being loaded 261 times but stored only twice, with
 * the single setter at 0x023DFDD8 writing it together with the adjacent
 * pointer-to-member words - the GameCube setProc(proc, func) shape.
 *
 * The VALUES are deliberately not named. WWHD renumbered at least one process
 * enum relative to the GameCube build (see WWHD_PROC_SHIP), so mapping these
 * ids to daPyProc_*_e names would be a guess. Show the number until each one is
 * confirmed.
 */
static __inline s32 daPy_getCurProc(void) {
    daPy_lk_c* link = daPy_lk_c_getPlayer();
    return link ? link->mCurProc : (s32)-1;
}


/**
 * [V] Link's demo mode, or daPyDemo_UNK00_e when he is not spawned.
 *
 * This is the scripted-animation state, NOT the player procedure - see
 * daPy_getCurProc() for that. The two are separate members and answer different
 * questions: mCurProc is "what is Link doing", mDemo.mDemoMode is "what scripted
 * sequence is playing over him".
 */
static __inline u32 daPy_getDemoMode(void) {
    daPy_lk_c* link = daPy_lk_c_getPlayer();
    return link ? link->mDemo.mDemoMode : (u32)daPyDemo_UNK00_e;
}

/** [V] The demo block itself, or NULL if Link is not spawned. */
static __inline daPy_demo_c* daPy_getDemo(void) {
    daPy_lk_c* link = daPy_lk_c_getPlayer();
    return link ? &link->mDemo : (daPy_demo_c*)0;
}

/**
 * [V] Is Link in this procedure right now?
 *
 * Typed, so the enum does the checking: daPy_isProc(daPyProc_DEMO_DOOR_OPEN_e)
 * rather than comparing daPy_getCurProc() against a bare number. Returns 0 when
 * he is not spawned, so it is safe to call unconditionally.
 */
static __inline int daPy_isProc(enum daPy_proc_e proc) {
    daPy_lk_c* link = daPy_lk_c_getPlayer();
    return link && link->mCurProc == (s32)proc;
}

/** [V] Is this scripted animation playing right now? The demo-mode counterpart
 *  of daPy_isProc(). */
static __inline int daPy_isDemoMode(enum daPy_demoMode_e mode) {
    daPy_lk_c* link = daPy_lk_c_getPlayer();
    return link && link->mDemo.mDemoMode == (u32)mode;
}

/** [P] Name of Link's current demo mode, or "?" when he is not spawned. */
static __inline const char* daPy_getDemoModeName(void) {
    daPy_lk_c* link = daPy_lk_c_getPlayer();
    return link ? daPy_demoModeName(link->mDemo.mDemoMode) : "?";
}

/* --- Storage-glitch states -----------------------------------------------
 * Chest storage and door cancel work by keeping one of these set across a
 * transition, so the sequence resumes somewhere the game never intended. Two
 * different members are involved and they are easy to confuse:
 *
 *   mDemo.mDemoMode  the scripted ANIMATION playing over Link
 *   mCurProc         the PROCEDURE driving him
 *
 * Both have a chest entry and they are not the same value. All the ids used
 * below are [V] - HD's player code tests each of them - except the demo-mode
 * DOOR entry, which was never observed, hence no helper for it.
 *
 * These are READ-ONLY. libwwhd offers no setter: a sequence depends on several
 * fields agreeing, and writing one alone leaves the player in a state the game
 * never constructs. Reading them to see what a run is doing is safe; forcing
 * them is not something a library should hand out.
 * ---------------------------------------------------------------------- */

/**
 * [P] Is Link aboard the King of Red Lions?
 *
 * The twelve SHIP_* procedures are one contiguous run of ids, which is what
 * makes a range test rather than twelve comparisons correct here: the run is
 * bounded by the address run the names were aligned on, so a procedure inside
 * it is a boat procedure whether or not its individual name is right.
 *
 * [P] rather than [V] because it rests on the name transcription. The ids
 * themselves are verified.
 */
static __inline int daPy_isRidingShip(void) {
    s32 proc = daPy_getCurProc();
    return proc >= (s32)daPyProc_SHIP_READY_e &&
           proc <= (s32)daPyProc_SHIP_RESTART_e;
}

/** [P] Is Link in the water? Same reasoning as daPy_isRidingShip(): SWIM_UP,
 *  SWIM_WAIT and SWIM_MOVE are consecutive. */
static __inline int daPy_isSwimming(void) {
    s32 proc = daPy_getCurProc();
    return proc >= (s32)daPyProc_SWIM_UP_e &&
           proc <= (s32)daPyProc_SWIM_MOVE_e;
}

/** [V] The chest-opening ANIMATION is playing (demo mode 10). */
static __inline int daPy_isChestOpenDemo(void) {
    return daPy_isDemoMode(daPyDemo_OPEN_TREASURE_e);
}

/** [V] The chest-opening PROCEDURE is running (proc 173). */
static __inline int daPy_isChestOpenProc(void) {
    return daPy_isProc(daPyProc_DEMO_OPEN_TREASURE_e);
}

/** [V] The door-opening PROCEDURE is running (proc 193) - what door cancel
 *  interrupts. The demo-mode side has its own DOOR entry, which HD was NOT
 *  observed using, so this is the reliable route. */
static __inline int daPy_isDoorOpenProc(void) {
    return daPy_isProc(daPyProc_DEMO_DOOR_OPEN_e);
}

/** [V] The salvage-chest procedure (proc 211). */
static __inline int daPy_isSalvageOpenProc(void) {
    return daPy_isProc(daPyProc_DEMO_OPEN_SALVAGE_TREASURE_e);
}

/** [P] Name of Link's current procedure, or "?" when he is not spawned.
 *  The id is verified; the name is transcribed - see d_a_player_proc.h. */
static __inline const char* daPy_getCurProcName(void) {
    daPy_lk_c* link = daPy_lk_c_getPlayer();
    return link ? daPy_procName(link->mCurProc) : "?";
}

/**
 * [V] Link's swim speed constant, or NULL before a region is selected.
 *
 * WHY THIS AND NOT speedF. The swim procedure recomputes fopAc_ac_c::speedF
 * from stick input every frame and then decomposes it into speed.x/speed.z, so
 * a value written into speedF between frames is overwritten before anything
 * integrates it. This constant is the INPUT to that calculation, so writing it
 * changes the result the game itself produces - the general rule for making a
 * modifier stick is in docs/SUBSYSTEMS.md.
 *
 * Stock value is WWHD_SWIM_SPEED_STOCK. It lives in .rodata, which on Wii U is
 * loaded into the data region rather than the write-protected code region, so
 * it is writable. Restore it when you are done: it is global, not per-save.
 */
static __inline f32* daPy_getSwimSpeedPtr(void) {
    if (!wwhd_regionResolved)
        return (f32*)0;
    return WWHD_AT_DATA(f32, wwhd_map->swimSpeedConst);
}

/** [V] The shipped value of the above, for restoring it. */
#define WWHD_SWIM_SPEED_STOCK 18.0f

/** [V] The procedure table, or NULL before a region is selected. */
static __inline daPy_procEntry_c* daPy_getProcTable(void) {
    if (!wwhd_regionResolved)
        return (daPy_procEntry_c*)0;
    return WWHD_AT_DATA(daPy_procEntry_c, wwhd_map->daPyProcTable);
}

/**
 * [V] The room Link is standing in, or -1 if he is not spawned.
 *
 * Prefer this over dComIfGp_getCurStageRoomNo(): the stage record holds the
 * room the stage was ENTERED at and is not updated as he walks, whereas this
 * tracks him. In a single-room stage they agree, which is what makes the
 * difference easy to miss.
 */
static __inline s8 daPy_getRoomNo(void) {
    return fopAcM_getRoomNo(dComIfGp_getPlayer());
}

/** [V] The room Link BELONGS to - where he was placed on entry - or -1.
 *  See the "which room number?" note in d_stage.h. */
static __inline s8 daPy_getHomeRoomNo(void) {
    return fopAcM_getHomeRoomNo(dComIfGp_getPlayer());
}

/** [V] The room record for the room Link is in, or NULL. */
static __inline dStage_roomStatus_c* daPy_getRoomStatus(void) {
    return dStage_getRoomStatus(daPy_getRoomNo());
}

/** [V] The zone id of the room Link is in, or -1. */
static __inline s8 daPy_getZoneNo(void) {
    return dStage_getZoneNo(daPy_getRoomNo());
}

/** [V] Link's body-relative look/yaw angle. */
static __inline s16 daPy_getLookAngleY(const daPy_lk_c* player) {
    return player ? player->mBodyAngle.y : (s16)0;
}

/** [V] The save block holding Link's items and status. */
static __inline dSv_player_c* daPy_lk_c_getPlayer_save(void) {
    return dComIfGs_getPlayerSave();
}

/**
 * [V] Move Link so that the move survives the next frame.
 *
 * Writes the store the execute reloads from, then the actor's own copies so the
 * current frame is consistent. `old` goes with `current` because the actor
 * update copies one into the other before running the actor, and leaving old
 * behind reads as one enormous step through the world. Momentum is cleared for
 * the same reason a teleport should not fling you on arrival.
 */
static __inline void daPy_setPosition(const cXyz* pos) {
    cXyz* storePos;
    daPy_lk_c* link;
    if (!pos)
        return;
    storePos = daPy_getStorePos();
    if (!storePos)
        return;
    *storePos = *pos;
    link = daPy_lk_c_getPlayer();
    if (link) {
        link->base.current.pos = *pos;
        link->base.old.pos     = *pos;
        fopAcM_stop(&link->base);
    }
}

/** [V] Face Link along `angle`, surviving the next frame. */
static __inline void daPy_setFacing(s16 angle) {
    csXyz* storeShape = daPy_getStoreShapeAngle();
    daPy_lk_c* link;
    if (!storeShape)
        return;
    storeShape->y = angle;
    link = daPy_lk_c_getPlayer();
    if (link)
        link->base.shape_angle.y = angle;
}

/* --- Link's create parameters --------------------------------------------
 * [V] dStage_playerInit (0x025C1384) reads the room from bits 0..5 and the
 * start mode from bits 12..15 of the parameters word it hands Link's create;
 * for a -1 spawn that word comes straight from dSv_restart_c::mRestartParam,
 * for -3 from dSv_turnRestart_c::mParam.
 *
 * [V] Link's create (0x02410BE8) dispatches on the start mode (bits 12..15)
 * and, for a mode with no fixed cutscene, hands bits 24..31 to the event
 * manager's setStartDemo (0x02543534), which treats 0xFF as "no event".
 *
 * [V] daPy_lk_c::getDayNightParamData (0x0243B188) packs the word the
 * Song of Passing resumes from: mode 2 when the SHIP_RIDE status bit is set,
 * and 0x140 rather than 0x40 when a boat actor exists - so mode 2 is "start
 * aboard" and bit 0x100 "the boat is here". Bit 0x40 is GameCube's "do not
 * refresh the restart room on create" and unsighted here.
 * ---------------------------------------------------------------------- */
#define WWHD_PLAYER_START_MODE_NORMAL 0u      /* [P] the PLYR default */
#define WWHD_PLAYER_START_MODE_SHIP   2u      /* [V] */
#define WWHD_PLAYER_EVENT_NONE        0xFFu   /* [V] */
#define WWHD_PLAYER_PARAM_HAS_SHIP    0x100u  /* [V] */
#define WWHD_PLAYER_PARAM_KEEP_RESTART 0x40u  /* [P] */

static __inline u32 daPy_packStartParam(s8 roomNo, u32 startMode, u32 eventIdx) {
    return ((u32)roomNo & 0x3Fu) | ((startMode & 0xFu) << 12) |
           ((eventIdx & 0xFFu) << 24);
}

#endif /* LIBWWHD_D_A_PLAYER_H */
