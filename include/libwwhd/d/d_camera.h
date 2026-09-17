#ifndef LIBWWHD_D_CAMERA_H
#define LIBWWHD_D_CAMERA_H

#include "libwwhd/wwhd_types.h"
#include "libwwhd/wwhd_region.h"
#include "libwwhd/f_op/f_op_actor.h"

/**
 * libwwhd - camera (d_camera.cpp)
 *
 * dCamera_c holds the transform twice: a committed block the renderer reads and
 * a working block the active mode function writes. Writing either from a
 * present-time hook is undone within a frame; owning the camera means replacing
 * the mode function in mAlgTable. docs/SUBSYSTEMS.md (Camera) explains why, and is worth
 * reading before building anything on this header.
 *
 * Owning the mode function is not the end of Run, either. Its tail holds the
 * committed center above the floor under the player and, when the style asks,
 * lifts the eye to the ground or water under it - see mFloorY, mCalcFlags and
 * dCamera_style_c::mFlags for the three things a mode function has to write
 * to take the camera below either.
 */

/** [V] Spherical direction: radius plus two short angles. */
typedef struct cSGlobe {
    /* 0x0 */ f32 mRadius;
    /* 0x4 */ s16 mTheta;
    /* 0x6 */ s16 mPhi;
} cSGlobe;
WWHD_ASSERT_SIZE(cSGlobe, 0x8);

/** [V] Mode FourCC written into dCamera_c::mModeFourCC. */
#define dCam_MODE_RIDE 0x52494445u   /* 'RIDE' */

/**
 * Algorithm ids indexing mAlgTable.
 *
 * Every SLOT is verified: the table was dumped and all 20 entries carry
 * {0x0000, 0xFFFF, fn} with 20 distinct, non-null mode functions, and entry 20
 * is visibly not of that shape. Two of them are cross-checked against map slots
 * this library already resolved independently - entry 8's function is
 * dCam_rideCamera and entry 19's is dCam_demoCamera.
 *
 * The NAMES are GameCube's. A stripped binary carries no symbols, so a name can
 * only be confirmed by working out what its mode function does; that has been
 * done for RIDE and DEMO and nothing else. Each id below carries its verified
 * USA mode-function address, which is the way in if you want to name one.
 */
enum dCamAlg_e {
    dCamAlg_LET            = 0,   /* [V] slot; name [I]. fn 0x02502424 */
    dCamAlg_FOLLOW         = 1,   /* [V] slot; name [I]. fn 0x025028B8 */
    dCamAlg_LOCKON         = 2,   /* [V] slot; name [I]. fn 0x025052E8 */
    dCamAlg_TALKTO         = 3,   /* [V] slot; name [I]. fn 0x02508A60 */
    dCamAlg_SUBJECT        = 4,   /* [V] slot; name [I]. fn 0x025071FC */
    dCamAlg_FIXED_POSITION = 5,   /* [V] slot; name [I]. fn 0x0250CC80 */
    dCamAlg_FIXED_FRAME    = 6,   /* [V] slot; name [I]. fn 0x0250C77C */
    dCamAlg_TOWER          = 7,   /* [V] slot; name [I]. fn 0x0250B30C */
    dCamAlg_RIDE           = 8,   /* [V] the boat camera. fn 0x0250D4E8,
                                   *     which is the dCam_rideCamera slot */
    dCamAlg_HUNG           = 9,   /* [V] slot; name [I]. fn 0x0250EF98 */
    dCamAlg_MANUAL         = 10,  /* [V] slot; name [I]. fn 0x0250FDC8 */
    dCamAlg_EVENT          = 11,  /* [V] slot; name [I]. fn 0x024FF164 */
    dCamAlg_CRAWL          = 12,  /* [V] slot; name [I]. fn 0x02511200 */
    dCamAlg_HOOKSHOT       = 13,  /* [V] slot; name [I]. fn 0x02511B5C */
    dCamAlg_TORNADO        = 14,  /* [V] slot; name [I]. fn 0x025121E8 */
    dCamAlg_VOMIT          = 15,  /* [V] slot; name [I]. fn 0x02513518 */
    dCamAlg_SHIELD         = 16,  /* [V] slot; name [I]. fn 0x02513E98 */
    dCamAlg_NON_OWNER      = 17,  /* [V] slot; name [I]. fn 0x0251485C */
    dCamAlg_FOLLOW2        = 18,  /* [V] slot; name [I]. fn 0x02514CFC */
    dCamAlg_DEMO           = 19,  /* [V] last entry; Run calls it directly.
                                   *     fn 0x024FC0A8 = dCam_demoCamera */
    dCamAlg_MAX            = 20   /* [V] table length */
};

