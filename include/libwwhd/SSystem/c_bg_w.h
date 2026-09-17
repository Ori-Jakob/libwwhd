#ifndef LIBWWHD_C_BG_W_H
#define LIBWWHD_C_BG_W_H

#include "libwwhd/wwhd_types.h"
#include "libwwhd/wwhd_region.h"

/**
 * libwwhd - collision geometry data (c_bg_w.cpp / c_bg_s.cpp)
 *
 * cBgD_t is the header of a loaded collision file: six count/pointer pairs and
 * a flag. It was read in one shot from cBgD_t::Initialize, which relocates each
 * file-relative offset to an absolute pointer and asserts the alignment of every
 * table on the way, naming all six.
 *
 * The registry is mapped too - see cBgS_ChkElm, which is what
 * dComIfG_play_c::mBgS opens with - and the ground query is available via
 * dBgS_getGroundY() below.
 *
 * dBgS_GndChk's own layout is still NOT mapped, and deliberately does not need
 * to be: the game's ground helper takes a bare cXyz* and constructs the chk on
 * its own stack with its own constructor, so libwwhd never has to describe a
 * structure it has not verified.
 */

/** [V] A collision vertex. The GameCube cBgD_Vtx_t is a bare Vec. */
typedef struct cBgD_Vtx_t {
    /* 0x0 */ f32 x;
    /* 0x4 */ f32 y;
    /* 0x8 */ f32 z;
} cBgD_Vtx_t;
WWHD_ASSERT_SIZE(cBgD_Vtx_t, 0xC);

/**
 * [V] One collision group. Size 0x34 is measured, not assumed: Initialize
 * walks m_g_tbl with `iVar2 += 0x34` for m_g_num iterations, relocating the
 * pointer at each entry's +0x00 - which is m_name.
 *
 * The tree half of the struct was read out of WWHD group walkers: parent is
 * followed upward, next_sibling drives the sibling loop, first_child drives the
 * recursion, room_id is what the group-to-room lookup returns, tree_idx gates
 * on 0xFFFF and indexes the 0x1C-stride tree table, and info is returned whole.
 *
 * The TRANSFORM half is not read anywhere in WWHD. Every group walker in the
 * collision module goes through the pre-baked tree table at m_tree_tbl instead,
 * so scale, rotation, translation and first_vtx_idx keep the GameCube layout
 * with no WWHD sighting behind them.
 */
typedef struct cBgD_Grp_t {
    /* 0x00 */ wwhd_gptr_t m_name;           /* [V] char*, relocated on load */
    /* 0x04 */ f32   m_scale[3];             /* [I] never read */
    /* 0x10 */ s16   m_rotation[3];          /* [I] never read */
    /* 0x16 */ u8    _pad_16[2];             /* [?] */
    /* 0x18 */ f32   m_translation[3];       /* [I] never read */
    /* 0x24 */ u16   m_parent;               /* [V] walked upward, 0xFFFF end */
    /* 0x26 */ u16   m_next_sibling;         /* [V] sibling loop, 0xFFFF end */
    /* 0x28 */ u16   m_first_child;          /* [V] recursion entry */
    /* 0x2A */ u16   m_room_id;              /* [V] what the group-to-room
                                              *     lookup returns */
    /* 0x2C */ u16   m_first_vtx_idx;        /* [I] never read */
    /* 0x2E */ u16   m_tree_idx;             /* [V] 0xFFFF gate; indexes the
                                              *     0x1C-stride tree table */
    /* 0x30 */ u32   m_info;                 /* [V] returned whole as a u32 */
} cBgD_Grp_t;
WWHD_ASSERT_OFFSET(cBgD_Grp_t, m_name,         0x00);
WWHD_ASSERT_OFFSET(cBgD_Grp_t, m_parent,       0x24);
WWHD_ASSERT_OFFSET(cBgD_Grp_t, m_next_sibling, 0x26);
WWHD_ASSERT_OFFSET(cBgD_Grp_t, m_first_child,  0x28);
WWHD_ASSERT_OFFSET(cBgD_Grp_t, m_room_id,      0x2A);
WWHD_ASSERT_OFFSET(cBgD_Grp_t, m_tree_idx,     0x2E);
WWHD_ASSERT_OFFSET(cBgD_Grp_t, m_info,         0x30);
WWHD_ASSERT_SIZE  (cBgD_Grp_t,         0x34);

