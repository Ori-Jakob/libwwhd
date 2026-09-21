#ifndef LIBWWHD_MAP_H
#define LIBWWHD_MAP_H

#include "libwwhd/wwhd_types.h"

/**
 * libwwhd - the per-region address table.
 *
 * Struct LAYOUTS are shared between builds; absolute ADDRESSES are not. Every
 * address libwwhd knows lives in the one table below, with all three builds on
 * the same row, because the failure this guards against is a single address
 * ported wrong in a single region: invisible across three separate files,
 * obvious in one column.
 *
 * Layout sharing is measured, not assumed (docs/REGIONS.md):
 *   USA == EUR   28927 matched functions, zero member-offset differences.
 *   JAP          identical except dComIfG_play_c, which loses 4 bytes before
 *                +0x4810. See WWHD_PLAY_TAIL_FROM and d_com_inf_game.h.
 *
 * Every address was ported by tools/PortMap.java with at least two independent
 * strategies agreeing, then checked instruction-by-instruction against USA.
 */

typedef enum wwhd_region_e {
    WWHD_REGION_NONE = 0,
    WWHD_REGION_USA,       /* BCZE */
    WWHD_REGION_EUR,       /* BCZP */
    WWHD_REGION_JAP,       /* BCZJ */
    WWHD_REGION_RANDO,     /* the Randomizer: BCZE with code appended, USA table */
    WWHD_REGION_COUNT
} wwhd_region_e;

/* Segment a slot lives in. Call sites still pick WWHD_AT_TEXT / WWHD_AT_DATA
 * themselves; this is here so the porting tool knows which matching strategy
 * applies to a slot, and so a debug dump can label one. */
#define WWHD_SEG_TEXT 0
#define WWHD_SEG_DATA 1

/* --- The slot table ------------------------------------------------------
 * X(name, seg, USA, EUR, JAP)
 * Marker and evidence sit above each row. Where the evidence runs to a
 * paragraph it lives beside the accessor that uses it instead.
 * ---------------------------------------------------------------------- */