/** [V] RIDE mode work block, overlaying mModeWork. Read from dCam_rideCamera. */
typedef struct dCam_rideWork_c {
    /* 0x00 */ wwhd_gptr_t mpShip;      /* [V] fopAc_ac_c*, the asserted wk->ship */
    /* 0x04 */ u8          _unk_04[4];  /* [?] */
    /* 0x08 */ u8          mHighAngle;  /* [V] set when pitch exceeds a limit */
    /* 0x09 */ u8          mNeedReset;  /* [V] */
    /* 0x0A */ u8          _unk_0A[2];  /* [?] */
    /* 0x0C */ s32         mSubMode;    /* [V] 0/1/2 select the blend path */
    /* 0x10 */ cXyz        mLookAt;     /* [V] fed to the collision probe */
    /* 0x1C */ cXyz        mOffset;     /* [V] zeroed on mode entry */
    /* 0x28 */ cXyz        mEyeTarget;  /* [V] spring target for the eye */
    /* 0x34 */ s16         mYawWork;    /* [V] ship shape_angle.y + mCamYawOfs */
    /* 0x36 */ u8          _unk_36[2];  /* [?] */
    /* 0x38 */ f32         mSpring0;    /* [V] four lerped spring coefficients */
    /* 0x3C */ f32         mSpring1;    /* [V] */
    /* 0x40 */ f32         mSpring2;    /* [V] */
    /* 0x44 */ f32         mSpring3;    /* [V] */
    /* 0x48 */ f32         mPitchBlend; /* [V] */
    /* 0x4C */ f32         mDistBlend;  /* [V] */
} dCam_rideWork_c;
WWHD_ASSERT_SIZE(dCam_rideWork_c, 0x50);

/**
 * [V] dCamParam_c - the camera's handle on the style record it is running.
 *
 * dCamParam_c::Change (0x024F727C) stores the index at +0x8 and the record's
 * address at +0x4, computed as camStyleTable + idx * 0x84 after a bounds check
 * against camStyleCount; +0x0 it never touches, which is where a vtable sits.
 * Run reads the record back through +0x4 (dCamera_c +0x8A8) after the mode
 * function has returned.
 */
typedef struct dCamParam_c {
    /* 0x0 */ wwhd_gptr_t _unk_0;    /* [?] vtable, most likely */
    /* 0x4 */ wwhd_gptr_t mpStyle;   /* [V] dCamera_style_c* */
    /* 0x8 */ s32         mStyleNo;  /* [V] */
} dCamParam_c;
WWHD_ASSERT_OFFSET(dCamParam_c, mpStyle,  0x4);
WWHD_ASSERT_OFFSET(dCamParam_c, mStyleNo, 0x8);

/*
 * The real object is 0x8E0 bytes - its constructor allocates exactly that.
 * What follows is a verified PREFIX, not the whole thing, so there is no size
 * assertion and you must not allocate one.
 *
 * The first three members are read from that constructor, which zeroes a word
 * at +0x00 and bytes at +0x04 and +0x05 before initialising +0x08 and +0x38 -
 * two members this header already had verified, which is what ties the opening
 * to this struct rather than some other object.
 */
