#ifndef LIBWWHD_D_A_SHIP_H
#define LIBWWHD_D_A_SHIP_H

#include "libwwhd/wwhd_types.h"
#include "libwwhd/wwhd_region.h"
#include "libwwhd/f_op/f_op_actor.h"
#include "libwwhd/d/d_camera.h"
#include "libwwhd/d/d_com_inf_game.h"

/**
 * libwwhd - the King of Red Lions (d_a_ship.cpp)
 *
 * daShip_c derives from fopAc_ac_c, so the boat's position is
 * ship->base.current.pos and its facing ship->base.shape_angle. The object is
 * large - accesses reach +0xD194, so upwards of 53 KB - and its true size is
 * unknown, so the struct stops after the last confirmed field and is not
 * size-asserted. Subclass fields were read from daShip_c::draw and the RIDE
 * camera.
 */
typedef struct daShip_c {
    /* 0x0000 */ fopAc_ac_c  base;                        /* [V] */
    /* 0x03AC */ u8          _unk_03AC[0x3B4 - 0x3AC];    /* [?] */
    /* 0x03B4 */ wwhd_gptr_t mpModelMain;                 /* [V] J3DModel* hull */
    /* 0x03B8 */ wwhd_gptr_t mpModelSail;                 /* [V] drawn with hull */
    /* 0x03BC */ wwhd_gptr_t mpSrtAnm;                    /* [V] the `srt != 0`
                                                           *     assert guards a
                                                           *     fetch from this */
    /* 0x03C0 */ wwhd_gptr_t mpTexAnm;                    /* [V] +0x1C takes the
                                                           *     frame value */
    /* 0x03C4 */ u8          mBaseMtx[0x30];              /* [V] concatenated
                                                           *     into the draw */
    /* 0x03F4 */ u8          _unk_03F4[0x418 - 0x3F4];    /* [?] */
    /* 0x0418 */ wwhd_gptr_t mpModelSpare0;               /* [V] drawn in state 2 */
    /* 0x041C */ wwhd_gptr_t mpModelSpare1;               /* [V] drawn in state 3 */
    /* 0x0420 */ wwhd_gptr_t mpModelSpare2;               /* [V] */
    /* 0x0424 */ u8          mClothPacket[0x554 - 0x424]; /* [V] offset; the sail
                                                           *     cloth sim driven
                                                           *     by mClothCount.
                                                           *     Interior [?] */
    /* 0x0554 */ wwhd_gptr_t mpVtable2;                   /* [V] secondary
                                                           *     dispatch table */
    /* 0x0558 */ u8          _unk_0558[0x5B4 - 0x558];    /* [?] */
    /* 0x05B4 */ wwhd_gptr_t mpSubActor;                  /* [V] */
    /* 0x05B8 */ u8          _unk_05B8[0x637 - 0x5B8];    /* [?] */
    /* 0x0637 */ s8          mState;                      /* [V] 2 and 3 select
                                                           *     draw paths */
    /* 0x0638 */ u8          _unk_0638[0x644 - 0x638];    /* [?] */
    /* 0x0644 */ u32         mFlags;                      /* [V] 0x00200000 is
                                                           *     toggled around
                                                           *     the sail path;
                                                           *     0x80000000 read
                                                           *     by RIDE */
    /* 0x0648 */ u8          _unk_0648[0x64C - 0x648];    /* [?] */
    /* 0x064C */ s16         mCamYaw;                     /* [V] read by RIDE */
    /* 0x064E */ u8          _unk_064E[0x664 - 0x64E];    /* [?] */
    /* 0x0664 */ u16         mSailAngle;                  /* [V] sin/cos index */
    /* 0x0666 */ s16         mSailPitch;                  /* [V] */
    /* 0x0668 */ u8          _unk_0668[0x67C - 0x668];    /* [?] */
    /* 0x067C */ s16         mCamYawOfs;                  /* [V] added to
                                                           *     shape_angle.y */
    /* 0x067E */ s16         mCamPitchOfs;                /* [V] */
    /* 0x0680 */ s16         mCamRollOfs;                 /* [V] turning path */
    /* 0x0682 */ u8          _unk_0682[0x686 - 0x682];    /* [?] */
    /* 0x0686 */ s16         mClothCount;                 /* [V] cloth runs >1 */
    /* 0x0688 */ u8          _unk_0688[0x6B8 - 0x688];    /* [?] */
    /* 0x06B8 */ f32         mAnimFrame;                  /* [V] */
    /* 0x06BC */ f32         mDrawYOffset;                /* [V] added to pos.y
                                                           *     for drawing */
} daShip_c;
WWHD_ASSERT_OFFSET(daShip_c, base,         0x0000);
WWHD_ASSERT_OFFSET(daShip_c, mpModelMain,  0x03B4);
WWHD_ASSERT_OFFSET(daShip_c, mpSrtAnm,     0x03BC);
WWHD_ASSERT_OFFSET(daShip_c, mpVtable2,    0x0554);
WWHD_ASSERT_OFFSET(daShip_c, mState,       0x0637);
WWHD_ASSERT_OFFSET(daShip_c, mFlags,       0x0644);
WWHD_ASSERT_OFFSET(daShip_c, mSailAngle,   0x0664);
WWHD_ASSERT_OFFSET(daShip_c, mCamYawOfs,   0x067C);
WWHD_ASSERT_OFFSET(daShip_c, mClothCount,  0x0686);
WWHD_ASSERT_OFFSET(daShip_c, mDrawYOffset, 0x06BC);