#define WWHD_MAP_SLOTS(X)                                                      \
    /* [V] JUT_ShowAssert(file, line, expr). 2716 call sites; the anchor the   \
     *     whole library is derived from, and what ExtractAsserts keys on. */  \
    X(ShowAssert,           TEXT, 0x0273AA24u, 0x0273B2E0u, 0x0273B510u)       \
    /* [V] Same shape, non-returning (halt). */                                \
    X(ShowAssertHalt,       TEXT, 0x0273AA38u, 0x0273B2F4u, 0x0273B524u)       \
                                                                               \
    /* --- d_com_inf_game --- */                                               \
    /* [V] Static gameInfo. USA/EUR size 0x62F8, JAP 0x62F0. */                \
    X(gameInfoStatic,       DATA, 0x1046F0B0u, 0x1046F0B0u, 0x1046F0B0u)       \
    /* [V] Lazy accessor returning gameInfoStatic. */                          \
    X(getGameInfoStatic,    TEXT, 0x025200D4u, 0x025200D8u, 0x025200D8u)       \
    /* [V] Constructor for the static object. */                               \
    X(gameInfoStatic_ct,    TEXT, 0x02520030u, 0x02520034u, 0x02520034u)       \
    /* [V] POINTER to the heap gameInfo, not the object. Zero until built. */  \
    X(pGameInfoHeap,        DATA, 0x101F84DCu, 0x101F84F4u, 0x101F8524u)       \
    /* [V] Constructor/allocator for the heap object. */                       \
    X(gameInfoHeap_ct,      TEXT, 0x02720400u, 0x02720CBCu, 0x02720EECu)       \
    /* [V] dComIfGp_setNextStage - the warp primitive. Identified by the       \
     *     8-argument ("majroom",0,0,0xFF,0.0f,0,1,0) call in d_a_mo2.cpp. */  \
    X(setNextStage,         TEXT, 0x0252012Cu, 0x02520130u, 0x02520130u)       \
                                                                               \
    /* --- d_stage --- */                                                      \
    /* [V] dStage_roomStatus_c mStatus[WWHD_ROOM_MAX]. */                      \
    X(roomStatus,           DATA, 0x1047E6CCu, 0x1047E6CCu, 0x1047E6C4u)       \
    /* [V] getStatusProcID(roomNo). Asserts roomNo >= 0, roomProc != 0. */     \
    /* [V] dStage_roomControl_c::mStayNo - the game-global current room,      \
     *     with mOldStayNo in the byte after it. Sits 4 bytes before the      \
     *     record table in every build. 41 switch-routing call sites read     \
     *     the room number from here. */                                      \
    X(roomStayNo,           DATA, 0x1047E6C8u, 0x1047E6C8u, 0x1047E6C0u)      \
    X(roomProcGet,          TEXT, 0x025D98E8u, 0x025D98A8u, 0x025D98D4u)       \
    /* [V] dStage_roomControl_c::loadRoom(this, count, rooms): the room      \
     *     streaming step the player's room check (USA 0x025C35E8) runs on  \
     *     the RTBL entry of the room under Link. Walks the 64 status       \
     *     records at stride 0x22C; see d_stage_room.h. Only caller is the  \
     *     room check in every build. */                                     \
    X(dStage_loadRoom,      TEXT, 0x025C34ACu, 0x025C346Cu, 0x025C3498u)       \
    /* [V] dStage_roomControl_c::zoneCountCheck(this, roomNo): the zone      \
     *     bookkeeping plus setStayNo (USA 0x025C1070) the room check runs  \
     *     when the ground polygon's room differs from mStayNo. */           \
    X(dStage_zoneCountCheck, TEXT, 0x025C33E8u, 0x025C33A8u, 0x025C33D4u)      \
                                                                               \
    /* --- d_camera --- */                                                     \
    /* [V] s32 count, then dCamera_style_c[count]. dCamParam_c::Change       \
     *     (0x024F727C) reads both. .rodata, identical in all three builds;   \
     *     the first record is "NN00" in each. */                             \
    X(camStyleCount,        DATA, 0x10044878u, 0x10044878u, 0x10044878u)       \
    X(camStyleTable,        DATA, 0x1004487Cu, 0x1004487Cu, 0x1004487Cu)       \
    /* [V] dCamera_algEntry_c[dCamAlg_MAX] - the FlyCam hook point. */         \
    X(camAlgTable,          DATA, 0x101D5678u, 0x101D5678u, 0x101D5698u)       \
    /* [V] dCamera_c::Run - dispatches the mode fn, then commits work->live. */\
    X(dCam_run,             TEXT, 0x024FE3E8u, 0x024FE3ECu, 0x024FE3F0u)       \
    /* [V] Computes mEye from mCenter + mDirection; takes mCalcFlags. */       \
    X(dCam_calcTrans,       TEXT, 0x024FD11Cu, 0x024FD120u, 0x024FD124u)       \
    /* [V] RIDE (boat) mode function. (dCamera_c*, styleIdx) */                \
    X(dCam_rideCamera,      TEXT, 0x0250D4E8u, 0x0250D4ECu, 0x0250D4F0u)       \
    /* [V] DEMO mode function; Run calls it directly during a demo. */         \
    X(dCam_demoCamera,      TEXT, 0x024FC0A8u, 0x024FC0ACu, 0x024FC0B0u)       \
    /* [V] (this,&pos,actor,mask) -> 0 when the view is blocked. */            \
    X(dCam_bgCheck,         TEXT, 0x024FCBE8u, 0x024FCBECu, 0x024FCBF0u)       \
    /* [V] (this,&out,actorID) */                                              \
    X(dCam_actorPos,        TEXT, 0x024F8D5Cu, 0x024F8D60u, 0x024F8D64u)       \
    /* [V] (this,&out,actorID) */                                              \
    X(dCam_actorAngle,      TEXT, 0x024F8F30u, 0x024F8F34u, 0x024F8F38u)       \
    /* [V] (this,actorID) -> fopAc_ac_c* */                                    \
    X(dCam_actorIdToPtr,    TEXT, 0x02508A58u, 0x02508A5Cu, 0x02508A60u)       \
                                                                               \
    /* --- d_save: dSv_info_c-level routing. this = dSv_info_c* --- */         \
    /* [V] (this,no,roomNo) */                                                 \
    X(dSv_info_onSwitch,    TEXT, 0x025B9E38u, 0x025B9DF8u, 0x025B9E24u)       \
    /* [P] */                                                                  \
    X(dSv_info_offSwitch,   TEXT, 0x025B9F7Cu, 0x025B9F3Cu, 0x025B9F68u)       \
    /* [P] */                                                                  \
    X(dSv_info_isSwitch,    TEXT, 0x025BA0C0u, 0x025BA080u, 0x025BA0ACu)       \
    /* [P] */                                                                  \
    X(dSv_info_revSwitch,   TEXT, 0x025BA20Cu, 0x025BA1CCu, 0x025BA1F8u)       \
    /* [V] (this,no,roomNo) */                                                 \
    X(dSv_info_onItem,      TEXT, 0x025BA384u, 0x025BA344u, 0x025BA370u)       \
    /* [P] */                                                                  \
    X(dSv_info_isItem,      TEXT, 0x025BA494u, 0x025BA454u, 0x025BA480u)       \
    /* [V] (this,stageNo) */                                                   \
    X(dSv_info_getSave,     TEXT, 0x025B9C9Cu, 0x025B9C5Cu, 0x025B9C88u)       \
    /* [V] (this,stageNo) */                                                   \
    X(dSv_info_putSave,     TEXT, 0x025B9D24u, 0x025B9CE4u, 0x025B9D10u)       \
                                                                               \
    /* --- d_save: leaf bit banks. this = the bank, not dSv_info_c --- */      \
    /* [V] no < 128 */                                                         \
    X(dSv_memBit_onSwitch,  TEXT, 0x025B8CE0u, 0x025B8CA0u, 0x025B8CCCu)       \
    /* [V] no < 32 */                                                          \
    X(dSv_memBit_onItem,    TEXT, 0x025B8EC0u, 0x025B8E80u, 0x025B8EACu)       \
    /* [V] no < 32 */                                                          \
    X(dSv_memBit_isItem,    TEXT, 0x025B8F34u, 0x025B8EF4u, 0x025B8F20u)       \
    /* [V] no < 64 */                                                          \
    X(dSv_danBit_onSwitch,  TEXT, 0x025B91ACu, 0x025B916Cu, 0x025B9198u)       \
    /* [V] no < 48 */                                                          \
    X(dSv_zoneBit_onSwitch, TEXT, 0x025B93BCu, 0x025B937Cu, 0x025B93A8u)       \
    /* [V] no < 16 */                                                          \
    X(dSv_zoneBit_onItem,   TEXT, 0x025B95A0u, 0x025B9560u, 0x025B958Cu)       \
    /* [V] array constructor */                                                \
    X(dSv_zone_ct,          TEXT, 0x025B9774u, 0x025B9734u, 0x025B9760u)       \
    /* [V] (this,grid,bit). The salvage bank, not the sea chart - this was   \
     * named dSv_map_onGridBit until its asserts were traced to their owner. */\
    X(dSv_ocean_onSvBit,    TEXT, 0x025B89B4u, 0x025B8974u, 0x025B89A0u)       \
    /* [V] (this,grid,bit) */                                                  \
    X(dSv_ocean_isSvBit,    TEXT, 0x025B8A50u, 0x025B8A10u, 0x025B8A3Cu)       \
    /* [V] void (*l_itemGetFunc[256])(void), indexed by item number. Found     \
     * from the 61 chart give-stubs, whose slots land on item 0xC2..0xFE in    \
     * every build - which is what pins chartId = 0xFF - itemNo. */            \
    X(itemGetFuncTable,     DATA, 0x101E3A98u, 0x101E3A98u, 0x101E3AB8u)       \
    /* [V] The chart give-stub body: onGetMap, offOpenMap, offCompleteMap. */  \
    X(itemGetCollectMap,    TEXT, 0x02550310u, 0x02550314u, 0x02550314u)       \
                                                                               \
    /* --- d_a_player / d_a_ship --- */                                        \
    /* [V] Item acquisition gate; 0 means "you may have it". */                \
    X(daPy_checkGetItem,    TEXT, 0x025BBA1Cu, 0x025BB9DCu, 0x025BBA08u)       \
    /* [V] The cut function daShip_c::draw tests for. */                       \
    X(daShip_evtCutFn,      TEXT, 0x0247F0D0u, 0x0247F0D4u, 0x0247F0D8u)       \
                                                                               \
    /* --- d_a_player --- */                                                  \
    /* [V] Anchor for THREE adjacent .bss globals, not a struct base: Link's \
     *     current.angle at +0x00, shape_angle at +0x08 and current.pos at    \
     *     +0x40. The player execute reloads the actor from them every frame, \
     *     so writing the actor alone is undone. See d_a_player.h. Identical  \
     *     in all three builds, same five referencing functions in each. */   \
    X(lkTransformStore,     DATA, 0x1046CD08u, 0x1046CD08u, 0x1046CD08u)      \
                                                                              \
    /* [V] daPy_procEntry_c[222] - the player procedure table, indexed by     \
     *     mCurProc. The setter builds base + id*0xC, and the table ends in   \
     *     zeros after 222 entries. .rodata, so identical in all builds. */   \
    X(daPyProcTable,        DATA, 0x10036DF0u, 0x10036DF0u, 0x10036DF0u)                                                                                        /* [V] Link's swim speed, stock 18.0f. The swim procedure loads it fresh       *     every frame (lfs f31, 0x5638(r12) at 0x0242ED4C, immediately after      *     the mCurProc dispatch), which is why scaling speedF does nothing        *     and scaling THIS does. .rodata: byte-identical at this address in       *     all three builds. */                                                   X(swimSpeedConst,       DATA, 0x10035638u, 0x10035638u, 0x10035638u)                                                                                        /* [V] The King of Red Lions' sail speed, stock 55.0f. Same shape:             *     lfs f31, -0x58F8(r8) at 0x0247E994 with r8 = 0x10040000. .rodata,       *     identical in all three builds. */                                      X(sailSpeedConst,       DATA, 0x1003A708u, 0x1003A708u, 0x1003A708u)      \
                                                                              \
    /* --- c_counter: the game frame counter --- */                           \
    /* [V] counter_class g_Counter. mCounter0 advances once per game          \
     *     frame from fapGm_Execute, so it does NOT tick on a repeated        \
     *     presentation the way an overlay's own frame count does. */         \
    X(gCounter,             DATA, 0x101FF558u, 0x101FF558u, 0x101FF558u)      \
    /* [V] cCt_Counter(reset). Unique in the binary: the only function        \
     *     comparing r3 against 1 and storing to two adjacent .bss words. */  \
    X(cCt_Counter,          TEXT, 0x0200E6ECu, 0x0200E6ECu, 0x0200E6ECu)      \
    /* [V] fapGm_Execute - fpcM_Management(0, fapGm_After) then               \
     *     cCt_Counter(0). The counter's only caller. */                      \
    X(fapGm_Execute,        TEXT, 0x025D42ECu, 0x025D42ACu, 0x025D42D8u)      \
                                                                              \
    /* --- d_kankyo --- */                                                    \
    /* [V] dScnKy_env_light_c. Lazily constructed; see envLightGuard. */       \
    X(envLight,             DATA, 0x10475A68u, 0x10475A68u, 0x10475A60u)       \
    /* [V] Construct-once guard. Non-zero once built. */                       \
    X(envLightGuard,        DATA, 0x104773B4u, 0x104773B4u, 0x104773ACu)       \
    /* [V] The accessor carrying that guard. */                                \
    X(getEnvlight,          TEXT, 0x02555D0Cu, 0x02555D10u, 0x02555D10u)       \
    /* [V] Lighting colour solve; walks the time-band table. */                \
    X(kankyo_colorSolve,    TEXT, 0x025580FCu, 0x02558100u, 0x02558100u)       \
    /* [V] Per-frame environment update; calls the solve. */                   \
    X(kankyo_calc,          TEXT, 0x02558C40u, 0x02558C44u, 0x02558C44u)       \
                                                                               \
    /* --- collision --- */                                                   \
    /* [V] Ground height at a position. Takes a cXyz* and NOTHING else:       \
     *     it builds the dBgS_GndChk on its own stack with the game's own     \
     *     constructor, calls GroundCross, and folds in the sea surface.      \
     *     That is why libwwhd can offer a ground check without knowing       \
     *     the chk layout. Returns WWHD_BGS_NO_GROUND on a miss. */           \
    X(dBgS_getGroundY,      TEXT, 0x024F17D4u, 0x024F17D8u, 0x024F17DCu)      \
    /* [V] cBgS::GroundCross(cBgS*, chk) - the 256-entry walk itself. */      \
    X(cBgS_GroundCross,     TEXT, 0x02008974u, 0x02008974u, 0x02008974u)      \
    /* [V] dBgS::Regist(dBgW*, fopAc_ac_c*). Reads the actor's +0x32A and     \
     *     +0x326, which re-confirms shape_angle and current.roomNo. */       \
    X(dBgS_Regist,          TEXT, 0x024EEA6Cu, 0x024EEA70u, 0x024EEA74u)      \
                                                                              \
    /* --- d_demo: the cutscene system and its subsystem pointers --- */      \
    /* [V] JMSControl* m_mesgControl - the message controller, built          \
     *     with a 0x74-byte allocation. Interior unmapped. */                 \
    X(mesgControl,          DATA, 0x104756C8u, 0x104756C8u, 0x104756C0u)      \
    /* [V] JPAEmitterManager*. Corroborates the mpParticleMng name in         \
     *     dComIfG_play_c, which was offset-[V] but name-[P]. */              \
    X(jpaEmitterMng,        DATA, 0x1047B2D4u, 0x1047B2D4u, 0x1047B2CCu)      \
    /* [V] JStudio control / object for the running demo. */                  \
    X(demoControl,          DATA, 0x101D5FE8u, 0x101D5FE8u, 0x101D6008u)      \
    X(demoObject,           DATA, 0x101D5FFCu, 0x101D5FFCu, 0x101D601Cu)      \
    /* [V] d_demo.cpp:1741-1767, which wires all of the above and is          \
     *     the entry point for mapping the 61-anchor demo system. */          \
    X(demoSubsysInit,       TEXT, 0x025286E0u, 0x025286E4u, 0x025286E4u)      \
                                                                              \
    /* --- f_op_actor --- */                                                   \
    /* [V] (this,heap,size) */                                                 \
    X(fopAcM_createHeap,    TEXT, 0x025D5FECu, 0x025D5FACu, 0x025D5FD8u)       \
    /* [V] spawn relative to a parent */                                       \
    X(fopAcM_createChild,   TEXT, 0x025D5B20u, 0x025D5AE0u, 0x025D5B0Cu)       \
    /* [V] fopAcM_posMove(fopAc_ac_c*, const cXyz* offset): current.pos +=     \
     *     speed, plus the offset when non-NULL. Every actor execute ends      \
     *     in it, the boat's included: daShip_c::execute (0x02477A24 in        \
     *     USA) runs the current procedure, builds speed.x/z from speedF       \
     *     and current.angle.y, then calls this. Writing the boat's speed      \
     *     just before it is the boat's counterpart of                         \
     *     daPy_posMoveFromFootPos.                                            \
     *                                                                         \
     *     A leaf: first word lfs f12,0x340(r3) (0xC1830340), scratches r12    \
     *     and never touches r11. Its only same-module caller, the gravity     \
     *     variant 0x70 bytes later, keeps nothing live across it. All         \
     *     three builds read from their images: byte-identical, EUR at         \
     *     USA-0x40 and JAP at USA-0x14 like the fopAcM_* slots above, with    \
     *     the same 34 callers each. */                                        \
    X(fopAcM_posMove,       TEXT, 0x025D6800u, 0x025D67C0u, 0x025D67ECu)       \
    /* [V] (tag,&procName) */                                                  \
    X(fopAcM_searchByID,    TEXT, 0x025D5218u, 0x025D51D8u, 0x025D5204u)       \
    /* [V] Search tags handed to fopAcM_searchByID. These are label addresses  \
     *     in .text used as keys, NOT callable entry points. */                \
    X(tagActorSearch,       TEXT, 0x025E1234u, 0x025E11F4u, 0x025E1220u)       \
    X(tagProcSearch,        TEXT, 0x025E121Cu, 0x025E11DCu, 0x025E1208u)       \
    /* [V] Interleaved {f32 sin; f32 cos;} table, indexed (angle >> 3). */     \
    X(sinCosTable,          DATA, 0x104A44F8u, 0x104A4510u, 0x104A4640u)     \
    /* [V] The play scene's create-phase table: eleven function pointers  \
     *     dScnPly_Create (USA 0x025B31E4) hands the phase handler in r4,   \
     *     read from its lis/addi pair in every build. Slot 3 is phase_1. */ \
    X(dScnPly_phaseTable,   DATA, 0x101EAC64u, 0x101EAC64u, 0x101EAC84u)     \
    /* [V] phase_1 itself: the table's slot 3, and the one function in each \
     *     build that formats "Start StageName:RoomNo" - two routes agree.  \
     *     Copies the next-stage record over the current one and clears its \
     *     enable flag; the interposition point for a save-state load. */   \
    X(dScnPly_phase1,       TEXT, 0x025B1590u, 0x025B1550u, 0x025B1578u)     \
    /* [V] dStage_roomControl_c::mProcID: the id of the scene that owns the  \
     *     stage, 4 bytes before mStayNo. phase_1 writes it from the scene's \
     *     own id; dStage_playerInit reads it back for the search below. */  \
    X(roomProcId,           DATA, 0x1047E6C4u, 0x1047E6C4u, 0x1047E6BCu)     \
    /* [V] fopScnM_SearchByID: hands tagActorSearch the id. Used by          \
     *     dStage_playerInit; each port matched through that callback. */    \
    X(fopScnM_searchByID,   TEXT, 0x025DC80Cu, 0x025DC7CCu, 0x025DC7F8u)     \
    /* [V] fopScnM_ChangeReq(scene, name, overlap, 5, flag): what the play   \
     *     scene's draw, the stage select and the file select call to move  \
     *     to another scene. Read out of each build's draw. */               \
    X(fopScnM_changeReq,    TEXT, 0x025DC86Cu, 0x025DC82Cu, 0x025DC858u)     \
    /* [V] The audio side of a stage change. Every menu-driven start (the    \
     *     file select at 0x025ADC60, dComIfG_changeOpeningScene) and the    \
     *     play scene's own draw follow fopScnM_ChangeReq with this, as      \
     *     (&mNextStage, mNextStage.mRoomNo, mNextStage.mLayer). A one-      \
     *     shot: when bgmStageTimer is 0 it hands the record to the audio    \
     *     manager (0x02027814, which maps the stage name to its BGM) and    \
     *     sets the timer to 0x24. Read out of each build's draw. A stage    \
     *     entered without it starts with the audio manager still tuned      \
     *     to the scene before. */                                           \
    X(bgmStagePrepare,      TEXT, 0x025E17CCu, 0x025E178Cu, 0x025E17B8u)     \
    /* [V] The byte the routine above arms. The main frame (0x025F172C)      \
     *     counts it down to 1, and the play scene's first create phase      \
     *     (table slot 1, 0x025B13EC) waits while it is 2 or more, then      \
     *     applies the change and clears it. 0 is idle. Read out of that     \
     *     phase in each build. */                                           \
    X(bgmStageTimer,        DATA, 0x101F4707u, 0x101F4707u, 0x101F4727u)     \
    /* [V] Pointer to the overlap (fade) singleton at 0x1048A55C while a     \
     *     scene change with an overlap is in flight, 0 otherwise. Read      \
     *     out of fopScnM_ChangeReq's overlap step (0x025DBE80) in each      \
     *     build; that step refuses while it is set. */                      \
    X(sceneOverlap,         DATA, 0x101F36CCu, 0x101F36CCu, 0x101F36ECu)     \
    /* [V] The scene manager's "a change with an overlap is queued" flag,    \
     *     set by fopScnM_ChangeReq's core (0x025DCE04) and cleared by the   \
     *     request's last phase (0x025DCCE0). While it is set every          \
     *     further overlap ChangeReq returns 0, which the play scene's       \
     *     draw never checks. Read out of that core in each build. */        \
    X(sceneChangeBusy,      DATA, 0x101F37E0u, 0x101F37E0u, 0x101F3800u)     \
    /* [V] Fade every BGM handle out over N frames and clear the audio       \
     *     manager's BGM state (wraps 0x02021F28): what the play scene's     \
     *     draw does with 30 right after its ChangeReq and before priming    \
     *     the next stage's music. Read out of each build's draw. */         \
    X(bgmStopAll,           TEXT, 0x025E1904u, 0x025E18C4u, 0x025E18F0u)     \
    /* [V] Non-zero while the queued stage's BGM wave banks are still        \
     *     loading (0x0202796C checks the two banks of the stage's set).     \
     *     Create-phase table slot 5 (0x025B1F54) waits for 0. */            \
    X(bgmStageBusy,         TEXT, 0x025E18B8u, 0x025E1878u, 0x025E18A4u)     \
    /* [V] Request the twenty common wave banks (the table at 0x10003844)    \
     *     that every play stage's music needs, skipping any already         \
     *     loaded (wraps 0x02027A7C). The file select's create phase at      \
     *     0x025ADA7C is its only caller: a fresh boot's title never loads   \
     *     them, and the play scene's create-phase slot 5 waits for them     \
     *     through bgmStageBusy, so a play scene started straight from       \
     *     that title hangs on a black screen. USA and EUR identical; JAP    \
     *     read out of its file select, 4 bytes later. */                    \
    X(bgmCommonLoad,        TEXT, 0x02031090u, 0x02031090u, 0x02031094u)     \
    /* [V] Non-zero once all twenty common wave banks are resident, or       \
     *     when there is no audio manager (wraps 0x020278E8). The file       \
     *     select's next phase (0x025ADAC8) polls it before it goes on. */   \
    X(bgmCommonReady,       TEXT, 0x020310A4u, 0x020310A4u, 0x020310A8u)     \
    /* [V] The audio manager pointer: the word every stage BGM wrapper       \
     *     above loads its object from. The same address in all three        \
     *     builds, read out of each build's bgmStagePrepare. What is known   \
     *     of the object is in m_Do/m_Do_audio.h. */                         \
    X(audioMgr,             DATA, 0x101FFC78u, 0x101FFC78u, 0x101FFC78u)     \
    /* [V] mDoRst's data pointer: set at boot (0x025F1660) to a .bss block   \
     *     whose first word is the reset flag dComIfG_resetToOpening reads   \
     *     and the logo scene clears. Read out of each build's             \
     *     resetToOpening. */                                               \
    X(resetData,            DATA, 0x101F4974u, 0x101F4974u, 0x101F4994u)       \
                                                                               \
    /* --- d_msg: the message box --- */                                       \
    /* [V] dMsgBox_c::isDecidePressed - `return (this->+0x54 & 1) != 0`, the   \
     *     six instructions every waiting message state asks whether the       \
     *     player pressed the advance button. Nine callers: the page wait,     \
     *     the hand-send wait, both close waits, both choice boxes, the        \
     *     choice-list open, the number input and the NPC answer wait.         \
     *                                                                         \
     *     DO NOT REPLACE THIS WITH A C FUNCTION. It clobbers r0 and r3 and    \
     *     NOTHING ELSE, and its callers were compiled knowing that: the       \
     *     choice-list open holds `this` in r11 - a volatile register - across \
     *     the call and computes its state manager from r11 afterwards. An     \
     *     ordinary C replacement clobbers r4..r12, so `this` comes back as    \
     *     rubbish and the next state change runs on a pointer that is not a   \
     *     box. That is a hang or a crash a few frames later, in game code,    \
     *     with nothing of yours on the stack. Route the hook through an       \
     *     assembly shim that saves r4..r12, CTR and CR around the call.       \
     *                                                                         \
     *     Byte-identical in all three builds, which is how it was ported;     \
     *     the same six bytes also match one unrelated `&1` accessor 0x844     \
     *     lower in every build, so the nine callers are what pin it - the     \
     *     first of them is always the page wait at this address + 0x18. */    \
    X(dMsg_decideCheck,     TEXT, 0x026FCEC8u, 0x026FD70Cu, 0x026FD98Cu)       \
    /* [V] dMsgBox_c print step: how many characters the typing state prints   \
     *     this frame. Reads the OTHER input word, +0x50: bit 1 makes it jump  \
     *     mPrintPos straight to WWHD_MSG_PRINT_POS_MAX and bit 0 multiplies   \
     *     the rate, which is why holding the advance button turns pages but   \
     *     does not rush the text - the two are different words. Sits at       \
     *     dMsg_decideCheck - 0x3C8 in every build, the same first twelve      \
     *     bytes in each, with the typing state its second caller at +0x110.   \
     *     That fixed spacing is the port: the whole translation unit is       \
     *     laid out identically in the three images. */                        \
    X(dMsg_printStep,       TEXT, 0x026FCB00u, 0x026FD344u, 0x026FD5C4u)       \
    /* [V] dMsgObject_c* - the message manager, and the only pointer needed to \
     *     reach a live text box without a process search. Read out of the     \
     *     typing state, which opens `lis rX, 0x101f` then `lwz rX, off(rX)`   \
     *     in every build: 0x4B5C in USA and EUR, 0x4B74 in JAP. The typing    \
     *     state was itself located by its fixed spacing from                  \
     *     dMsg_decideCheck, so the three were read from the same place. */    \
    X(msgObject,            DATA, 0x101F4B5Cu, 0x101F4B5Cu, 0x101F4B74u)       \
    /* [V] fStateMgr getState(mgr) - returns the CURRENT state's descriptor,   \
     *     the same object the message code compares against when it asks      \
     *     which state a box is in (0x026B699C calls this then reads the       \
     *     descriptor's vtable at +0x08). A descriptor carries its name at     \
     *     +0x04, so this is how a caller turns a live box into a printable    \
     *     state name. Byte-identical at this address in all three builds,     \
     *     like the rest of the low framework. */                              \
    X(fStateMgr_getState,   TEXT, 0x02006478u, 0x02006478u, 0x02006478u)      \
    /* [V] dMsgBox_c input latch for the mActiveBox 0 and 1 boxes ONLY. Copies\
     *     the shared UI input record (see uiDisplayMgr) into the box: +0x50\
     *     from record+0x10C, +0x54 from record+0x00, +0x58 from record+0x00 |\
     *     record+0x08. It is a copy, not an accumulate - the box's own +0x54 \
     *     is overwritten outright, so a synthetic button written to that word\
     *     anywhere else in the frame is discarded here before any state reads\
     *     it. Its two callers, 0x026B3C24 (mActiveBox 0) and 0x026B67A4      \
     *     (mActiveBox 1), run it immediately before fStateMgr execute on     \
     *     box+0x18 with nothing in between.                                  \
     *                                                                        \
     *     IT IS NOT THE ONLY INPUT LATCH. Four other box updaters carry the  \
     *     same three stores without calling this one: 0x026BC2DC (mActiveBox \
     *     2/3/4) delegates to 0x026BC148, while 0x026ADE10 (mActiveBox 5),   \
     *     0x026B8DBC and 0x026B04B4 inline them. The last two are gated on   \
     *     their own state rather than on mActiveBox and are what drive a     \
     *     cutscene box, so a hook here reaches ordinary dialogue and misses  \
     *     demo text completely - confirmed on hardware, where a counter on   \
     *     this function never moved across a whole cutscene. To reach every  \
     *     class at once, write uiDisplayMgr instead.                       \
     *                                                                        \
     *     Both callers hold the box in r30 across the call, and the function \
     *     is a frameless leaf taking only r3, so replacing it stays safe     \
     *     under a backend whose stub clobbers r11. Located in each build by  \
     *     the store to 0x58(r3); all three bodies are identical and open     \
     *     `lis r12, 0x101f`. */                                              \
    X(dMsgBox_setInput,     TEXT, 0x026FF5ACu, 0x026FFE68u, 0x02700094u)      \
    /* [V] The Wii U display / UI-input manager, held as a POINTER: the       \
     *     word at this address is the manager, not the manager itself. Two   \
     *     things in it are known.                                            \
     *                                                                        \
     *     Manager + 0x00 is the UI input record every message box copies its \
     *     buttons from. Record word 0x00 becomes dMsgBox_c::mInputFlags (bit \
     *     0 decide, bit 1 cancel), word 0x08 is OR'd into the box's +0x58,   \
     *     and word 0x43 becomes dMsgBox_c::mPrintFlags. All six box updaters \
     *     read it, so it is the one lever that reaches every message class - \
     *     see dMsgBox_setInput and dMsg_getInputRecord().                    \
     *                                                                        \
     *     Manager + 0x1D0 is the display mode: 1 while the game plays on the \
     *     TV, 2 while it plays on the GamePad. 0 and 3 are two further       \
     *     targets the same setter accepts. Written by the setter at USA      \
     *     0x02618094 / EUR 0x02618798, whose only two callers are the        \
     *     ::StateID_ChangeToTvMode and ::StateID_ChangeToDrcMode state       \
     *     functions - identical apart from passing 1 and 2 - and read back by\
     *     the per-frame device pass at USA 0x02617AF4, which switches on the \
     *     same field. Offset confirmed in USA and EUR; treat a value outside \
     *     0..3 as unknown rather than trusting it.                           \
     *                                                                        \
     *     The base was read out of dMsgBox_setInput's `lwz r12, off(r12)` in \
     *     each build. */                                                     \
    X(uiDisplayMgr,         DATA, 0x101F5088u, 0x101F50A0u, 0x101F50D0u)      \
    /* [V] daPyProc_MOVE_e, Link's ground move procedure. It is proc 6 in     \
     *     daPyProcTable, whose entries are 12 bytes with the function pointer\
     *     at +0x04, so entry 6 is the word at daPyProcTable + 0x4C - read    \
     *     from there in each build rather than matched by shape. Entries 3   \
     *     and 4 in that table agree with daPyProc_CONTROLL_WAIT_e and        \
     *     daPyProc_WAIT_e, which is what pins the stride.                    \
     *                                                                        \
     *     Worth hooking because a ground speed boost written from outside the\
     *     player update is too late. The proc produces mNormalSpeed, and     \
     *     posMoveFromFootPos has already turned it into speedF by the time a \
     *     once-a-frame tick runs, so writing speedF from there changes what a\
     *     HUD reads and nothing else. swimSpeedConst carries the same lesson \
     *     for swimming. Boosting just after this returns lands between the   \
     *     proc producing mNormalSpeed and the conversion that consumes it.   \
     *                                                                        \
     *     Scale mNormalSpeed, not mMaxNormalSpeed. This proc reads +0x3C4 for\
     *     its own cap and 0x023E14A0 divides by it to pick walk against run  \
     *     animations, so inflating it makes Link walk-animate at running     \
     *     speed.                                                             \
     *                                                                        \
     *     A large non-leaf that calls out on every path, so no caller can    \
     *     keep anything live in a volatile across it and replacing it is safe\
     *     under a backend whose stub clobbers r11. */                        \
    X(daPy_procMove,        TEXT, 0x024198D4u, 0x024198D8u, 0x024198DCu)      \
    /* [V] daPyProc_CRAWL_MOVE_e, Link's crawl. Entry 16 of daPyProcTable,    \
     *     read from daPyProcTable + 0xC4 in each build - see daPy_procMove   \
     *     for the table shape.                                               \
     *                                                                        \
     *     Unlike the ground move it sets mNormalSpeed outright rather than   \
     *     ramping toward a target: both exits assign +0x6A14 to plus or minus\
     *     the value 0x0242B800 returns. So a boost applied after it returns  \
     *     cannot compound, because the next frame overwrites rather than     \
     *     accumulates. */                                                    \
    X(daPy_procCrawlMove,   TEXT, 0x0242C80Cu, 0x0242C810u, 0x0242C814u)      \
    /* [V] daPyProc_SWIM_MOVE_e. Entry 55 of daPyProcTable, so                \
     *     daPyProcTable + 0x298 - see daPy_procMove for the table shape.     \
     *     Prologue is stwu r1,-0x58(r1).                                     \
     *                                                                        \
     *     Hooked for the same reason as the ground move: an instant speed    \
     *     control that writes mNormalSpeed from a once-a-frame tick is racing\
     *     whatever consumes it. Applying just after this returns is          \
     *     unambiguous. */                                                    \
    X(daPy_procSwimMove,    TEXT, 0x0242F70Cu, 0x0242F710u, 0x0242F714u)     \
    /* [V] daPy_lk_c::posMoveFromFootPos, the one place mNormalSpeed becomes \
     *     movement. It scales +0x6A14 by the slope, stores speedF, splits   \
     *     that along current.angle.y into speed.x/z, adds gravity and then  \
     *     moves current.pos - the pipeline described at                     \
     *     WWHD_DAPY_OFF_NORMAL_SPEED. Called from posMove (0x023FDA70 in    \
     *     USA) AFTER the current procedure has run, so a value written just \
     *     before it wins over every procedure, including the idle ones the  \
     *     three hooks above never see.                                      \
     *                                                                       \
     *     Prologue stwu r1,-0x128(r1) (0x9421FED8). The body clobbers r12   \
     *     before its first branch and r11 on both sides of it, so no caller \
     *     keeps anything live in them across the call and a stub that uses  \
     *     r11 is safe. All three builds were read from their images and are \
     *     byte-identical apart from the +4 (EUR) and +8 (JAP) shifts, each  \
     *     with the same five call sites in posMove. */                      \
    X(daPy_posMoveFromFootPos, TEXT, 0x023FCB9Cu, 0x023FCBA0u, 0x023FCBA4u)  \
    /* [V] daPy_lk_c::draw. Paints the red damage fog into tevStr (fopAc     \
     *     +0x1B0) while mDamageWaitTimer (+0x3B0) is above zero, so the     \
     *     invincibility mod zeroes that timer around this call to hide      \
     *     the flash. Prologue stwu r1,-0x140(r1) (0x9421FEC0); the first    \
     *     64 bytes are identical in all three builds at the usual +4        \
     *     (EUR) and +8 (JAP) shifts. */                                     \
    X(daPy_draw,            TEXT, 0x023D9820u, 0x023D9824u, 0x023D9828u)     \
    /* [V] cCcS::Move, the hit collision resolve. Its object lists are       \
     *     complete on entry and zeroed on exit (see d/d_cc_s.h). Prologue   \
     *     mflr r0 (0x7C0802A6); byte-identical in all three builds, as      \
     *     is cCcS::Set at 0x0200E240. */                                    \
    X(cCcS_Move,            TEXT, 0x0200E558u, 0x0200E558u, 0x0200E558u)     \
    /* [V] dCcMassS_Mng::Chk - the vegetation collision question. Grass,    \
     *     trees and flowers own no cCcD_Obj; each instance asks this       \
     *     once a frame whether anything hit a cylinder of the species'     \
     *     shared size at its position (see d/d_cc_mass_s.h). Prologue      \
     *     stwu r1,-0xA0(r1) (0x9421FF60) in all three builds. */           \
    X(dCcMassS_Chk,         TEXT, 0x025170D8u, 0x025170DCu, 0x025170E0u)

