#ifndef LIBWWHD_D_CC_S_H
#define LIBWWHD_D_CC_S_H

#include "libwwhd/wwhd_types.h"
#include "libwwhd/wwhd_region.h"
#include "libwwhd/d/d_com_inf_game.h"

/**
 * libwwhd - the hit collision system (c_cc_s.cpp / c_cc_d.cpp, dCcS on top)
 *
 * Every actor registers its attack, hurt and body shapes once per frame with
 * cCcS::Set (0x0200E240, byte-identical in all three builds, first word
 * 0x80040000). Set appends the object to one list per role it has the set
 * bit for, and to a fourth list of everything registered this frame. Move
 * (0x0200E558, also identical, first word 0x7C0802A6) resolves the frame:
 * ChkAtTg (0x0200D19C) fills the hit words, MoveCo (0x0200D684) pushes bodies
 * apart, and the four counts are zeroed at its end. So the lists are complete
 * on entry to Move and empty on exit; a viewer copies the pointers before it
 * and reads the objects after it.
 *
 * The instance is play + 0x1404, the GameCube dComIfG_play_c::mCcS, and the
 * play scene creates it there (0x025B1FF8 hands gameInfo + 0x26A4 to the
 * clear routine 0x02518760). Below WWHD_PLAY_TAIL_FROM, so the same on JAP.
 *
 * cCcD_Obj (the thing in the lists) keeps the GameCube layout. The three role
 * blocks are {set params, result params, hit partner} at +0x00, +0x18 and
 * +0x2C: cCcD_*SPrm_Set_e is bit 0 of the set word, cCcD_*RPrm_Hit_e bit 0
 * of the result word, both read straight out of Set and the hit setter
 * 0x0200CFA8. The shape attribute is a second base at +0x114 (its vtable
 * pointer) with the geometry data at +0x118: Link's hurt cylinder at
 * daPy_lk_c + 0x765C has its centre set at +0x118, radius at +0x124 and
 * height at +0x128 by setCollision (0x024076CC), the GameCube cM3dGCyl
 * order. The attribute vtable pointer is the only type tag; libwwhd does
 * not carry its per-build values, so a reader calibrates them from Link's
 * own members (see WWHD_DAPY_OFF_CC_* in d_a_player.h).
 */

#define WWHD_CCS_AT_MAX   256
#define WWHD_CCS_TG_MAX   768   /* [V] HD grew this from 256 */
#define WWHD_CCS_CO_MAX   256
#define WWHD_CCS_ALL_MAX  1280

typedef struct cCcS {
    /* 0x0000 */ wwhd_gptr_t mpObjAt[WWHD_CCS_AT_MAX];   /* [V] */
    /* 0x0400 */ wwhd_gptr_t mpObjTg[WWHD_CCS_TG_MAX];   /* [V] */
    /* 0x1000 */ wwhd_gptr_t mpObjCo[WWHD_CCS_CO_MAX];   /* [V] */
    /* 0x1400 */ wwhd_gptr_t mpObjAll[WWHD_CCS_ALL_MAX]; /* [V] every Set this frame */
    /* 0x2800 */ s32         mObjAtCount;                /* [V] */
    /* 0x2804 */ s32         mObjTgCount;                /* [V] */
    /* 0x2808 */ s32         mObjCoCount;                /* [V] */
    /* 0x280C */ s32         mObjAllCount;               /* [V] */
    /* 0x2810 */ u8          mMassMng[0x2850 - 0x2810];  /* [?] */
    /* 0x2850 */ wwhd_gptr_t vtbl;                       /* [V] dCcS vtable */
} cCcS;
WWHD_ASSERT_OFFSET(cCcS, mpObjAll,     0x1400);
WWHD_ASSERT_OFFSET(cCcS, mObjAtCount,  0x2800);
WWHD_ASSERT_OFFSET(cCcS, mObjAllCount, 0x280C);
WWHD_ASSERT_OFFSET(cCcS, vtbl,         0x2850);
WWHD_ASSERT_SIZE  (cCcS,               0x2854);

#define WWHD_PLAY_OFF_CCS 0x1404

/** [V] The hit collision system, or NULL before play exists. */
static __inline cCcS* dComIfG_Ccsp(void) {
    dComIfG_play_c* p = dComIfGp_getPlay();
    return p ? (cCcS*)((u8*)p + WWHD_PLAY_OFF_CCS) : (cCcS*)0;
}