/* Confirmed fields too far out to pad to. Reach them by byte offset. */
#define daShip_OFF_clothSub   0x1B54  /* [V] sub-object executed in state 2 */
#define daShip_OFF_evtCutIdx  0xD190  /* [V] s16, checked against 0 */
#define daShip_OFF_evtCutId   0xD192  /* [V] s16, checked against -1 */
#define daShip_OFF_evtCutFunc 0xD194  /* [V] fn ptr, compared to evtCutFn */

/* ========================================================================
 * Finding the boat
 *
 * Three routes. Use the first.
 *
 * 0. THE PLAY SLOT. dComIfG_play_c keeps the boat at play+0x489C, slot 2 of its
 *    actor-pointer array, and the game's own "is the boat there?" test is a
 *    NULL check on that slot. get_daShip() is that read. It needs no camera, no
 *    process search and no text delta, and it works whether or not Link is
 *    aboard. See the note in d_com_inf_game.h for how the slot was identified.
 *
 * 1. THE RIDE CAMERA. While Link is sailing, the camera runs its RIDE mode and
 *    stores the boat pointer in its own work block. daShip_getFromCamera()
 *    reads it: also a plain memory read. Useful as a cross-check, and it tells
 *    you the boat is actually being ridden rather than merely spawned.
 *
 *    Verified from the mode function itself, which sets up its work pointer as
 *    `addic. r30, r31, 0x37C` - the camera plus mModeFourCC - and then stores
 *    each search result to `0x4(r30)`, which is mModeWork + 0x00. That is
 *    exactly where dCam_rideWork_c puts mpShip, and it re-confirms mModeFourCC
 *    at 0x37C and mModeWork at 0x380 on the way past. All five of the mode's
 *    search paths write the same slot, so the pointer is refreshed every frame
 *    the mode runs.
 *
 *    The catch: it is only meaningful while the mode is RIDE, so the accessor
 *    checks mModeFourCC and returns NULL otherwise. It also needs a camera
 *    pointer, and dCamera_c is placement-constructed inside its owning actor
 *    at owner+0x248 rather than hanging off a global - so a tool gets one by
 *    keeping the pointer its own camera mode function is handed.
 *
 * 2. THE PROCESS SEARCH, daShip_searchByProc() below. Needs a game call, and
 *    is the only one that can return NULL for reasons you cannot see.
 * ===================================================================== */

/**
 * [V] The King of Red Lions, or NULL when it is not spawned.
 *
 * The cheap, always-available route: a read of the play slot. This is what the
 * game itself does before touching the boat.
 */
static __inline daShip_c* get_daShip(void) {
    return (daShip_c*)dComIfGp_getShipActor();
}

/** [V] Non-zero when the boat is spawned. */
static __inline int daShip_isAlive(void) {
    return get_daShip() != (daShip_c*)0;
}

/** [V] Non-zero when this camera is running the RIDE mode. */
static __inline int dCam_isRideMode(const dCamera_c* cam) {
    return cam && cam->mModeFourCC == dCam_MODE_RIDE;
}

/**
 * [V] The King of Red Lions, read out of the RIDE camera's work block, or NULL
 * when that camera is not in RIDE mode. Needs no game call.
 */
