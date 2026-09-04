#ifndef LIBWWHD_D_COM_INF_GAME_H
#define LIBWWHD_D_COM_INF_GAME_H

#include "libwwhd/wwhd_types.h"
#include "libwwhd/wwhd_region.h"
#include "libwwhd/d/d_save.h"
#include "libwwhd/d/d_event.h"
#include "libwwhd/d/d_stage.h"
#include "libwwhd/SSystem/c_bg_w.h"
#include "libwwhd/f_op/f_op_actor.h"

/**
 * libwwhd - game info root (d_com_inf_game.cpp)
 *
 * WWHD carries TWO objects shaped like the GameCube dComIfG_inf_c, both mapped
 * from their constructors. The static one at wwhd_map->gameInfoStatic keeps the
 * GameCube layout; the heap one behind wwhd_map->pGameInfoHeap shifts it +0x20
 * past a two-vtable prefix. They are not one object reached two ways - they
 * have different constructors for their play sub-object - and setNextStage
 * writes to BOTH, so neither is "the" root. docs/SUBSYSTEMS.md (Game info root) has the detail.
 *
 * REGION NOTE: dComIfG_play_c members at or after WWHD_PLAY_TAIL_FROM sit 4
 * bytes lower on JAP. The struct below carries USA/EUR offsets; reach anything
 * past that boundary through the accessors at the bottom of this header, which
 * apply wwhd_playOfs(). Members before it, including the stage record at
 * play+0x3EA0, are identical in all three builds.
 */

/**
 * dComIfG_play_c. Offsets are relative to the play base.
 *
 * Size 0x4A90 is [V] for USA/EUR: the constructor builds play at +0x12A0 and
 * the next sub-object at +0x5D30. JAP is 0x4A8C.
 */
