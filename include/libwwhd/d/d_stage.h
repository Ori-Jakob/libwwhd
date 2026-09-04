#ifndef LIBWWHD_D_STAGE_H
#define LIBWWHD_D_STAGE_H

#include "libwwhd/wwhd_types.h"
#include "libwwhd/wwhd_region.h"

/**
 * libwwhd - stage and room state (d_stage.cpp / d_stage.h)
 *
 * Two things live here: the per-room status table, and the next-stage request
 * that drives every warp. docs/SUBSYSTEMS.md (Stage) has the derivation.
 */

/** [V] Asserted roomNo < 64 by the room accessors. */
#define WWHD_ROOM_MAX 64

/* ========================================================================
 * Per-room status
 * ===================================================================== */

/**
 * dStage_roomStatus_c - one record per room.
 *
 * Every named offset is +0x118 above its GameCube position, and the record grew
 * from 0x114 to 0x22C by the same 0x118. That gap is mTevStr: dKy_tevstr_c is
 * 0xB0 on GameCube and 0x1C8 here. The same 0x118 accounts for the actor base
 * shift, which is two unrelated structures agreeing.
 */
typedef struct dStage_roomStatus_c {
    /* 0x000 */ u8 mRoomDt[0x054];        /* [P] dStage_roomDt_c, GC size    */
    /* 0x054 */ u8 mTevStr[0x1C8];        /* [V] dKy_tevstr_c. Size derived
                                           *     from mFlags landing at 0x21C */
    /* 0x21C */ u8 mFlags;                /* [V] bits 0x08/0x10 tested by the
                                           *     room-visible check           */
    /* 0x21D */ u8 mDraw;                 /* [V] set by onStatusDraw          */
    /* 0x21E */ s8 mZoneCount;            /* [P] */
    /* 0x21F */ s8 mZoneNo;               /* [V] routed to dSv_zone_c lookups */
    /* 0x220 */ s8 mMemBlockID;           /* [P] */
    /* 0x221 */ u8 _pad_221[3];           /* [?] */
    /* 0x224 */ u32 mProcID;              /* [V] room process id              */
    /* 0x228 */ wwhd_gptr_t mpBgW;        /* [P] dBgW* */
} dStage_roomStatus_c;
WWHD_ASSERT_OFFSET(dStage_roomStatus_c, mTevStr,  0x054);
WWHD_ASSERT_OFFSET(dStage_roomStatus_c, mFlags,   0x21C);
WWHD_ASSERT_OFFSET(dStage_roomStatus_c, mDraw,    0x21D);
WWHD_ASSERT_OFFSET(dStage_roomStatus_c, mZoneNo,  0x21F);
WWHD_ASSERT_OFFSET(dStage_roomStatus_c, mProcID,  0x224);
WWHD_ASSERT_SIZE  (dStage_roomStatus_c,           0x22C);

/* --- WHICH ROOM NUMBER? -------------------------------------------------
 * There are four, they disagree, and picking the wrong one is the easiest
 * mistake to make here. Measured by what the save switch/item routers are
 * actually handed across their ~650 call sites:
 *
 *   home.roomNo     actor +0x2FE   346 sites   the room an actor BELONGS to
 *   current.roomNo  actor +0x326   189 sites   where an actor IS right now
 *   mStayNo         global          41 sites   the game's current room
 *   mCurStage.mRoomNo              (not used)  where the STAGE was entered
 *
 * Rules of thumb:
 *   - "Where is Link?"                 daPy_getRoomNo()      (current.roomNo)
 *   - "Which room owns this flag?"     the actor's home.roomNo
 *   - "What room is the game in?"      dStage_getStayNo()
 *   - "How did we enter this stage?"   dComIfGp_getCurStageRoomNo()
 *
 * They agree in a single-room stage, which is exactly why a mix-up survives
 * testing and then misroutes a save flag somewhere sprawling like the sea.
 * ---------------------------------------------------------------------- */

/** [V] The game-global current room, or -1 before a region is selected.
 *  dStage_roomControl_c::mStayNo. */
static __inline s8 dStage_getStayNo(void) {
    if (!wwhd_regionResolved)
        return (s8)-1;
    return *WWHD_AT_DATA(s8, wwhd_map->roomStayNo);
}

/** [V] The previous room, which the room control keeps in the next byte. */
static __inline s8 dStage_getOldStayNo(void) {
    if (!wwhd_regionResolved)
        return (s8)-1;
    return *WWHD_AT_DATA(s8, wwhd_map->roomStayNo + 1);
}

/** [V] The room table, or NULL before a region is selected. */
static __inline dStage_roomStatus_c* dStage_getRoomStatusTable(void) {
    if (!wwhd_regionResolved)
        return (dStage_roomStatus_c*)0;
    return WWHD_AT_DATA(dStage_roomStatus_c, wwhd_map->roomStatus);
}