typedef struct dCamera_c {
    /* 0x000 */ wwhd_gptr_t mpCamera;      /* [P] word, NULLed by the ctor */
    /* 0x004 */ u8          mActive;       /* [P] byte, zeroed by the ctor */
    /* 0x005 */ u8          mPause;        /* [P] byte, zeroed by the ctor;
                                            *     two stb sites elsewhere    */
    /* 0x006 */ s16         _unk_006;      /* [?] */

    /* Committed transform - what the renderer consumes. */
    /* 0x008 */ cSGlobe     mDirection;    /* [V] */
    /* 0x010 */ cXyz        mCenter;       /* [V] look-at target */
    /* 0x01C */ cXyz        mEye;          /* [V] camera position */
    /* 0x028 */ cXyz        mUp;           /* [V] written as a group in Run */
    /* 0x034 */ s16         mBank;         /* [V] roll */
    /* 0x036 */ u8          _unk_036[2];   /* [?] */
    /* 0x038 */ f32         mFovy;         /* [V] */

    /* Working transform - what the active mode function produces. */
    /* 0x03C */ cSGlobe     mWorkDirection;/* [V] built from eye - center */
    /* 0x044 */ cXyz        mWorkCenter;   /* [V] -> mCenter */
    /* 0x050 */ cXyz        mWorkEye;      /* [V] -> mDirection -> mEye */
    /* 0x05C */ s16         mWorkBank;     /* [V] -> mBank */
    /* 0x05E */ u8          _unk_05E[2];   /* [?] */
    /* 0x060 */ f32         mWorkFovy;     /* [V] -> mFovy, scaled */

    /* 0x064 */ u8          _unk_064[0x068 - 0x064]; /* [?] */
    /* 0x068 */ u32         mCalcFlags;    /* [V] Run sets 9 before the mode
                                            *     function, lets the style's
                                            *     flags raise it after, then
                                            *     hands it to dCam_calcTrans.
                                            *     Bit 3 is dCamCalc_SURFACE. */
    /* 0x06C */ s16         _unk_06C;      /* [V] written from an angle */
    /* 0x06E */ u8          _unk_06E[0x07C - 0x06E]; /* [?] */
    /* 0x07C */ u32         mFrameCounter; /* [V] incremented each Run */
    /* 0x080 */ u8          _unk_080[0x100 - 0x080]; /* [?] */
    /* 0x100 */ u8          mDirtyFlag0;   /* [V] the three are tested together.
                                            *     onModeChange zeroes them; a mode
                                            *     function raises them once its
                                            *     blend-in from the previous view
                                            *     is complete */
    /* 0x101 */ u8          mDirtyFlag1;   /* [V] */
    /* 0x102 */ u8          mDirtyFlag2;   /* [V] */
    /* 0x103 */ u8          _unk_103[0x108 - 0x103]; /* [?] */
    /* 0x108 */ s32         mModeFrame;    /* [V] frames since the mode was
                                            *     picked. onModeChange
                                            *     (0x024FA79C) zeroes it and Run
                                            *     increments it AFTER the
                                            *     dispatch, so a mode function
                                            *     sees 0 exactly once and only
                                            *     then initialises mModeWork.
                                            *     See dCam_restartMode(). */
    /* 0x10C */ s32         _unk_10C;      /* [V] zeroed by onModeChange */
    /* 0x110 */ u8          mModeBlendIn;  /* [V] onModeChange sets 1; 0 makes
                                            *     the follow camera snap to its
                                            *     target instead of blending */
    /* 0x111 */ u8          _unk_111[0x11C - 0x111]; /* [?] */
    /* 0x11C */ s32         mModeTimer;    /* [V] blend-in counter */
    /* 0x120 */ s32         mPlayerIdx;    /* [V] scales by 0x34 into play */
    /* 0x124 */ u32         mTargetActorID;/* [V] actor the camera follows */
    /* 0x128 */ u8          _unk_128[0x14C - 0x128]; /* [?] */
    /* 0x14C */ f32         mSightRadius;  /* [V] lock-on sight radius, smoothed
                                            *     in dCam_calcTrans; zeroed by
                                            *     onModeChange */
    /* 0x150 */ u8          _unk_150[0x158 - 0x150]; /* [?] */
    /* 0x158 */ f32         mBlendRate;    /* [V] */
    /* 0x15C */ u8          _unk_15C[0x23C - 0x15C]; /* [?] */
    /* 0x23C */ f32         mFovyBase;     /* [V] divided to form a ratio */
    /* 0x240 */ f32         mDistCur;      /* [V] */
    /* 0x244 */ u8          _unk_244[0x318 - 0x244]; /* [?] */
    /* 0x318 */ u8          mSeaArea;      /* [V] the player is over the sea */
    /* 0x319 */ u8          _unk_319[0x31C - 0x319]; /* [?] */
    /* 0x31C */ f32         mSeaWaveY;     /* [V] wave height under the player */
    /* 0x320 */ u8          _unk_320[0x358 - 0x320]; /* [?] */
    /* 0x358 */ f32         mFloorY;       /* [V] ground under the player, or
                                            *     the wave when that is higher;
                                            *     Run holds mCenter.y at or
                                            *     above this + mFloorMargin */
    /* 0x35C */ u8          _unk_35C[0x37C - 0x35C]; /* [?] */
    /* 0x37C */ u32         mModeFourCC;   /* [V] active mode id */
    /* 0x380 */ union {
                    dCam_rideWork_c ride;  /* [V] */
                    u8              raw[0x510 - 0x380];
                } mModeWork;               /* [V] */
    /* 0x510 */ u32         mFlags;        /* [V] 0x400 set on arrival, bit 15
                                            *     forces sub-mode 2, bit 20
                                            *     forces the dirty flags */
    /* 0x514 */ s32         mStyleIdx;     /* [V] indexes the style table */
    /* 0x518 */ u8          _unk_518[0x844 - 0x518]; /* [?] */
    /* 0x844 */ f32         mFloorMargin;  /* [V] dCamBGChk_c::mFloorMargin,
                                            *     32.0 on GameCube */
    /* 0x848 */ u8          _unk_848[0x8A4 - 0x848]; /* [?] */
    /* 0x8A4 */ dCamParam_c mParam;        /* [V] the style Run reads back */
} dCamera_c;
WWHD_ASSERT_OFFSET(dCamera_c, mDirection,     0x008);
WWHD_ASSERT_OFFSET(dCamera_c, mCenter,        0x010);
WWHD_ASSERT_OFFSET(dCamera_c, mEye,           0x01C);
WWHD_ASSERT_OFFSET(dCamera_c, mUp,            0x028);
WWHD_ASSERT_OFFSET(dCamera_c, mBank,          0x034);
WWHD_ASSERT_OFFSET(dCamera_c, mFovy,          0x038);
WWHD_ASSERT_OFFSET(dCamera_c, mWorkDirection, 0x03C);
WWHD_ASSERT_OFFSET(dCamera_c, mWorkCenter,    0x044);
WWHD_ASSERT_OFFSET(dCamera_c, mWorkEye,       0x050);
WWHD_ASSERT_OFFSET(dCamera_c, mWorkBank,      0x05C);
WWHD_ASSERT_OFFSET(dCamera_c, mWorkFovy,      0x060);
WWHD_ASSERT_OFFSET(dCamera_c, mCalcFlags,     0x068);
WWHD_ASSERT_OFFSET(dCamera_c, mDirtyFlag0,    0x100);
WWHD_ASSERT_OFFSET(dCamera_c, mModeFrame,     0x108);
WWHD_ASSERT_OFFSET(dCamera_c, mModeBlendIn,   0x110);
WWHD_ASSERT_OFFSET(dCamera_c, mModeTimer,     0x11C);
WWHD_ASSERT_OFFSET(dCamera_c, mSightRadius,   0x14C);
WWHD_ASSERT_OFFSET(dCamera_c, mPlayerIdx,     0x120);
WWHD_ASSERT_OFFSET(dCamera_c, mTargetActorID, 0x124);
WWHD_ASSERT_OFFSET(dCamera_c, mModeFourCC,    0x37C);
WWHD_ASSERT_OFFSET(dCamera_c, mModeWork,      0x380);
WWHD_ASSERT_OFFSET(dCamera_c, mFlags,         0x510);
WWHD_ASSERT_OFFSET(dCamera_c, mStyleIdx,      0x514);
WWHD_ASSERT_OFFSET(dCamera_c, mSeaArea,       0x318);
WWHD_ASSERT_OFFSET(dCamera_c, mSeaWaveY,      0x31C);
WWHD_ASSERT_OFFSET(dCamera_c, mFloorY,        0x358);
WWHD_ASSERT_OFFSET(dCamera_c, mFloorMargin,   0x844);
WWHD_ASSERT_OFFSET(dCamera_c, mParam,         0x8A4);