typedef struct dComIfG_play_c {
    /* 0x0000 */ u8 mBgS[0x3E94];                /* [V] OFFSET. dBgS, the
                                                  *     collision root: 104 call
                                                  *     sites in 87 functions
                                                  *     pass play+0x0000 as the
                                                  *     `this` for the d_bg_s
                                                  *     routines. Its SIZE is
                                                  *     unknown - the array runs
                                                  *     to the next known member
                                                  *     and also covers dCcS and
                                                  *     dADM. Interior [?] */
    /* 0x3E94 */ dStage_startStage_c mCurStage;  /* [V] the stage you are in.
                                                  *     578 reads across 208
                                                  *     functions */
    /* 0x3EA0 */ dStage_nextStage_c  mNextStage; /* [V] the pending warp */
    /* 0x3EAE */ u8 _pad_3EAE[0x3EB0 - 0x3EAE];  /* [?] */
    /* 0x3EB0 */ u8 mStageData[0x3F30 - 0x3EB0]; /* [V] offset; 206 reads across
                                                  *     119 functions. Interior
                                                  *     [?] */
    /* 0x3F30 */ dEvt_control_c   mEvtCtrl;      /* [V] live event state */
    /* 0x4024 */ dEvent_manager_c mEvtManager;   /* [V] incl. camera ownership */
    /* 0x4564 */ u8 _unk_4564[0x4810 - 0x4564];  /* [?] attention, archives */
    /* ---- WWHD_PLAY_TAIL_FROM: everything below shifts -4 on JAP ---- */
    /* 0x4810 */ wwhd_gptr_t mpParticleMng;      /* [V] 1423 reads, passed as
                                                  *     arg 0 to the emitter
                                                  *     spawn at 0x025A847C.
                                                  *     Name confirmed: it is
                                                  *     the same object as the
                                                  *     JPAEmitterManager the
                                                  *     demo init asserts on
                                                  *     (see d_demo.h). */
    /* 0x4814 */ u8 _unk_4814[0x4858 - 0x4814];  /* [?] */
    /* 0x4858 */ wwhd_gptr_t _unk_ptr_4858;      /* [V] pointer; meaning [?] */
    /* 0x485C */ u8 _unk_485C[0x488C - 0x485C];  /* [?] */
    /* 0x488C */ wwhd_gptr_t mpPlayer;           /* [V] Link. 1021 reads; 302
                                                  *     functions deref it with
                                                  *     verified actor fields */
    /* 0x4890 */ u8  _unk_4890[4];               /* [V] NOT a pointer: byte
                                                  *     access only, 70 fns   */
    /* 0x4894 */ wwhd_gptr_t mpPlayerSlot1;      /* [V] slot 1. Holds Link too
                                                  *     in normal play - its
                                                  *     readers dereference
                                                  *     mCurProc at +0x65F0 and
                                                  *     the mDemo block, which
                                                  *     are daPy_lk_c's.      */
    /* 0x4898 */ u32 _unk_4898;                  /* [V] word access, 26 fns */
    /* 0x489C */ wwhd_gptr_t mpShip;             /* [V] daShip_c* - the King of
                                                  *     Red Lions. See the note
                                                  *     below.                */
    /* 0x48A0 */ u32 _unk_48A0;                  /* [?] never touched */
    /* 0x48A4 */ f32 mItemLifeCount;             /* [V] pending life delta, in
                                                  *     quarter-hearts. Float
                                                  *     access, 18 fns.       */
    /* 0x48A8 */ s32 mItemRupeeCount;            /* [V] pending rupee delta.
                                                  *     Word access, 39 fns.  */
    /* 0x48AC */ u8  _unk_48AC[0x48BE - 0x48AC]; /* [?] */
    /* 0x48BE */ s16 mItemMaxLifeCount;          /* [V] pending max-life delta.
                                                  *     Halfword access, 3 fns:
                                                  *     heart piece, heart
                                                  *     container, and one more */
    /* 0x48C0 */ s16 mItemMagicCount;            /* [V] pending magic delta.
                                                  *     Magic jars 0x09/0x0A
                                                  *     and the Deku Leaf add
                                                  *     here; the magic meter
                                                  *     folds it in          */
    /* 0x48C2 */ u8  _unk_48C2[2];               /* [?] */
    /* 0x48C4 */ s16 mItemMaxMagicCount;         /* [V] pending max-magic
                                                  *     delta: Deku Leaf +16,
                                                  *     upgrade 0xB2 +32     */
    /* 0x48C6 */ u8  _unk_48C6[2];               /* [?] */
    /* 0x48C8 */ s16 mItemArrowCount;            /* [P] pending arrows: the
                                                  *     three arrow pickups  */
    /* 0x48CA */ u8  _unk_48CA[2];               /* [?] */
    /* 0x48CC */ s16 mItemBombCount;             /* [P] pending bombs        */
    /* 0x48CE */ u8  _unk_48CE[2];               /* [?] */
    /* 0x48D0 */ s16 mItemBeastCount[8];         /* [P] pending spoils, by
                                                  *     bag index; each spoil
                                                  *     stub adds one        */
    /* 0x48E0 */ u8  _unk_48E0[0x490C - 0x48E0]; /* [?] */
    /* 0x490C */ u16 mMagicDisp;                 /* [P] the magic meter's
                                                  *     on-screen count,
                                                  *     mirrored by the meter's
                                                  *     draw; one writer
                                                  *     (0x02593B10)          */
    /* 0x490E */ u16 mRupeeDisp;                 /* [V] the rupee count on
                                                  *     screen, mirrored from
                                                  *     the meter's own digits
                                                  *     (d_meter.h). Written by
                                                  *     the meter's create and
                                                  *     its rupee routine; read
                                                  *     by the shop UI and by
                                                  *     the "count settled"
                                                  *     test at 0x02263234   */
    /* 0x4910 */ u8  _unk_4910[0x4A38 - 0x4910]; /* [?] */
    /* 0x4A38 */ u32 mPlayerStatus[1][4];        /* [V] stride 0x10; the camera
                                                  *     tests [i][0] and [i][1]
                                                  *     every frame */
    /* 0x4A48 */ u8  _unk_4A48[0x4A8C - 0x4A48]; /* [?] */
    /* 0x4A8C */ u8  _unk_flag_4A8C;             /* [V] gates a camera branch */
    /* 0x4A8D */ u8  _unk_4A8D[0x4A90 - 0x4A8D]; /* [?] */
} dComIfG_play_c;
WWHD_ASSERT_OFFSET(dComIfG_play_c, mCurStage,     0x3E94);
WWHD_ASSERT_OFFSET(dComIfG_play_c, mNextStage,    0x3EA0);
WWHD_ASSERT_OFFSET(dComIfG_play_c, mStageData,    0x3EB0);
WWHD_ASSERT_OFFSET(dComIfG_play_c, mEvtCtrl,      0x3F30);
WWHD_ASSERT_OFFSET(dComIfG_play_c, mEvtManager,   0x4024);
WWHD_ASSERT_OFFSET(dComIfG_play_c, mpParticleMng, WWHD_PLAY_TAIL_FROM);
WWHD_ASSERT_OFFSET(dComIfG_play_c, mpPlayer,          0x488C);
WWHD_ASSERT_OFFSET(dComIfG_play_c, mpPlayerSlot1,     0x4894);
WWHD_ASSERT_OFFSET(dComIfG_play_c, mpShip,           0x489C);
WWHD_ASSERT_OFFSET(dComIfG_play_c, mItemLifeCount,    0x48A4);
WWHD_ASSERT_OFFSET(dComIfG_play_c, mItemRupeeCount,   0x48A8);
WWHD_ASSERT_OFFSET(dComIfG_play_c, mItemMaxLifeCount, 0x48BE);
WWHD_ASSERT_OFFSET(dComIfG_play_c, mItemMagicCount,   0x48C0);
WWHD_ASSERT_OFFSET(dComIfG_play_c, mItemMaxMagicCount,0x48C4);
WWHD_ASSERT_OFFSET(dComIfG_play_c, mItemBeastCount,   0x48D0);
WWHD_ASSERT_OFFSET(dComIfG_play_c, mRupeeDisp,        0x490E);
WWHD_ASSERT_OFFSET(dComIfG_play_c, mPlayerStatus, 0x4A38);
WWHD_ASSERT_SIZE  (dComIfG_play_c,                0x4A90);