/**
 * [V] Collision file header. Every field confirmed from Initialize, which
 * asserts each table's alignment by name and then relocates it:
 *
 *     m_v_tbl    % 4 == 0     m_tree_tbl % 2 == 0
 *     m_t_tbl    % 2 == 0     m_g_tbl    % 4 == 0
 *     m_b_tbl    % 2 == 0     m_ti_tbl   % 4 == 0
 *
 * m_g_num is confirmed as the count by being the relocation loop's bound, and
 * bit 31 of flag is the already-initialised guard the whole routine is wrapped
 * in. The layout is identical to GameCube.
 */
typedef struct cBgD_t {
    /* 0x00 */ s32         m_v_num;     /* [V] */
    /* 0x04 */ wwhd_gptr_t m_v_tbl;     /* [V] cBgD_Vtx_t*  */
    /* 0x08 */ s32         m_t_num;     /* [V] */
    /* 0x0C */ wwhd_gptr_t m_t_tbl;     /* [V] cBgD_Tri_t*  */
    /* 0x10 */ s32         m_b_num;     /* [V] */
    /* 0x14 */ wwhd_gptr_t m_b_tbl;     /* [V] cBgD_Blk_t*  */
    /* 0x18 */ s32         m_tree_num;  /* [V] */
    /* 0x1C */ wwhd_gptr_t m_tree_tbl;  /* [V] cBgD_Tree_t* */
    /* 0x20 */ s32         m_g_num;     /* [V] relocation loop bound */
    /* 0x24 */ wwhd_gptr_t m_g_tbl;     /* [V] cBgD_Grp_t*, stride 0x34 */
    /* 0x28 */ s32         m_ti_num;    /* [V] */
    /* 0x2C */ wwhd_gptr_t m_ti_tbl;    /* [V] cBgD_Ti_t*   */
    /* 0x30 */ u32         flag;        /* [V] bit 31 = initialised */
} cBgD_t;
WWHD_ASSERT_OFFSET(cBgD_t, m_v_tbl,   0x04);
WWHD_ASSERT_OFFSET(cBgD_t, m_t_tbl,   0x0C);
WWHD_ASSERT_OFFSET(cBgD_t, m_b_tbl,   0x14);
WWHD_ASSERT_OFFSET(cBgD_t, m_tree_tbl,0x1C);
WWHD_ASSERT_OFFSET(cBgD_t, m_g_num,   0x20);
WWHD_ASSERT_OFFSET(cBgD_t, m_g_tbl,   0x24);
WWHD_ASSERT_OFFSET(cBgD_t, m_ti_tbl,  0x2C);
WWHD_ASSERT_OFFSET(cBgD_t, flag,      0x30);
WWHD_ASSERT_SIZE  (cBgD_t,            0x34);

/** [V] Set once cBgD_t::Initialize has relocated the tables. Before that the
 *  pointers are file-relative offsets, not addresses. */
#define WWHD_BGD_INITIALISED 0x80000000u

/** [V] Non-zero once the tables hold real pointers. */
static __inline int cBgD_isInitialised(const cBgD_t* d) {
    return d && (d->flag & WWHD_BGD_INITIALISED) != 0;
}

/** [V] One collision group, or NULL. Refuses before relocation, when m_g_tbl
 *  is still an offset rather than a pointer. */
static __inline cBgD_Grp_t* cBgD_getGroup(const cBgD_t* d, int idx) {
    if (!cBgD_isInitialised(d) || idx < 0 || idx >= d->m_g_num || !d->m_g_tbl)
        return (cBgD_Grp_t*)0;
    return &WWHD_AT(cBgD_Grp_t, d->m_g_tbl)[idx];
}