/*
 * [V] What Run (0x024FE3E8) does to the working block after the mode function
 * returns, read straight out of its tail:
 *
 *   1. mCalcFlags was set to 9 before the dispatch. Now the style's flag word
 *      is read through mParam.mpStyle: dCamStyleFlag_BUMP_FULL makes it 0x3F,
 *      else dCamStyleFlag_BUMP_BASIC makes it 0x0F, and
 *      dCamStyleFlag_LOCKON_SIGHT ors in 0x40.
 *   2. mCenter.y = max(mWorkCenter.y, mFloorY + mFloorMargin), where
 *      checkGroundInfo (0x024F9044) filled mFloorY at the top of Run from the
 *      ground under the PLAYER, raised to daSea_calcWave where he is at sea.
 *   3. dCam_calcTrans (0x024FD11C, the GameCube bumpCheck) builds mEye from
 *      the working block; with dCamCalc_SURFACE set it then asks
 *      getWaterSurfaceHeight (0x024FCEEC: ground under the eye + 5, or the
 *      wave + 20 at sea) and lifts mEye.y to it, rebuilding mDirection from
 *      mEye - mCenter.
 *
 * So a mode function that wants the eye under the surface writes three things
 * from inside the call, where they land between Run's reset and Run's read:
 * mCalcFlags = 0, mFloorY far below anything, and the two bump bits cleared
 * in the style record for as long as it holds the camera. mCalcFlags and
 * mFloorY are recomputed every Run and need no restore; the record is .rodata
 * and global, so it does.
 *
 * [V] Handing the camera back is not free either. Run picks the type, mode and
 * style BEFORE the dispatch, and a change there runs onModeChange (0x024FA79C):
 * mModeFrame = 0, the three dirty flags = 0, _unk_10C = 0, mModeBlendIn = 1,
 * mSightRadius = 0 and mFlags &= ~0x211E. The game's mode function then
 * initialises its mModeWork block on the one dispatch where mModeFrame reads 0,
 * because Run increments the counter after the call returns. A replacement
 * that answers that dispatch itself eats the init: the next frame the real
 * function finds mModeFrame = 1 and the dirty flags clear, takes its blend-in
 * path, and divides by a blend length it never computed. The follow camera
 * (0x025028B8) does exactly that with the float at +0x384, and the NaN it
 * produces reaches dCam_calcTrans, whose cM3dGSph constructor asserts on it
 * (c_m3d_g_sph.cpp:31). A replacement therefore restarts the mode and calls the
 * real function on its last dispatch - see dCam_restartMode().
 */