/** [V] Offset of the next-stage record inside play. Below the JAP boundary, so
 *  identical in all three builds. */
#define WWHD_PLAY_OFF_NEXTSTAGE 0x3EA0

/**
 * Static instance, GameCube-compatible layout.
 *
 * Constructor: memset(this, 0x12A0) -> dSv_info_c; array_ct(+0x380, 0x10, 0x24)
 * -> mMemory; danBit_ct(+0x778); array_ct(+0x7A8, 0x20, 0x4C) -> mZone;
 * play_ct(+0x12A0); sub_ct(+0x5D30); *(u8*)(+0x62F1) = 0xFF.
 * JAP moves the last three to 0x5D2C / 0x62ED and allocates 0x62F0.
 */
typedef struct dComIfG_inf_c {
    /* 0x0000 */ dSv_info_c     save;                        /* [V] */
    /* 0x12A0 */ dComIfG_play_c play;                        /* [V] */
    /* 0x5D30 */ u8             _unk_5D30[0x62F8 - 0x5D30];  /* [?] */
} dComIfG_inf_c;
WWHD_ASSERT_OFFSET(dComIfG_inf_c, save, 0x0000);
WWHD_ASSERT_OFFSET(dComIfG_inf_c, play, 0x12A0);
WWHD_ASSERT_SIZE  (dComIfG_inf_c,       0x62F8);

/** [V] Byte offset of the dSv_info_c inside the heap object. */
#define WWHD_GAMEINFO_HEAP_PREFIX 0x20
/** [V] Real allocation size of the heap object. */
#define WWHD_GAMEINFO_HEAP_SIZE   0xF02D80u

/**
 * Heap instance: the same members shifted +0x20 by a two-vtable prefix.
 *
 * Declared without the 15.7 MB tail - it is unmapped and a struct that large is
 * unpleasant for no benefit - so the size is documented, not asserted.
 */
typedef struct dComIfG_inf_heap_c {
    /* 0x0000 */ u8             _prefix[0x0C];          /* [?] */
    /* 0x000C */ wwhd_gptr_t    _vtable0;               /* [V] */
    /* 0x0010 */ u8             _prefix2[0x1C - 0x10];  /* [?] */
    /* 0x001C */ wwhd_gptr_t    _vtable1;               /* [V] */
    /* 0x0020 */ dSv_info_c     save;                   /* [V] */
    /* 0x12C0 */ dComIfG_play_c play;                   /* [V] */
} dComIfG_inf_heap_c;
WWHD_ASSERT_OFFSET(dComIfG_inf_heap_c, _vtable0, 0x000C);
WWHD_ASSERT_OFFSET(dComIfG_inf_heap_c, _vtable1, 0x001C);
WWHD_ASSERT_OFFSET(dComIfG_inf_heap_c, save,     0x0020);
WWHD_ASSERT_OFFSET(dComIfG_inf_heap_c, play,     0x12C0);

/* --- Accessors ---------------------------------------------------------- */

/** [V] The static gameInfo, or NULL before a region is selected. */
static __inline dComIfG_inf_c* dComIfGp_getGameInfo(void) {
    if (!wwhd_regionResolved)
        return (dComIfG_inf_c*)0;
    return WWHD_AT_DATA(dComIfG_inf_c, wwhd_map->gameInfoStatic);
}

/** [V] The heap gameInfo, or NULL before it is constructed. */
static __inline dComIfG_inf_heap_c* dComIfGp_getGameInfoHeap(void) {
    u32 p;
    if (!wwhd_regionResolved)
        return (dComIfG_inf_heap_c*)0;
    p = *WWHD_AT_DATA(u32, wwhd_map->pGameInfoHeap);
    return p ? WWHD_AT(dComIfG_inf_heap_c, p) : (dComIfG_inf_heap_c*)0;
}

