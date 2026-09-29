#ifndef LIBWWHD_M_DO_RES_LOADER_H
#define LIBWWHD_M_DO_RES_LOADER_H

#include "libwwhd/wwhd_types.h"
#include "libwwhd/wwhd_map.h"
#include "libwwhd/wwhd_region.h"

/**
 * libwwhd - the archive loader (wwhd_map resLoader)
 *
 * Every archive request (dComIfG_setStageRes and friends, USA 0x0260D618) is
 * refcounted by key and queued in a ring of 0xA0-byte records. The per-frame
 * update (USA 0x0260C74C, the same bytes in EUR and JAP) loads the head of the
 * ring, registers it with the resource manager and only then pops it, so the
 * pending count includes the archive in flight. dComIfG_syncStageRes answers
 * "done" for anything that is not in the ring.
 *
 * [V] Releases are deferred while the loader is busy: a refcount drop that
 * would free an archive is queued instead (+0x1104 ring) and the update drains
 * that queue only once nothing is pending. A scene deleted in the middle of its
 * own loading therefore keeps every archive it held until the loader next goes
 * idle - and if the next scene starts requesting before that, the old scene's
 * memory is still allocated while the new one loads. Registration failures in
 * that state are not reported (USA 0x0260623C returns 0 and the caller at
 * 0x0260C290 ignores it), the archive is still popped as done, and the first
 * lookup of it comes back NULL: this is the d_stage.cpp:4871 stageRsrc assert
 * seen when a game is started from a title screen that is still loading.
 *
 * [V] The second way to the same assert: every stage archive's heap is made
 * inside the resource manager's two stage heaps (+0x2048/+0x204C, created on
 * demand by USA 0x02607F80/0x02604094), and deleting ANY resource named
 * "Stage" destroys both with all their children (USA 0x0260757C ->
 * 0x026072AC/0x02607414). An old scene whose delete runs after the next scene
 * has loaded its stage takes the new stage's archive down with it. The play
 * scene's own stage changes never overlap like that; a title screen deleted
 * while it still has work in flight (on hardware its IsDelete can lag for
 * frames) does. Hold the new scene until the old stage's archive is gone:
 * before phase_1 renames the stage, dComIfG_getStageRes("Stage", "stage.dzs")
 * still answers for the old one.
 */
#define WWHD_RESLOADER_OFF_PENDING  0x0D8  /* [V] s32, requests queued or in flight */
#define WWHD_RESLOADER_OFF_STATE    0x0DC  /* [V] s32, 0 idle, 1/2 loading */
#define WWHD_RESLOADER_OFF_DEFERRED 0x1110 /* [V] s32, releases waiting for idle */

/** [V] The loader, or NULL before it exists. */
static __inline void* wwhd_getResLoader(void) {
    u32 p;
    if (!wwhd_regionResolved)
        return (void*)0;
    p = *WWHD_AT_DATA(u32, wwhd_map->resLoader);
    return p ? WWHD_AT(void, p) : (void*)0;
}

/** [V] Pending requests, in-flight included; -1 without a loader. */
static __inline s32 wwhd_resLoaderPending(void) {
    const u8* l = (const u8*)wwhd_getResLoader();
    return l ? *(const s32*)(l + WWHD_RESLOADER_OFF_PENDING) : -1;
}

/** [V] Deferred releases not yet applied; -1 without a loader. */
static __inline s32 wwhd_resLoaderDeferred(void) {
    const u8* l = (const u8*)wwhd_getResLoader();
    return l ? *(const s32*)(l + WWHD_RESLOADER_OFF_DEFERRED) : -1;
}

/** [V] Nothing loading, nothing queued and every deferred release applied:
 *  the state a scene change from the file select always starts in. */
static __inline int wwhd_resLoaderSettled(void) {
    const u8* l = (const u8*)wwhd_getResLoader();
    if (!l)
        return 0;
    return *(const s32*)(l + WWHD_RESLOADER_OFF_PENDING) == 0 &&
           *(const s32*)(l + WWHD_RESLOADER_OFF_STATE) == 0 &&
           *(const s32*)(l + WWHD_RESLOADER_OFF_DEFERRED) == 0;
}

#ifdef WWHD_ENABLE_GAME_CALLS
typedef void* (*dComIfG_getStageRes_t)(const char* arc, const char* file);

/** [V] A file of the current stage's archive, or NULL when it is not loaded. */
static __inline void* dComIfG_getStageRes(const char* arc, const char* file) {
    if (!wwhd_textResolved || !wwhd_regionResolved)
        return (void*)0;
    return WWHD_FN(dComIfG_getStageRes_t, wwhd_map->dComIfG_getStageRes)(arc, file);
}
#endif /* WWHD_ENABLE_GAME_CALLS */

#endif /* LIBWWHD_M_DO_RES_LOADER_H */