typedef struct wwhd_map_t {
#define X(name, seg, usa, eur, jap) wwhd_addr_t name;
    WWHD_MAP_SLOTS(X)
#undef X
} wwhd_map_t;

/**
 * Per-region facts that are not addresses.
 *
 * probeAddr / probeWord0 / probeWord1 are the region check: the first two
 * instruction words of JUT_ShowAssert, read at THAT REGION'S OWN address for
 * it. The words are identical in every build because it is the same function,
 * so what discriminates is the address - probing a EUR image at the USA address
 * lands in unrelated code. All six cross-region probes were confirmed to
 * mismatch.
 *
 * playTailShift is added to dComIfG_play_c member offsets at or after
 * WWHD_PLAY_TAIL_FROM. Zero except on JAP.
 */
typedef struct wwhd_regionInfo_t {
    u32         region;
    const char* code;              /* "BCZE" / "BCZP" / "BCZJ" */
    wwhd_addr_t textEnd;           /* [V] last byte of .text */
    wwhd_addr_t bssEnd;            /* [V] last byte of .bss  */
    wwhd_addr_t probeAddr;         /* [V] JUT_ShowAssert; see the probe note */
    s32         playTailShift;     /* [V] */
    u32         gameInfoStaticSize;/* [V] from the constructor allocation */
} wwhd_regionInfo_t;