/**
 * [V] Live save state - THE HEAP ROOT, not the static one.
 *
 * The two roots are not interchangeable, and this is the single easiest way to
 * write a tool that runs, reports success and changes nothing:
 *
 *     save -> the HEAP object   (*pGameInfoHeap + 0x20)
 *     play -> the STATIC object (gameInfoStatic + 0x12A0)
 *
 * The save side is settled by 0x025C1E44, which calls dSv_info_c::getSave with
 * `this = *pGameInfoHeap + 0x20` and, on the next line, the dan-bit routine
 * with `*pGameInfoHeap + 0x7BC` - which is that same base + 0x79C, exactly
 * dSv_info_c::mDan. Every other save toucher agrees: the rupee meter clamps
 * mRupee at heap+0x24, the magic meter writes mMaxMagic and mMagic at heap+0x33
 * and heap+0x34, and the bow and bomb-bag stubs write their counts and
 * capacities at heap+0x89/0x8F and heap+0x8A/0x90.
 *
 * The play side is settled by the same rupee meter, which reads its pending
 * rupee delta from the STATIC root at +0x5B48 - static.play + 0x48A8 - in the
 * middle of the very function that writes the save through the heap pointer.
 * Two roots, one function, no ambiguity.
 *
 * An earlier version of this accessor used the static root. It compiled, it
 * returned a plausible non-NULL pointer, and everything written through it was
 * discarded, because nothing in the game reads the static object's save.
 */
static __inline dSv_info_c* dComIfGs_getSaveInfo(void) {
    dComIfG_inf_heap_c* g = dComIfGp_getGameInfoHeap();
    return g ? &g->save : (dSv_info_c*)0;
}

/** [V] The player/inventory save block: items, rupees, bottles, bag. */
static __inline dSv_player_c* dComIfGs_getPlayerSave(void) {
    dSv_info_c* s = dComIfGs_getSaveInfo();
    return s ? &s->mSavedata.mPlayer : (dSv_player_c*)0;
}

/** [V] The play struct. Members past WWHD_PLAY_TAIL_FROM must be reached
 *  through the accessors below, not as struct members. */
static __inline dComIfG_play_c* dComIfGp_getPlay(void) {
    dComIfG_inf_c* g = dComIfGp_getGameInfo();
    return g ? &g->play : (dComIfG_play_c*)0;
}

/** Byte pointer into play at a region-corrected offset, or NULL. */
static __inline void* dComIfGp_playAt(u32 usaOfs) {
    dComIfG_play_c* p = dComIfGp_getPlay();
    if (!p)
        return (void*)0;
    return (void*)((u8*)p + wwhd_playOfs(usaOfs));
}

/** [V] Live event controller. Below the JAP boundary, so a direct member. */
static __inline dEvt_control_c* dComIfGp_getEvent(void) {
    dComIfG_play_c* p = dComIfGp_getPlay();
    return p ? &p->mEvtCtrl : (dEvt_control_c*)0;
}

/** [V] Live event manager, including event ownership of the camera. */
static __inline dEvent_manager_c* dComIfGp_getEventManager(void) {
    dComIfG_play_c* p = dComIfGp_getPlay();
    return p ? &p->mEvtManager : (dEvent_manager_c*)0;
}

static __inline u8 dEvt_getEventMode(void) {
    dEvt_control_c* e = dComIfGp_getEvent();
    return e ? e->mMode : (u8)dEvtMode_NONE_e;
}

static __inline int dEvt_isEventRunning(void) {
    return dEvt_getEventMode() != dEvtMode_NONE_e;
}

static __inline void dEvt_setEventMode(u8 mode) {
    dEvt_control_c* e = dComIfGp_getEvent();
    if (e)
        e->mMode = mode;
}

static __inline void dEvt_setEventRunning(int on) {
    dEvt_setEventMode(on ? (u8)dEvtMode_TALK_e : (u8)dEvtMode_NONE_e);
}

static __inline u32 dEvent_getCameraPlay(void) {
    dEvent_manager_c* m = dComIfGp_getEventManager();
    return m ? m->mCameraPlay : 0u;
}

static __inline int dEvent_isCameraPlay(void) {
    return dEvent_getCameraPlay() != 0u;
}

static __inline void dEvent_setCameraPlay(int on) {
    dEvent_manager_c* m = dComIfGp_getEventManager();
    if (m)
        m->mCameraPlay = on ? 1u : 0u;
}


/* --- Warping -------------------------------------------------------------
 * A warp is a request, not an action: fill in the next-stage record and set
 * mEnable, and the game performs the load on its own schedule.
 *
 * This is done by writing the record rather than by calling the game's
 * setNextStage, because the two are equivalent - the game routine is a guard
 * plus five field writes, all read out of WWHD - and a plain write needs no
 * resolved text delta and no call into game code from a hook thread.
 * wwhd_map->setNextStage is kept for reference and for parity checking.
 * ---------------------------------------------------------------------- */

/** [V] The pending stage request. Below the JAP boundary, so a direct member. */
static __inline dStage_nextStage_c* dComIfGp_getNextStage(void) {
    dComIfG_play_c* p = dComIfGp_getPlay();
    return p ? &p->mNextStage : (dStage_nextStage_c*)0;
}