#define dCamCalc_SURFACE 0x8u   /* [V] lift the eye to the ground or water */

/**
 * [V] What onModeChange (0x024FA79C) does to the camera, minus the per-mode
 * extras (mode 14 resets the view, mode 3 clears an attention bit, and a
 * follow <-> lock-on toggle within one style zeroes mModeBlendIn). Call it
 * before dispatching the real mode function to make it start over from the
 * working eye and center as if the mode had just been selected.
 */
static __inline void dCam_restartMode(dCamera_c* cam) {
    cam->mSightRadius = 0.0f;
    cam->mDirtyFlag0 = 0;
    cam->mDirtyFlag1 = 0;
    cam->mDirtyFlag2 = 0;
    cam->mModeFrame = 0;
    cam->_unk_10C = 0;
    cam->mModeBlendIn = 1;
    cam->mFlags &= ~0x211Eu;
}

/** [V] The mode function's signature: Run passes the camera and mStyleIdx. */
typedef int (*dCam_modeFn_t)(dCamera_c* cam, int styleIdx);

/**
 * [V] One style record - the GameCube dCamera__Style: a four-character name,
 * the algorithm it dispatches to, thirty tuning floats and a flag word. Run
 * does algTable[style.mAlgorithm] for the style at mStyleIdx.
 *
 * The table starts at wwhd_map->camStyleTable and has wwhd_map->camStyleCount
 * records; both come from dCamParam_c::Change (0x024F727C), and the first
 * record reads "NN00" in all three builds. This header used to put the table
 * four bytes later, on the algorithm word - the word Run reads, so the mode
 * lookup was right and everything else in the record was off by one field.
 */
typedef struct dCamera_style_c {
    /* 0x00 */ u32 mName;           /* [V] four characters, e.g. "FN08" */
    /* 0x04 */ s32 mAlgorithm;      /* [V] index into the algorithm table */
    /* 0x08 */ f32 mParam[30];      /* [V] tuning; -100000.0 means unset */
    /* 0x80 */ u16 mFlags;          /* [V] dCamStyleFlag_* */
    /* 0x82 */ u8  _pad_82[2];      /* [?] */
} dCamera_style_c;
WWHD_ASSERT_OFFSET(dCamera_style_c, mAlgorithm, 0x04);
WWHD_ASSERT_OFFSET(dCamera_style_c, mParam,     0x08);
WWHD_ASSERT_OFFSET(dCamera_style_c, mFlags,     0x80);
WWHD_ASSERT_SIZE(dCamera_style_c, 0x84);