/**
 * [V] dComIfG_play_c members at or after this offset shift by playTailShift.
 *
 * Measured across the whole binary: 8463 differing member offsets over 28731
 * matched function pairs, every one exactly -4, lowest 0x4810. Members below it
 * are unchanged - the stage record at play+0x3EA0 reads identically in all
 * three builds.
 */
#define WWHD_PLAY_TAIL_FROM 0x4810

#define X(name, seg, usa, eur, jap) .name = usa,
static const wwhd_map_t wwhd_map_usa = { WWHD_MAP_SLOTS(X) };
#undef X
#define X(name, seg, usa, eur, jap) .name = eur,
static const wwhd_map_t wwhd_map_eur = { WWHD_MAP_SLOTS(X) };
#undef X
#define X(name, seg, usa, eur, jap) .name = jap,
static const wwhd_map_t wwhd_map_jap = { WWHD_MAP_SLOTS(X) };
#undef X

/**
 * [V] The probe signature: JUT_ShowAssert's first five instructions.
 *
 * MASKED, BECAUSE THE LOADER REWRITES TWO OF THEM. Words 1 and 2 are the
 * ha16/lo16 halves of a .data address, and the console patches both when it
 * relocates the segment - on a real Wii U word 1 reads 0x3CA01064 rather than
 * the 0x3CA01014 stored in the RPX, the 0x50 being the high half of the
 * 0x00502200 data offset. Comparing them verbatim is a probe that can only
 * ever match under an emulator, where the module stays at its link addresses.
 * That is exactly the relocation trap this library warns about, applied to the
 * one piece of code whose job is to detect the relocation.
 *
 * Word 3 is a condition-register op and word 4 a PC-relative branch, so both
 * survive relocation untouched. The pattern itself is byte-identical in all
 * three builds; what identifies a region is WHICH address carries it, which is
 * why probeAddr stays per-region and these do not.
 */