/** [V] The stage currently loaded. Below the JAP boundary. */
static __inline dStage_startStage_c* dComIfGp_getCurStage(void) {
    dComIfG_play_c* p = dComIfGp_getPlay();
    return p ? &p->mCurStage : (dStage_startStage_c*)0;
}

/**
 * [V] Current stage name, or "" when unavailable. Always NUL-terminated.
 *
 * The four getters here read the record the game filled in when the stage was
 * LOADED. mRoomNo and mLayer get 2 and 0 stores respectively across the whole
 * binary, so they are set once on entry and not maintained as Link moves - for
 * the room he is actually standing in, use daPy_getRoomNo().
 */
static __inline const char* dComIfGp_getCurStageName(void) {
    dStage_startStage_c* c = dComIfGp_getCurStage();
    return c ? c->mName : "";
}

/** [V] Room the current stage was entered at, or -1 when unavailable. */
static __inline s8 dComIfGp_getCurStageRoomNo(void) {
    dStage_startStage_c* c = dComIfGp_getCurStage();
    return c ? c->mRoomNo : (s8)-1;
}

/** [V] Layer the current stage was entered on, or WWHD_STAGE_LAYER_KEEP. */
static __inline s8 dComIfGp_getCurStageLayer(void) {
    dStage_startStage_c* c = dComIfGp_getCurStage();
    return c ? c->mLayer : WWHD_STAGE_LAYER_KEEP;
}

/** [V] Spawn point the current stage was entered at, or -1. */
static __inline s16 dComIfGp_getCurStageSpawnPoint(void) {
    dStage_startStage_c* c = dComIfGp_getCurStage();
    return c ? c->mPoint : (s16)-1;
}

/* The same four for the PENDING warp, which read as the request rather than
 * the current state. All zero/empty when nothing is queued. */

/** [V] Name of the queued stage, or "" when none is pending. */
static __inline const char* dComIfGp_getNextStageName(void) {
    dStage_nextStage_c* n = dComIfGp_getNextStage();
    return (n && n->mEnable) ? n->mName : "";
}

/** [V] Room of the queued stage, or -1 when none is pending. */
static __inline s8 dComIfGp_getNextStageRoomNo(void) {
    dStage_nextStage_c* n = dComIfGp_getNextStage();
    return (n && n->mEnable) ? n->mRoomNo : (s8)-1;
}

/** [V] Layer of the queued stage, or WWHD_STAGE_LAYER_KEEP. */
static __inline s8 dComIfGp_getNextStageLayer(void) {
    dStage_nextStage_c* n = dComIfGp_getNextStage();
    return (n && n->mEnable) ? n->mLayer : WWHD_STAGE_LAYER_KEEP;
}

/** [V] Spawn point of the queued stage, or -1 when none is pending. */
static __inline s16 dComIfGp_getNextStageSpawnPoint(void) {
    dStage_nextStage_c* n = dComIfGp_getNextStage();
    return (n && n->mEnable) ? n->mPoint : (s16)-1;
}

/** [V] The loaded stage's chunk directory, or NULL. Below the JAP boundary. */
static __inline dStage_dt_c* dComIfGp_getStageDt(void) {
    dComIfG_play_c* p = dComIfGp_getPlay();
    return p ? (dStage_dt_c*)p->mStageData : (dStage_dt_c*)0;
}

/** [V] The STAG record of the loaded stage, or NULL until one is loaded. */
static __inline dStage_stagInfo_c* dComIfGp_getStageStagInfo(void) {
    return dStage_dt_getStagInfo(dComIfGp_getStageDt());
}

/**
 * [V] The dSv_memory slot the loaded stage saves into, or -1.
 *
 * The live per-stage bits (dSv_info_c::mMemory) are copied into this slot only
 * when the stage is left, so a snapshot of the block taken mid-stage has to do
 * that copy itself to be the save the game would have written.
 */
static __inline int dComIfGp_getStageSaveTblNo(void) {
    int n = dStage_stagInfo_getSaveTbl(dComIfGp_getStageStagInfo());
    return (n >= 0 && n < dSv_STAGE_MAX) ? n : -1;
}

/** [V] The collision root, or NULL. There is no ground-query API here - see
 *  SSystem/c_bg_w.h for what is and is not mapped. */
static __inline void* dComIfGp_getBgS(void) {
    dComIfG_play_c* p = dComIfGp_getPlay();
    return p ? (void*)p->mBgS : (void*)0;
}

/** [V] The collision registry, which dBgS opens with: 256 entries at
 *  play + 0x0000, spanning 0x1400 bytes. */