/* [V] Style flag bits Run acts on after the mode function; see dCamera_c. The
 * GameCube names are dCamPrmFlg_UNK001/002/400, so these are named by effect.
 * FN08, the ordinary follow camera, carries BUMP_FULL; most styles carry one of
 * the first two. */
#define dCamStyleFlag_BUMP_FULL    0x0001u  /* mCalcFlags := 0x3F */
#define dCamStyleFlag_BUMP_BASIC   0x0002u  /* mCalcFlags := 0x0F, if FULL is clear */
#define dCamStyleFlag_LOCKON_SIGHT 0x0400u  /* mCalcFlags |= 0x40 */

/**
 * [V] One algorithm-table record.
 *
 * mVtableIndex is always -1, so Run dispatches through mFn directly. That makes
 * taking the camera a pure data patch: no branch range limit, no trampoline,
 * and reversible by restoring the saved word.
 */
typedef struct dCamera_algEntry_c {
    /* 0x0 */ s16         mThisOffset;  /* [V] always 0 */
    /* 0x2 */ s16         mVtableIndex; /* [V] always -1 = plain fn pointer */
    /* 0x4 */ wwhd_gptr_t mFn;          /* [V] the mode function */
} dCamera_algEntry_c;
WWHD_ASSERT_OFFSET(dCamera_algEntry_c, mFn, 0x4);
WWHD_ASSERT_SIZE  (dCamera_algEntry_c, 0x8);

/** [V] The algorithm table, or NULL before a region is selected. Lives in
 *  .data, so it needs the runtime data delta. */
static __inline dCamera_algEntry_c* dCam_getAlgTable(void) {
    if (!wwhd_regionResolved)
        return (dCamera_algEntry_c*)0;
    return WWHD_AT_DATA(dCamera_algEntry_c, wwhd_map->camAlgTable);
}

/** [V] The algorithm table's LINK-TIME address, or 0 before a region is
 *  selected. For a writer that relocates addresses itself, such as a data
 *  swap; anything that dereferences wants dCam_getAlgTable(). */
static __inline wwhd_addr_t dCam_getAlgTableAddr(void) {
    return wwhd_regionResolved ? wwhd_map->camAlgTable : 0u;
}

/** [V] The style table, or NULL before a region is selected. */
static __inline dCamera_style_c* dCam_getStyleTable(void) {
    if (!wwhd_regionResolved)
        return (dCamera_style_c*)0;
    return WWHD_AT_DATA(dCamera_style_c, wwhd_map->camStyleTable);
}

/** [V] How many records the style table has (145), or 0 before a region is
 *  selected. */
static __inline s32 dCam_getStyleCount(void) {
    if (!wwhd_regionResolved)
        return 0;
    return *WWHD_AT_DATA(s32, wwhd_map->camStyleCount);
}

/** [V] One style record, or NULL if `idx` is out of range. */
static __inline dCamera_style_c* dCam_getStyle(int idx) {
    dCamera_style_c* t = dCam_getStyleTable();
    if (!t || idx < 0 || idx >= dCam_getStyleCount())
        return (dCamera_style_c*)0;
    return &t[idx];
}

/**
 * [V] The record the camera is running, or NULL. Taken from the camera's own
 * pointer rather than mStyleIdx because that pointer is what Run's tail reads
 * the flag word through.
 */
static __inline dCamera_style_c* dCam_getCurStyle(dCamera_c* cam) {
    if (!cam || !cam->mParam.mpStyle)
        return (dCamera_style_c*)0;
    return WWHD_AT(dCamera_style_c, cam->mParam.mpStyle);
}

/** [V] One algorithm slot, or NULL if `alg` is out of range. */
static __inline dCamera_algEntry_c* dCam_getAlgEntry(int alg) {
    dCamera_algEntry_c* t = dCam_getAlgTable();
    if (!t || alg < 0 || alg >= dCamAlg_MAX)
        return (dCamera_algEntry_c*)0;
    return &t[alg];
}

