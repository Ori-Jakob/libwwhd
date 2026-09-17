#ifndef LIBWWHD_F_OP_SCENE_H
#define LIBWWHD_F_OP_SCENE_H

#include "libwwhd/wwhd_types.h"
#include "libwwhd/wwhd_region.h"

/**
 * libwwhd - the scene layer (f_op_scene_mng.cpp)
 *
 * A scene is a process, and its name at +0x08 says which kind it is. The
 * play scene's draw (USA 0x025AF8A0) polls the next-stage record only when the
 * scene it runs for is named 7, and the title screen is the same code running
 * under another name with Link sailing the title sea, so a warp queued there
 * sits unconsumed. What the title and the file select do to leave is the pair
 * every menu uses: fill the next-stage record, then
 * fopScnM_ChangeReq(scene, 7, 0, 5, 0) - the debug stage select at 0x025AC798
 * and dScnName's changeGameScene both do exactly that. The scene pointer is
 * fopScnM_SearchByID over dStage_roomControl_c::mProcID, which phase_1 stores
 * from the scene's own id (0x025B1590) and dStage_playerInit reads back
 * through the same search (0x025C1384). docs/SUBSYSTEMS.md (Scenes).
 */

/* [V] Scene names. 7 is what the play scene's draw compares its own +0x08
 * against and what every stage change passes.
 *
 * [V] 8 is the title screen. dComIfG_changeOpeningScene (0x02520230) queues
 * the "sea_T" stage and asks for scene 8; the logo scene (0x025ABF00) goes
 * there at boot and dComIfG_resetToOpening returns there. The logo and the
 * Start prompt are an actor inside it (d_a_title.cpp, 0x024B87A4) that finds
 * the stage scene through dStage_getStageProcID and asks it for scene 9 on
 * Start.
 *
 * [V] 10 exists but is not the boot title: the play scene changes to it at
 * 0x02707E78, after the ending. GameCube's TITLE_SCENE, kept for that. */
#define WWHD_SCENE_PLAY    7
#define WWHD_SCENE_OPENING 8
#define WWHD_SCENE_TITLE   10

/** [V] The overlap (fade) every stage change in the menus passes. */
#define WWHD_OVERLAP_FADE  0

/** [V] A scene's name, or -1 for NULL. Read at +0x08 by the play scene's
 *  draw and by dStage_playerInit. */
static __inline s16 fopScn_getName(const void* scene) {
    return scene ? *(const s16*)((const u8*)scene + 0x08) : (s16)-1;
}

/** [V] The process id of the scene that owns the stage, or 0 before a region
 *  is selected. Stale once that scene is gone, which is what makes the search
 *  below return NULL from the file select. */
static __inline u32 dStage_getStageProcID(void) {
    if (!wwhd_regionResolved)
        return 0u;
    return *WWHD_AT_DATA(u32, wwhd_map->roomProcId);
}

/** [V] The overlap (fade) singleton the scene manager keeps while a change
 *  with an overlap is in flight, as a game pointer; 0 otherwise. */
static __inline u32 fopScnM_getOverlap(void) {
    if (!wwhd_regionResolved)
        return 0u;
    return *WWHD_AT_DATA(u32, wwhd_map->sceneOverlap);
}

/** [V] Non-zero while a scene change with an overlap is queued or running.
 *  fopScnM_ChangeReq refuses another one until the request's last phase
 *  clears this, and the play scene's draw never checks that result. */
static __inline int fopScnM_isChangeBusy(void) {
    if (!wwhd_regionResolved)
        return 0;
    return *WWHD_AT_DATA(u32, wwhd_map->sceneChangeBusy) != 0u;
}

#ifdef WWHD_ENABLE_GAME_CALLS

typedef void* (*fopScnM_searchByID_t)(u32 id);
typedef int   (*fopScnM_changeReq_t)(void* scene, s16 name, s16 overlap, int arg, int flag);

/** [V] The scene that owns the stage, or NULL: outside play and title, or
 *  before the text delta is known. */
static __inline void* fopScnM_getStageScene(void) {
    fopScnM_searchByID_t search;
    u32 id;
    if (!wwhd_textResolved || !wwhd_regionResolved)
        return (void*)0;
    id = dStage_getStageProcID();
    if (id == 0u || id == 0xFFFFFFFFu)
        return (void*)0;
    search = WWHD_FN(fopScnM_searchByID_t, wwhd_map->fopScnM_searchByID);
    return search(id);
}

/** [V] Ask the scene manager to replace `scene` with the play scene:
 *  fopScnM_ChangeReq(scene, 7, overlap 0, peek 5, menuStyle). The last is
 *  HD's addition to ChangeReq; it reaches the overlap request (0x025DC344, a
 *  byte at +0x28) and from there the wipe manager. The file select
 *  (0x025ADC60) and the opening scene (0x025AEC74) pass 1, the play scene's
 *  own stage changes pass 0.
 *
 *  Pass 0 when `scene` is a play-type scene that owns a stage - the title
 *  screen included - so the change follows the play scene's own path. [P]
 *  With 1 the old scene outlives the creation of the new one, which is how
 *  a menu keeps drawing during the load. Observed on console: a title-to-
 *  play change asked for before the title has finished building (no Link
 *  yet) ends in the new scene's create-phase slot 4 asserting
 *  d_stage.cpp:4871 stageRsrc != 0, with either value. Wait for the
 *  title's Link, load the common wave banks first the way the file select
 *  does (dComIfG_loadCommonBgmBanks, then dComIfG_commonBgmBanksReady) -
 *  the play scene's create-phase slot 5 waits for them and a fresh boot's
 *  title never loads them - and stop the BGM (dComIfG_stopBgm) the way the
 *  play scene's draw does. The next-stage record must already be set.
 *  Non-zero when the request was queued. */
static __inline int fopScnM_changeToPlay(void* scene, int menuStyle) {
    fopScnM_changeReq_t change;
    if (!scene || !wwhd_textResolved || !wwhd_regionResolved)
        return 0;
    change = WWHD_FN(fopScnM_changeReq_t, wwhd_map->fopScnM_changeReq);
    return change(scene, (s16)WWHD_SCENE_PLAY, (s16)WWHD_OVERLAP_FADE, 5,
                  menuStyle ? 1 : 0);
}

#endif /* WWHD_ENABLE_GAME_CALLS */

#endif /* LIBWWHD_F_OP_SCENE_H */