static __inline cBgS_ChkElm* dComIfGp_getBgSRegistry(void) {
    return (cBgS_ChkElm*)dComIfGp_getBgS();
}

/** [V] One registry slot, or NULL if out of range. */
static __inline cBgS_ChkElm* dComIfGp_getBgSElm(int idx) {
    cBgS_ChkElm* r = dComIfGp_getBgSRegistry();
    if (!r || idx < 0 || idx >= WWHD_BGS_ELM_MAX)
        return (cBgS_ChkElm*)0;
    return &r[idx];
}

/** [V] Non-zero while a stage change is already queued. */
static __inline int dComIfGp_isNextStagePending(void) {
    dStage_nextStage_c* n = dComIfGp_getNextStage();
    return n && n->mEnable != 0;
}

/**
 * [V] Request a stage change. Returns non-zero if the request was accepted.
 *
 * Refuses when one is already queued, exactly as the game's own setter does:
 * a second warp posted before the first is consumed would otherwise be merged
 * into a half-written record.
 *
 * mEnable is written LAST, after a barrier. The game polls this record from
 * another core, and setting the flag before the name is in place would let it
 * act on a stale or half-copied request. Everything else is ordered before it
 * for that reason alone.
 *
 * `layer` of WWHD_STAGE_LAYER_KEEP keeps the current layer.
 */
static __inline int dComIfGp_setNextStage(const char* name, s16 point,
                                          s8 roomNo, s8 layer, s8 wipe) {
    dStage_nextStage_c* n = dComIfGp_getNextStage();
    int i;
    if (!n || !name || n->mEnable != 0)
        return 0;

    for (i = 0; i < WWHD_STAGE_NAME_MAX - 1 && name[i]; ++i)
        n->mName[i] = name[i];
    for (; i < WWHD_STAGE_NAME_MAX; ++i)
        n->mName[i] = 0;

    n->mPoint  = point;
    n->mRoomNo = roomNo;
    n->mLayer  = layer;
    n->mWipe   = wipe;

#if defined(__GNUC__) && defined(__PPC__)
    __asm__ __volatile__("eieio" ::: "memory");
#elif defined(__GNUC__)
    __asm__ __volatile__("" ::: "memory");
#endif
    n->mEnable = 1;
    return 1;
}

/** [V] Withdraw a queued stage change, if the game has not consumed it yet. */
static __inline void dComIfGp_cancelNextStage(void) {
    dStage_nextStage_c* n = dComIfGp_getNextStage();
    if (n)
        n->mEnable = 0;
}

/**
 * [V] Re-run the last stage request as it was made.
 *
 * phase_1 (0x025B1590) copies the six halfwords of the record over the
 * current stage and then clears only mEnable, so the name, point, room, layer
 * and wipe of the request that brought this stage in are still sitting there.
 * Raising the flag again is a reload at the same entrance with the same
 * transition, and needs no other write.
 *
 * Refused while a request is pending, and when the record no longer names the
 * stage you are in: a withdrawn request (dComIfGp_cancelNextStage) leaves its
 * fields behind, and raising the flag over those would go somewhere else.
 */
static __inline int dComIfGp_reloadStage(void) {
    dStage_nextStage_c* n = dComIfGp_getNextStage();
    dStage_startStage_c* c = dComIfGp_getCurStage();
    int i;
    if (!n || !c || n->mEnable != 0 || !c->mName[0])
        return 0;
    for (i = 0; i < WWHD_STAGE_NAME_MAX; i++) {
        if (n->mName[i] != c->mName[i])
            return 0;
        if (c->mName[i] == '\0')
            break;
    }
#if defined(__GNUC__) && defined(__PPC__)
    __asm__ __volatile__("eieio" ::: "memory");
#elif defined(__GNUC__)
    __asm__ __volatile__("" ::: "memory");
#endif
    n->mEnable = 1;
    return 1;
}

/* --- The soft reset --------------------------------------------------------
 * [V] mDoRst's data block: the pointer at wwhd_map->resetData is set at boot
 * (0x025F1660) to four words in .bss whose first is the reset flag. Every
 * scene's draw asks dComIfG_resetToOpening (0x025202F8) about it and, when it
 * is up, changes to the opening scene and stops the audio; the logo scene
 * (0x025AC4D4) clears it. Nothing in HD raises it - there is no reset button
 * - so the GameCube reset path is here, complete and dormant.
 * ---------------------------------------------------------------------- */

/** [V] The reset flag word, or NULL before boot or a region. */
static __inline int* dComIfG_getResetFlag(void) {
    u32 p;
    if (!wwhd_regionResolved)
        return (int*)0;
    p = *WWHD_AT_DATA(u32, wwhd_map->resetData);
    return p ? WWHD_AT(int, p) : (int*)0;
}