/** [V] One room's record, or NULL if the index is out of range. */
static __inline dStage_roomStatus_c* dStage_getRoomStatus(int roomNo) {
    dStage_roomStatus_c* t = dStage_getRoomStatusTable();
    if (!t || roomNo < 0 || roomNo >= WWHD_ROOM_MAX)
        return (dStage_roomStatus_c*)0;
    return &t[roomNo];
}

/** [V] Zone id for a room, or -1 when unavailable. */
static __inline s8 dStage_getZoneNo(int roomNo) {
    dStage_roomStatus_c* r = dStage_getRoomStatus(roomNo);
    return r ? r->mZoneNo : (s8)-1;
}

/** [V] Room process id, or 0 when unavailable. */
static __inline u32 dStage_getRoomProcID(int roomNo) {
    dStage_roomStatus_c* r = dStage_getRoomStatus(roomNo);
    return r ? r->mProcID : 0u;
}

/* ========================================================================
 * The next-stage request - this is what a warp actually is
 * ===================================================================== */

/**
 * [V] dStage_startStage_c - a stage reference. dComIfG_play_c holds the CURRENT
 * stage as one of these; dStage_nextStage_c extends it with the request flag.
 *
 * Verified in WWHD by usage at play+0x3E94: the name is taken by `addi` (its
 * address handed to string routines) in 208 functions and read 578 times, with
 * an s16 at +0x08 and bytes at +0x0A/+0x0B - exactly these types. The layout is
 * identical to GameCube.
 */
typedef struct dStage_startStage_c {
    /* 0x00 */ char mName[8];   /* [V] */
    /* 0x08 */ s16  mPoint;     /* [V] */
    /* 0x0A */ s8   mRoomNo;    /* [V] */
    /* 0x0B */ s8   mLayer;     /* [V] */
} dStage_startStage_c;
WWHD_ASSERT_OFFSET(dStage_startStage_c, mPoint,  0x08);
WWHD_ASSERT_OFFSET(dStage_startStage_c, mRoomNo, 0x0A);
WWHD_ASSERT_SIZE  (dStage_startStage_c,          0x0C);

/**
 * [V] dStage_nextStage_c, at dComIfG_play_c + WWHD_PLAY_OFF_NEXTSTAGE.
 *
 * Every field was read out of WWHD: the setter strcpy's the name to +0x00, then
 * writes mPoint (sth +0x08), mRoomNo (stb +0x0A), mLayer (stb +0x0B), mWipe
 * (stb +0x0D) and mEnable (stb 1, +0x0C). The layout is identical to GameCube,
 * including the 0xE size.
 *
 * mEnable is a request flag, not a state: the game polls it, performs the load
 * and clears it. The setter REFUSES when it is already set, so a second warp
 * queued before the first is consumed is silently dropped rather than merged.
 */
typedef struct dStage_nextStage_c {
    /* 0x00 */ char mName[8];   /* [V] stage name, NUL-terminated by strcpy */
    /* 0x08 */ s16  mPoint;     /* [V] spawn point / start code */
    /* 0x0A */ s8   mRoomNo;    /* [V] */
    /* 0x0B */ s8   mLayer;     /* [V] -1 means "keep current" */
    /* 0x0C */ s8   mEnable;    /* [V] request pending */
    /* 0x0D */ s8   mWipe;      /* [V] transition style */
} dStage_nextStage_c;
WWHD_ASSERT_OFFSET(dStage_nextStage_c, mPoint,  0x08);
WWHD_ASSERT_OFFSET(dStage_nextStage_c, mRoomNo, 0x0A);
WWHD_ASSERT_OFFSET(dStage_nextStage_c, mLayer,  0x0B);
WWHD_ASSERT_OFFSET(dStage_nextStage_c, mEnable, 0x0C);
WWHD_ASSERT_OFFSET(dStage_nextStage_c, mWipe,   0x0D);
WWHD_ASSERT_SIZE  (dStage_nextStage_c,          0x0E);

/** [V] A stage name is at most 7 characters plus the terminator. */
#define WWHD_STAGE_NAME_MAX 8

/** [V] mLayer value meaning "keep the current layer". */
#define WWHD_STAGE_LAYER_KEEP ((s8)-1)

/* ========================================================================
 * Stage data and the save table
 * ===================================================================== */

/**
 * The STAG chunk of the loaded stage (GameCube stage_stag_info_class).
 *
 * mProp is the byte three unrelated functions shift right by one to get the
 * save-table index: the stage create at 0x025C1E44 before it calls getSave,
 * dStage_Delete at 0x025C3370 before it calls putSave, and the save routine
 * at 0x02721C44, which also bounds the result below dSv_STAGE_MAX. GameCube
 * keeps the same byte at +0x09 and reads (mProp >> 1) & 0x7F. The rest of the
 * record is the GameCube layout, unsighted here. Size not asserted.
 */