/** [V] One collision vertex, or NULL. */
static __inline cBgD_Vtx_t* cBgD_getVertex(const cBgD_t* d, int idx) {
    if (!cBgD_isInitialised(d) || idx < 0 || idx >= d->m_v_num || !d->m_v_tbl)
        return (cBgD_Vtx_t*)0;
    return &WWHD_AT(cBgD_Vtx_t, d->m_v_tbl)[idx];
}

/**
 * [V] One collision triangle, stride 0x0A: three vertex indices, the index
 * of its property record and its group. cBgW::GetTriPnt (0x0200A220) reads
 * the three indices at +0, +2 and +4 with `* 10`, and the property readers
 * (0x024EEB94, 0x024EEF58) take the halfword at +6 as the index into the
 * 0x10-stride property table. Same as GameCube.
 */
typedef struct cBgD_Tri_t {
    /* 0x0 */ u16 vtx0;
    /* 0x2 */ u16 vtx1;
    /* 0x4 */ u16 vtx2;
    /* 0x6 */ u16 id;   /* [V] property record */
    /* 0x8 */ u16 grp;  /* [P] group */
} cBgD_Tri_t;
WWHD_ASSERT_SIZE(cBgD_Tri_t, 0x0A);

/**
 * [V] One property record, four words, stride 0x10. The two readers above
 * mask word 0 and word 1 respectively. From the GameCube getters, word 1
 * holds the wall code (bits 8..11), the special code (12..15), the raw
 * attribute (16..20) and the ground code (21..25); word 3 holds the
 * pass-through bits, of which 0x04 is "Link passes through".
 */
typedef struct cBgD_Ti_t {
    /* 0x0 */ u32 mPolyInf0;
    /* 0x4 */ u32 mPolyInf1;
    /* 0x8 */ u32 mPolyInf2;
    /* 0xC */ u32 mPolyInf3;
} cBgD_Ti_t;
WWHD_ASSERT_SIZE(cBgD_Ti_t, 0x10);

#define WWHD_BGD_THROUGH_LINK 0x04u /* [P] mPolyInf3 bit: Link passes */

/** [V] One collision triangle, or NULL. */
static __inline cBgD_Tri_t* cBgD_getTri(const cBgD_t* d, int idx) {
    if (!cBgD_isInitialised(d) || idx < 0 || idx >= d->m_t_num || !d->m_t_tbl)
        return (cBgD_Tri_t*)0;
    return &WWHD_AT(cBgD_Tri_t, d->m_t_tbl)[idx];
}

/** [V] One property record, or NULL. */
static __inline cBgD_Ti_t* cBgD_getTi(const cBgD_t* d, int idx) {
    if (!cBgD_isInitialised(d) || idx < 0 || idx >= d->m_ti_num || !d->m_ti_tbl)
        return (cBgD_Ti_t*)0;
    return &WWHD_AT(cBgD_Ti_t, d->m_ti_tbl)[idx];
}

/**
 * [V] cBgW, the registered collision object: the vertex table it collides
 * with (+0x90, already world space; a moving object keeps a transformed
 * copy there) and the data it was built from (+0x94). Both read by
 * cBgW::GetTriPnt (0x0200A220), which asserts pm_bgd non-NULL.
 */
#define WWHD_BGW_OFF_VTX_TBL 0x90
#define WWHD_BGW_OFF_BGD     0x94

static __inline cBgD_t* cBgW_getBgd(const void* bgw) {
    return bgw ? WWHD_AT(cBgD_t, *(const wwhd_gptr_t*)((const u8*)bgw + WWHD_BGW_OFF_BGD))
               : (cBgD_t*)0;
}

static __inline cBgD_Vtx_t* cBgW_getVtxTbl(const void* bgw) {
    return bgw ? WWHD_AT(cBgD_Vtx_t, *(const wwhd_gptr_t*)((const u8*)bgw + WWHD_BGW_OFF_VTX_TBL))
               : (cBgD_Vtx_t*)0;
}


/* ========================================================================
 * The collision registry
 *
 * dBgS opens with an array of 256 of these, which is what every
 * "0 <= bg_index && bg_index < 256" assert in d_bg_s.cpp and c_bg_s.cpp is
 * bounding. Verified from the shared chk walker at 0x02008860, which iterates
 * exactly 0x100 entries advancing 5 words each time and tests m_flags bit 0
 * before touching an entry, and from the release helper at 0x020087EC, which
 * indexes `base + idx * 0x14 + 4` to reach the same flag.
 * ===================================================================== */