/* ------------------------------------------------------------------------- */
/* The camera process around dCamera_c                                        */

/**
 * [V] dCamera_c is a member of the camera process (the GameCube
 * camera_process_class): camera_execute (USA 0x024FFA3C, EUR 0x024FFA40,
 * JAP 0x024FFA44) calls Run on this + 0x248 in all three builds.
 *
 * The process also carries the view record the renderer consumes, the
 * GameCube view_class shifted up by 4: near at +0xCC, far at +0xD0, fovy in
 * degrees at +0xD4, aspect at +0xD8, eye at +0xDC, center at +0xE8, up at
 * +0xF4, bank at +0x100 and a GL-style perspective at +0x104. view_setup
 * (0x024F8430/34/38, run right after Run) fills eye, center, up, fovy and
 * bank from dCamera_c - or from the demo camera when one is active, which is
 * why a viewer reads this record rather than the dCamera_c members - and
 * takes near and far from the stage. camera_draw (0x024FFC40/44/48) builds
 * +0x104 from +0xD4/+0xD8/+0xCC/+0xD0 (0x028E9948: m22 = -(f+n)/(f-n),
 * m23 = -2fn/(f-n), m32 = -1) and hands the same four values, fovy in
 * radians, to a projection object at +0x8B4 (0x0274E04C: dirty byte at +0,
 * near +0x94, far +0x98, aspect +0xAC). That object's device matrix at +0x44
 * (returned by 0x0274D83C after the dirty update through the vtable at
 * +0x90, which builds +0x04 and then derives +0x44 from it) is what the draw
 * copies into the renderer's global projection (USA 0x104B470C), so it is
 * the projection the depth buffer was written with, device z range included.
 */
#define WWHD_CAMPROC_OFF_DCAMERA   0x248
#define WWHD_CAMPROC_OFF_VIEW      0x0CC
#define WWHD_CAMPROC_OFF_PROJ_OBJ  0x8B4
#define WWHD_PROJ_OFF_DEVICE_MTX   0x044

typedef struct dCam_view_t {
    /* 0x00 */ f32  mNear;        /* [V] the stage's near plane */
    /* 0x04 */ f32  mFar;         /* [V] the stage's far plane */
    /* 0x08 */ f32  mFovy;        /* [V] degrees, after the demo override */
    /* 0x0C */ f32  mAspect;      /* [V] a constant, 16:9 */
    /* 0x10 */ cXyz mEye;         /* [V] */
    /* 0x1C */ cXyz mCenter;      /* [V] */
    /* 0x28 */ cXyz mUp;          /* [V] */
    /* 0x34 */ s16  mBank;        /* [V] */
    /* 0x36 */ u8   _pad_36[2];
    /* 0x38 */ f32  mProjMtx[16]; /* [V] row-major, GL-style z */
} dCam_view_t;
WWHD_ASSERT_OFFSET(dCam_view_t, mEye,     0x10);
WWHD_ASSERT_OFFSET(dCam_view_t, mBank,    0x34);
WWHD_ASSERT_OFFSET(dCam_view_t, mProjMtx, 0x38);
WWHD_ASSERT_SIZE  (dCam_view_t, 0x78);

/** [V] The camera process a dCamera_c lives in, or NULL. */
static __inline u8* dCam_getProcess(dCamera_c* cam) {
    return cam ? (u8*)cam - WWHD_CAMPROC_OFF_DCAMERA : (u8*)0;
}

/** [V] The renderer's view record inside a camera process, or NULL. */
static __inline const dCam_view_t* dCam_getView(const void* process) {
    return process ? (const dCam_view_t*)((const u8*)process + WWHD_CAMPROC_OFF_VIEW)
                   : (const dCam_view_t*)0;
}

/** [V] The device projection matrix (16 floats, row-major) the scene is drawn
 *  with, inside a camera process, or NULL. Rows 2 and 3 give window depth:
 *  z_clip = m[10] * z_eye + m[11], w_clip = m[14] * z_eye + m[15]. */
static __inline const f32* dCam_getDeviceProjMtx(const void* process) {
    return process ? (const f32*)((const u8*)process + WWHD_CAMPROC_OFF_PROJ_OBJ +
                                  WWHD_PROJ_OFF_DEVICE_MTX)
                   : (const f32*)0;
}

#endif /* LIBWWHD_D_CAMERA_H */