#define WWHD_CCD_PRM_SET 0x1u /* [V] cCcD_*SPrm_Set_e */
#define WWHD_CCD_PRM_HIT 0x1u /* [V] cCcD_*RPrm_Hit_e */

/** [V] cM3dGCyl: bottom centre, radius, height along +Y. */
typedef struct cM3dGCyl_t { cXyz mCenter; f32 mRadius; f32 mHeight; } cM3dGCyl_t;
/** [V] cM3dGSph. */
typedef struct cM3dGSph_t { cXyz mCenter; f32 mRadius; } cM3dGSph_t;
/** [V] cM3dGCps: the GameCube cM3dGLin (start, end, its own vtable) then the radius. */
typedef struct cM3dGCps_t { cXyz mStart; cXyz mEnd; wwhd_gptr_t _vtbl; f32 mRadius; } cM3dGCps_t;
/** [P] cM3dGTri: the GameCube cM3dGPla (normal, d, vtable) then the corners. */
typedef struct cM3dGTri_t { cXyz mNormal; f32 mD; wwhd_gptr_t _vtbl; cXyz mA; cXyz mB; cXyz mC; } cM3dGTri_t;

typedef struct cCcD_Obj {
    /* 0x000 */ u32         mAtSPrm;    /* [V] bit 0 = registered as attack */
    /* 0x004 */ u32         mAtRPrm;    /* [V] bit 0 = hit something */
    /* 0x008 */ wwhd_gptr_t mAtHitObj;  /* [V] */
    /* 0x00C */ u8          _unk_00C[0x018 - 0x00C];
    /* 0x018 */ u32         mTgSPrm;    /* [V] bit 0 = registered as hurt */
    /* 0x01C */ u32         mTgRPrm;    /* [V] bit 0 = was hit */
    /* 0x020 */ wwhd_gptr_t mTgHitObj;  /* [V] */
    /* 0x024 */ u8          _unk_024[0x02C - 0x024];
    /* 0x02C */ u32         mCoSPrm;    /* [V] bit 0 = registered as body */
    /* 0x030 */ u32         mCoRPrm;    /* [V] bit 0 = pushed */
    /* 0x034 */ wwhd_gptr_t mCoHitObj;  /* [V] */
    /* 0x038 */ u8          _unk_038[0x03C - 0x038];
    /* 0x03C */ wwhd_gptr_t vtbl;       /* [V] cCcD_Obj vtable; +0x2C is GetShapeAttr */
    /* 0x040 */ u32         mFlags;     /* [V] tested as & 2 by ChkAtTg */
    /* 0x044 */ wwhd_gptr_t mpStts;     /* [V] cCcD_Stts*; its +0x0C is the owning actor */
    /* 0x048 */ u32         mRangeBits; /* [V] cCcD_DivideInfo, what ChkAtTg compares first */
    /* 0x04C */ u8          _unk_04C[0x0F8 - 0x04C]; /* [P] dCcD_GObjInf: the At block at
                                                       *     +0x50 (actor at +0x60), Tg at
                                                       *     +0x94 (actor +0xA4), Co at +0xDC */
    /* 0x0F8 */ cXyz        mAabMin;    /* [P] cM3dGAab inside the shape attribute */
    /* 0x104 */ cXyz        mAabMax;    /* [P] */
    /* 0x110 */ wwhd_gptr_t _aabVtbl;   /* [P] */
    /* 0x114 */ wwhd_gptr_t mAttrVtbl;  /* [V] the shape attribute's vtable: the type tag */
    /* 0x118 */ union {
                    cM3dGCyl_t cyl;
                    cM3dGSph_t sph;
                    cM3dGCps_t cps;
                    cM3dGTri_t tri;
                } mShape;               /* [V] cylinder; the rest by layout */
} cCcD_Obj;
WWHD_ASSERT_OFFSET(cCcD_Obj, mTgSPrm,   0x018);
WWHD_ASSERT_OFFSET(cCcD_Obj, mCoSPrm,   0x02C);
WWHD_ASSERT_OFFSET(cCcD_Obj, vtbl,      0x03C);
WWHD_ASSERT_OFFSET(cCcD_Obj, mpStts,    0x044);
WWHD_ASSERT_OFFSET(cCcD_Obj, mAabMin,   0x0F8);
WWHD_ASSERT_OFFSET(cCcD_Obj, mAttrVtbl, 0x114);
WWHD_ASSERT_OFFSET(cCcD_Obj, mShape,    0x118);

#endif /* LIBWWHD_D_CC_S_H */