/** [V] One registered collision object. Size and the first two fields are read
 *  out of WWHD; m_actor_ptr carries the GameCube layout. */
typedef struct cBgS_ChkElm {
    /* 0x00 */ wwhd_gptr_t m_bgw_base_ptr; /* [V] cBgW* */
    /* 0x04 */ u32         m_flags;        /* [V] bit 0 = in use */
    /* 0x08 */ u32         m_actor_id;     /* [V] owning process id */
    /* 0x0C */ wwhd_gptr_t m_actor_ptr;    /* [V] fopAc_ac_c*. dBgS::Regist
                                            *     keeps the actor in r6 and
                                            *     passes it as the 4th arg */
    /* 0x10 */ wwhd_gptr_t _vtable;        /* [P] */
} cBgS_ChkElm;
WWHD_ASSERT_OFFSET(cBgS_ChkElm, m_flags,    0x04);
WWHD_ASSERT_OFFSET(cBgS_ChkElm, m_actor_id,  0x08);
WWHD_ASSERT_OFFSET(cBgS_ChkElm, m_actor_ptr, 0x0C);
WWHD_ASSERT_SIZE  (cBgS_ChkElm,             0x14);

/** [V] Entries in the registry. Every bounds assert in the collision code
 *  checks against this. */
#define WWHD_BGS_ELM_MAX 256

/** [V] m_flags bit 0: this slot holds a live registration. */
#define WWHD_BGS_ELM_USED 1u

/** [V] Non-zero when the slot is registered. */
static __inline int cBgS_elmUsed(const cBgS_ChkElm* e) {
    return e && (e->m_flags & WWHD_BGS_ELM_USED) != 0;
}


/* ========================================================================
 * Ground query
 * ===================================================================== */

/** [V] Returned when nothing was hit. Exactly -1e9; the game's own two-probe
 *  variant compares against this constant before falling back. */
#define WWHD_BGS_NO_GROUND (-1000000000.0f)

#ifdef WWHD_ENABLE_GAME_CALLS
/**
 * [V] Ground height under a position, or WWHD_BGS_NO_GROUND if nothing is there.
 *
 * Calls the game's own helper, which:
 *   - constructs a dBgS_GndChk on its stack with the game's constructor,
 *   - copies the position into it,
 *   - runs cBgS::GroundCross against dComIfGp_getBgS(),
 *   - and, if the position is over water, returns the sea surface when that is
 *     higher - so this is "the surface you would land on", not just terrain.
 *
 * It takes nothing but the position and finds the collision root itself, which
 * is what makes this safe to expose: no part of the chk layout is guessed.
 *
 * Requires a resolved text delta, like every other game call. Returns
 * WWHD_BGS_NO_GROUND rather than jumping into unmapped memory if it is not.
 */
static __inline f32 dBgS_getGroundY(const cXyz* pos) {
    typedef f32 (*getGroundY_t)(const cXyz*);
    if (!pos || !wwhd_textResolved || !wwhd_regionResolved)
        return WWHD_BGS_NO_GROUND;
    return WWHD_FN(getGroundY_t, wwhd_map->dBgS_getGroundY)(pos);
}

/** [V] Non-zero when dBgS_getGroundY found a surface. */
static __inline int dBgS_hasGround(f32 y) {
    return y > WWHD_BGS_NO_GROUND;
}

/**
 * [V] Drop a position onto the surface below it. Returns non-zero if it moved.
 *
 * Leaves the position untouched when there is nothing underneath, so a caller
 * that ignores the result never teleports into the void.
 */
static __inline int dBgS_snapToGround(cXyz* pos) {
    f32 y;
    if (!pos)
        return 0;
    y = dBgS_getGroundY(pos);
    if (!dBgS_hasGround(y))
        return 0;
    pos->y = y;
    return 1;
}
#endif /* WWHD_ENABLE_GAME_CALLS */

#endif /* LIBWWHD_C_BG_W_H */