typedef struct dStage_stagInfo_c {
    /* 0x00 */ f32 mNearPlane;               /* [I] */
    /* 0x04 */ f32 mFarPlane;                /* [I] */
    /* 0x08 */ u8  mCameraMapToolID;         /* [I] */
    /* 0x09 */ u8  mProp;                    /* [V] bits 1..7 the save table */
    /* 0x0A */ u16 mParticleSceneNo;         /* [I] */
    /* 0x0C */ u32 mStageTypeAndSchbit;      /* [I] */
    /* 0x10 */ u32 mSchbitEnableAndFarPlane; /* [I] */
} dStage_stagInfo_c;
WWHD_ASSERT_OFFSET(dStage_stagInfo_c, mProp, 0x09);

/** [V] The dSv_memory slot a STAG record names, or -1 for NULL. The stage
 *  create and the stage delete compute exactly this. */
static __inline int dStage_stagInfo_getSaveTbl(const dStage_stagInfo_c* s) {
    return s ? (int)(s->mProp >> 1) : -1;
}

/**
 * dStage_dt_c - the loaded stage's chunk directory, embedded in dComIfG_play_c
 * at +0x3EB0: the play constructor at 0x0251FD94 calls its constructor on that
 * address, and standalone it allocates 0x7C. The chunk pointers are reached
 * through virtuals; the STAG pair is the one-instruction setter and getter at
 * 0x025C5038 (`stw r4, 0x48(r3)`) and 0x025C5040 (`lwz r3, 0x48(r3)`), slots
 * +0x154 and +0x15C of the vtable at 0x10055DE0. Only that member is named.
 */
typedef struct dStage_dt_c {
    /* 0x00 */ wwhd_gptr_t _vtable;              /* [V] 0x10055DE0 */
    /* 0x04 */ u8          _unk_04[0x48 - 0x04]; /* [?] the other chunks */
    /* 0x48 */ wwhd_gptr_t mpStagInfo;           /* [V] dStage_stagInfo_c*, or
                                                  *     0 before a stage loads */
} dStage_dt_c;
WWHD_ASSERT_OFFSET(dStage_dt_c, mpStagInfo, 0x48);

/** [V] The STAG record of a stage directory, or NULL before it is loaded. */
static __inline dStage_stagInfo_c* dStage_dt_getStagInfo(const dStage_dt_c* dt) {
    if (!dt || !dt->mpStagInfo)
        return (dStage_stagInfo_c*)0;
    return WWHD_AT(dStage_stagInfo_c, dt->mpStagInfo);
}

/* ========================================================================
 * Spawn points the player init treats specially
 * ===================================================================== */

/** [V] Read out of dStage_playerInit (0x025C1384): -1 places Link from the
 *  save's restart record, -2 from the ship data, -3 from the turn-restart
 *  record. Anything else is looked up in the room's PLYR list. */
#define WWHD_STAGE_POINT_RESTART      ((s16)-1)
#define WWHD_STAGE_POINT_SHIP         ((s16)-2)
#define WWHD_STAGE_POINT_TURN_RESTART ((s16)-3)

/* ========================================================================
 * The play scene's create phases
 * ===================================================================== */

/**
 * [V] dScnPly_Create hands the phase handler a table of eleven function
 * pointers in .data (wwhd_map->dScnPly_phaseTable) - GameCube has ten. Slot 3
 * is phase_1, which copies the next-stage record over the current one and
 * clears its enable flag; the body at wwhd_map->dScnPly_phase1 does exactly
 * that and is the one function in each build that formats
 * "Start StageName:RoomNo". It runs after the outgoing scene has been deleted
 * and before the stage archive is requested, which makes the slot the place to
 * interpose on a stage change. docs/SUBSYSTEMS.md (Save states).
 */
#define WWHD_DSCNPLY_PHASE_COUNT 11
#define WWHD_DSCNPLY_PHASE1_SLOT 3

typedef int (*dScnPly_phase_t)(void* scene);

/** [V] The live phase table, or NULL before a region is selected. */
static __inline wwhd_gptr_t* dScnPly_getPhaseTable(void) {
    if (!wwhd_regionResolved)
        return (wwhd_gptr_t*)0;
    return WWHD_AT_DATA(wwhd_gptr_t, wwhd_map->dScnPly_phaseTable);
}

/** [V] Link-time address of the phase_1 slot, for a data swap; 0 before a
 *  region is selected. */
static __inline wwhd_addr_t dScnPly_getPhase1SlotAddr(void) {
    if (!wwhd_regionResolved)
        return 0u;
    return wwhd_map->dScnPly_phaseTable + WWHD_DSCNPLY_PHASE1_SLOT * 4u;
}

#endif /* LIBWWHD_D_STAGE_H */