static __inline daShip_c* daShip_getFromCamera(const dCamera_c* cam) {
    wwhd_gptr_t p;
    if (!dCam_isRideMode(cam))
        return (daShip_c*)0;
    p = cam->mModeWork.ride.mpShip;
    if (!p)
        return (daShip_c*)0;
    return (daShip_c*)WWHD_AT(fopAc_ac_c, p);
}

/**
 * [V] The boat's sail speed constant, or NULL before a region is selected.
 *
 * The counterpart of daPy_getSwimSpeedPtr(), and for the same reason: the sail
 * speed is loaded from here rather than accumulated, so this is the field that
 * moves the boat and speedF is not.
 *
 * Stock value is WWHD_SAIL_SPEED_STOCK. Restore it when you are done - it is a
 * global constant, not per-save state.
 */
static __inline f32* daShip_getSailSpeedPtr(void) {
    if (!wwhd_regionResolved)
        return (f32*)0;
    return WWHD_AT_DATA(f32, wwhd_map->sailSpeedConst);
}

/** [V] The shipped value of the above, for restoring it. */
#define WWHD_SAIL_SPEED_STOCK 55.0f

/* ========================================================================
 * Position and facing
 *
 * daShip_c has no transform store of its own - unlike Link, whose position is
 * mirrored into three globals, the boat keeps its transform only in its actor.
 * That was checked rather than assumed: every .bss address the ship module
 * touches was enumerated and none is a transform mirror. So writing the actor
 * is the whole job here.
 * ===================================================================== */

/** [V] The boat's position. Returns non-zero on success. */
static __inline int daShip_getPosition(const daShip_c* ship, cXyz* out) {
    return ship ? fopAcM_getPosition(&ship->base, out) : 0;
}

/**
 * [V] Move the boat, writing `current` and `old` and clearing momentum, so the
 * move does not read as a frame of travel across the sea. Returns non-zero on
 * success.
 *
 * This does not move the collision the boat carries, or Link if he is standing
 * on it - it moves the actor. Expect to place Link separately.
 */
static __inline int daShip_setPosition(daShip_c* ship, const cXyz* pos) {
    return ship ? fopAcM_warpPosition(&ship->base, pos) : 0;
}

/** [V] The boat's facing angle, or 0 when unavailable. shape_angle.y is what
 *  the RIDE camera reads as the boat's heading. */
static __inline s16 daShip_getFacing(const daShip_c* ship) {
    return ship ? ship->base.shape_angle.y : (s16)0;
}

/** [V] Turn the boat. Returns non-zero on success. */
static __inline int daShip_setFacing(daShip_c* ship, s16 angle) {
    if (!ship)
        return 0;
    ship->base.shape_angle.y = angle;
    ship->base.current.angle.y = angle;
    return 1;
}

#ifdef WWHD_ENABLE_GAME_CALLS
/**
 * [V] Find the boat the way the RIDE camera does - a process search.
 *
 * Kept because it is a genuinely independent route, but prefer get_daShip():
 * this one needs a resolved text delta and can return NULL for reasons that are
 * not visible from outside.
 *
 * Needs to run inside the game with a resolved text delta - both the routine
 * and the search tag live in .text, which the console relocates. Returns NULL
 * rather than jumping into unmapped memory; calling it with an unresolved delta
 * is what crashed a retail console with an invalid instruction fetch.
 *
 * The MECHANISM is now confirmed against the RIDE camera's own call, which
 * builds the identical three parts: tag 0x025E121C, the key 0xA5 stored as a
 * u16 on the stack, and searchByID(tag, &key). libwwhd does the same thing
 * with the same values, so a NULL from this is a question about when the boat
 * is a searchable process, not about these constants. Do not "fix" them - and
 * prefer get_daShip(), which does not depend on any of it.
 */
static __inline daShip_c* daShip_searchByProc(void) {
    typedef void* (*fopAcM_searchByID_t)(void* tag, u16* procName);
    fopAcM_searchByID_t search;
    u16 key;
    if (!wwhd_textResolved || !wwhd_regionResolved)
        return (daShip_c*)0;
    search = WWHD_FN(fopAcM_searchByID_t, wwhd_map->fopAcM_searchByID);
    key = WWHD_PROC_SHIP;
    return (daShip_c*)search(WWHD_AT_TEXT(void, wwhd_map->tagProcSearch), &key);
}
#endif /* WWHD_ENABLE_GAME_CALLS */

#endif /* LIBWWHD_D_A_SHIP_H */