#define WWHD_PROBE_WORDS 5
static const u32 wwhd_probeWord[WWHD_PROBE_WORDS] = {
    0x7CA62B78u,  /* or    r6, r5, r5                          */
    0x3CA00000u,  /* lis   r5, ha16(msg)   - low half relocated */
    0x38A50000u,  /* addi  r5, r5, lo16(msg) - ditto            */
    0x4CC63182u,  /* crxor 4*cr1+eq, ...                        */
    0x48000124u   /* b     +0x124                               */
};
static const u32 wwhd_probeMask[WWHD_PROBE_WORDS] = {
    0xFFFFFFFFu, 0xFFFF0000u, 0xFFFF0000u, 0xFFFFFFFFu, 0xFFFFFFFFu
};

/**
 * [V] The Randomizer (wwhd_rando 1.2.0, title 0005000010143599) is the BCZE
 * image with 0x2E08 bytes of code appended to .text and 360 in-place patches
 * below 0x02700000; .rodata, .data and .bss keep their USA extents. Every one
 * of the 96 slots was compared against USA in Ghidra on 2026-09-17, whole
 * function bodies for the TEXT ones, and all are byte-identical, as are all
 * 48 sites the Cemu graphics pack patches. So the Randomizer reuses the USA
 * table outright; only textEnd differs. The probe cannot tell the two apart
 * (same JUT_ShowAssert at the same address), the title ID does.
 */
static const wwhd_regionInfo_t wwhd_regionInfo[WWHD_REGION_COUNT] = {
    { WWHD_REGION_NONE,  "",     0u,          0u,          0u,          0,  0u      },
    { WWHD_REGION_USA,   "BCZE", 0x028F87F3u, 0x104DA1C7u, 0x0273AA24u,  0, 0x62F8u },
    { WWHD_REGION_EUR,   "BCZP", 0x028F90B3u, 0x104DA2C7u, 0x0273B2E0u,  0, 0x62F8u },
    { WWHD_REGION_JAP,   "BCZJ", 0x028F92D3u, 0x104DA3C7u, 0x0273B510u, -4, 0x62F0u },
    { WWHD_REGION_RANDO, "BCZE", 0x028FB5FBu, 0x104DA1C7u, 0x0273AA24u,  0, 0x62F8u }
};

static const wwhd_map_t* const wwhd_mapTable[WWHD_REGION_COUNT] = {
    (const wwhd_map_t*)0, &wwhd_map_usa, &wwhd_map_eur, &wwhd_map_jap, &wwhd_map_usa
};

#endif /* LIBWWHD_MAP_H */