/** [V] Raise it. The game notices on its next draw. */
static __inline int dComIfG_requestReset(void) {
    int* flag = dComIfG_getResetFlag();
    if (!flag)
        return 0;
    *flag = 1;
    return 1;
}

/** [V] Spawn point recorded in the save's restart block, or -1. This is the
 *  one setNextStage writes when its setPoint argument is true, so it survives
 *  the load where the play-side record is rebuilt. */
static __inline s16 dComIfGs_getStartPoint(void) {
    dSv_info_c* s = dComIfGs_getSaveInfo();
    return s ? s->mRestart.mStartCode : (s16)-1;
}

/** [V] The restart / last-scene record, or NULL. Written alongside a warp. */
static __inline dSv_restart_c* dComIfGs_getRestart(void) {
    dSv_info_c* s = dComIfGs_getSaveInfo();
    return s ? &s->mRestart : (dSv_restart_c*)0;
}

/** [V] The turn-restart record, or NULL: what a -3 spawn resumes from. */
static __inline dSv_turnRestart_c* dComIfGs_getTurnRestart(void) {
    dSv_info_c* s = dComIfGs_getSaveInfo();
    return s ? &s->mTurnRestart : (dSv_turnRestart_c*)0;
}

/** [V] The first player status word, or 0. Past the JAP boundary. */
static __inline u32 dComIfGp_getPlayerStatus0(void) {
    u32* p = (u32*)dComIfGp_playAt(0x4A38);
    return p ? *p : 0u;
}

/** [P] SHIP_RIDE in the first status word. One WWHD sighting: the Song of
 *  Passing packer (0x0243B188) tests it to choose the "start aboard" mode,
 *  and it is the bit GameCube names daPyStts0_SHIP_RIDE_e. */
#define WWHD_PLAYER_STTS0_SHIP_RIDE 0x10000u

static __inline int dComIfGp_isPlayerShipRide(void) {
    return (dComIfGp_getPlayerStatus0() & WWHD_PLAYER_STTS0_SHIP_RIDE) != 0u;
}

/** [V] Link's actor, or NULL if he is not spawned. Past the JAP boundary. */
static __inline fopAc_ac_c* dComIfGp_getPlayer(void) {
    wwhd_gptr_t* slot = (wwhd_gptr_t*)dComIfGp_playAt(0x488C);
    if (!slot || !*slot)
        return (fopAc_ac_c*)0;
    return WWHD_AT(fopAc_ac_c, *slot);
}

/* ========================================================================
 * The actor-pointer slots, and where the boat is kept
 *
 * play+0x488C opens an array of three slots the game indexes with a stride of
 * EIGHT, not four - the camera setup indexes it as (base + n*8 + 0x5B2C) off
 * the static root. So the slots are +0x488C, +0x4894 and +0x489C, with a second
 * word in each entry that is not a pointer: +0x4890 is byte-accessed by 70
 * functions and +0x4898 word-accessed by 26. The array ends exactly where
 * mItemLifeCount begins, which is a nice check on the stride.
 *
 * Slot 2 is the King of Red Lions. That is not a transcription from GameCube -
 * it is what the 119 functions reading it dereference: +0x644 (daShip_c::mFlags)
 * and +0x3B4 (mpModelMain), both ship-specific, and none of the daPy_lk_c
 * members that slots 0 and 1 are full of.
 *
 * This corrects an earlier reading of this area, which had the array at stride
 * 4 and concluded slot +0x4894 was "NOT the ship". +0x4894 is indeed not the
 * ship - it is Link again - but the ship was one slot further on.
 *
 * HOW THE GAME ASKS "IS THE BOAT THERE?": it reads the slot and tests it
 * against zero. There is no separate spawned flag.
 * ===================================================================== */

/**
 * [V] The King of Red Lions, or NULL when it is not spawned.
 *
 * A plain read of play+0x489C - no process search, no text delta, no camera.
 * Returned as the actor base; d_a_ship.h wraps it as daShip_c*.
 */
static __inline fopAc_ac_c* dComIfGp_getShipActor(void) {
    wwhd_gptr_t* slot = (wwhd_gptr_t*)dComIfGp_playAt(0x489C);
    if (!slot || !*slot)
        return (fopAc_ac_c*)0;
    return WWHD_AT(fopAc_ac_c, *slot);
}

/** [V] Non-zero when the boat is spawned. The test the game itself makes. */
static __inline int dComIfGp_isShipAlive(void) {
    return dComIfGp_getShipActor() != (fopAc_ac_c*)0;
}

/** [V] The particle manager pointer, or 0. Past the JAP boundary. */
static __inline wwhd_gptr_t dComIfGp_getParticleMng(void) {
    wwhd_gptr_t* slot = (wwhd_gptr_t*)dComIfGp_playAt(WWHD_PLAY_TAIL_FROM);
    return slot ? *slot : 0u;
}

/* ========================================================================
 * Pending item deltas - the layer between a pickup and the wallet
 *
 * Rupees, life and max life each exist in THREE places, and only the middle one
 * is easy to miss:
 *
 *   1. dSv_player_status_a_c  mRupee / mLife / mMaxLife.  The save value. The
 *                             truth, and what dSv_setRupee() writes.
 *   2. these three fields.    A pending DELTA. Picking up a rupee does not
 *                             touch the wallet - it adds to mItemRupeeCount,
 *                             and the meter folds that into the save value and
 *                             clears it.
 *   3. the meter's own count. What the HUD draws. Derived, but NOT
 *                             self-correcting: the meter re-reads the save
 *                             value only when a delta arrives, so a save value
 *                             written with no delta is not shown until the
 *                             next pickup. d_meter.h exposes it, and
 *                             dMeter_setRupeeDisplay() is the companion to a
 *                             direct dSv_setRupee(). A copy of it sits in play
 *                             at +0x490E (dComIfGp_getRupeeDisp).
 *
 * Read out of the item give-functions, which the item table indexes directly:
 * green / blue / red rupee add 1, 5 and 0x14 to +0x48A8, a heart adds a float
 * to +0x48A4, and a heart container adds 4 - one container in quarter-hearts -
 * to +0x48BE. The container function also adds (mMaxLife - k) to the life
 * delta, which is the refill, and re-confirms mMaxLife at save+0x00 on the way
 * past.
 *
 * THE TRAP: a non-zero pending delta is applied ON TOP of whatever you write to
 * the save value. Setting rupees while a pickup is still in flight gives you
 * your number plus the pickup. dComIfGp_clearItemDeltas() zeroes them.
 *
 * Magic has the same shape at +0x48C0 / +0x48C4, read out of the magic jar,
 * Deku Leaf and meter-upgrade give stubs (d_item.h) and folded in by the magic
 * meter at 0x0259B698.
 * ===================================================================== */

/** [V] Pending life delta in quarter-hearts, or NULL. Past the JAP boundary. */
static __inline f32* dComIfGp_getItemLifeCount(void) {
    return (f32*)dComIfGp_playAt(0x48A4);
}

/** [V] Pending rupee delta, or NULL. Past the JAP boundary. */
static __inline s32* dComIfGp_getItemRupeeCount(void) {
    return (s32*)dComIfGp_playAt(0x48A8);
}

#define WWHD_SWIM_STAMINA_MAX 900

static __inline s32* dComIfGp_getSwimStamina(void) {
    return (s32*)dComIfGp_playAt(0x48AC);
}

/** [V] Pending max-life delta in quarter-hearts, or NULL. Past the JAP
 *  boundary. */
static __inline s16* dComIfGp_getItemMaxLifeCount(void) {
    return (s16*)dComIfGp_playAt(0x48BE);
}

/** [V] The rupee count on screen, or NULL. Past the JAP boundary. A copy of the
 *  meter's own digits, so writing it alone does not move the HUD: see
 *  dMeter_setRupeeDisplay() in d_meter.h. */
static __inline u16* dComIfGp_getRupeeDisp(void) {
    return (u16*)dComIfGp_playAt(0x490E);
}

/** [V] Pending magic delta, or NULL. Past the JAP boundary. */
static __inline s16* dComIfGp_getItemMagicCount(void) {
    return (s16*)dComIfGp_playAt(0x48C0);
}

/** [V] Pending max-magic delta, or NULL. Past the JAP boundary. */
static __inline s16* dComIfGp_getItemMaxMagicCount(void) {
    return (s16*)dComIfGp_playAt(0x48C4);
}

/**
 * [V] Drop every pending life, rupee and magic delta. Returns non-zero if
 * anything was pending.
 *
 * Use before writing a save value directly, so an in-flight pickup does not
 * land on top of what you set.
 */
static __inline int dComIfGp_clearItemDeltas(void) {
    f32* life = dComIfGp_getItemLifeCount();
    s32* rupee = dComIfGp_getItemRupeeCount();
    s16* maxLife = dComIfGp_getItemMaxLifeCount();
    s16* magic = dComIfGp_getItemMagicCount();
    s16* maxMagic = dComIfGp_getItemMaxMagicCount();
    int pending = 0;
    if (life && *life != 0.0f)      { *life = 0.0f;  pending = 1; }
    if (rupee && *rupee != 0)       { *rupee = 0;    pending = 1; }
    if (maxLife && *maxLife != 0)   { *maxLife = 0;  pending = 1; }
    if (magic && *magic != 0)       { *magic = 0;    pending = 1; }
    if (maxMagic && *maxMagic != 0) { *maxMagic = 0; pending = 1; }
    return pending;
}

#endif /* LIBWWHD_D_COM_INF_GAME_H */
