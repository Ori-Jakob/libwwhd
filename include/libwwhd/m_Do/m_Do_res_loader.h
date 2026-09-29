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
 *
 * [V] The third way: an archive is read by the store (resStore) and registered
 * from its file map (+0x14, keyed "path_arc", USA lookup 0x026123FC). The store
 * has two worker threads sharing one busy word: 0 idle, 1 an archive read
 * (+0x11240, USA handler 0x02613394), 2 the boot preload of
 * Pack/szs_permanent0-2.pack (+0x11244, USA handler 0x026134B4). The preload is
 * started once per boot by play-scene phase slot 5 (USA 0x025B1F54 ->
 * 0x02612A80, flag resMgr +0x212C), so it runs under the title screen. A read
 * requested while the word is not 0 (USA 0x02612908) never sets it to 1 and is
 * served from resident packs only (USA 0x02610258); the loader's completion
 * waits only while the word is 1, so it registers at once and a non-resident
 * archive comes back done with nothing registered. On Cemu the preload is over
 * before anyone can start a game; on hardware it runs for seconds. Hold the new
 * scene while wwhd_resStoreBusy() != 0.
 *
 * [V] Slot 5 also waits for the layout loader (USA 0x101F7274, busy byte +0x20)
 * before starting the preload; its thread reads layouts through the same store
 * and adds to the same map outside the store's lock (USA 0x026FBC98 ->
 * 0x02612E64).
 */
#define WWHD_RESLOADER_OFF_PENDING  0x0D8  /* [V] s32, requests queued or in flight */
#define WWHD_RESLOADER_OFF_STATE    0x0DC  /* [V] s32, 0 idle, 1/2 loading */
#define WWHD_RESLOADER_OFF_DEFERRED 0x1110 /* [V] s32, releases waiting for idle */
#define WWHD_RESLOADER_OFF_HEAP     0x1114 /* [V] heap of the last archive started */

#define WWHD_RESSTORE_OFF_BUSY      0x11248 /* [V] s32, 0 idle, 1 archive read, 2 pack preload */
#define WWHD_RESSTORE_OFF_NAME      0x1124C /* [V] SafeString, "path_arc" being read */
#define WWHD_LAYOUTLOADER_OFF_BUSY  0x020   /* [V] u8, set from a job's request to its end */

/** [V] sead::SafeString as the loader takes it: the text, then the vtable. */
typedef struct wwhd_safeString_t {
    const char* str;
    const void* vtbl;
} wwhd_safeString_t;

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

/** [V] The heap the last started archive was read into, 0 if none. */
static __inline u32 wwhd_resLoaderHeap(void) {
    const u8* l = (const u8*)wwhd_getResLoader();
    return l ? *(const u32*)(l + WWHD_RESLOADER_OFF_HEAP) : 0u;
}

/** [V] The archive data store, or NULL before it exists. */
static __inline void* wwhd_getResStore(void) {
    u32 p;
    if (!wwhd_regionResolved)
        return (void*)0;
    p = *WWHD_AT_DATA(u32, wwhd_map->resStore);
    return p ? WWHD_AT(void, p) : (void*)0;
}

/** [V] The store's busy word: 0 idle, 1 archive read, 2 boot pack preload;
 *  -1 without a store. */
static __inline s32 wwhd_resStoreBusy(void) {
    const u8* s = (const u8*)wwhd_getResStore();
    return s ? *(const s32*)(s + WWHD_RESSTORE_OFF_BUSY) : -1;
}

/** [V] "path_arc" of the store's current or last read, "" without a store. */
static __inline const char* wwhd_resStoreReading(void) {
    const u8* s = (const u8*)wwhd_getResStore();
    const char* name;
    if (!s)
        return "";
    name = *(const char* const*)(s + WWHD_RESSTORE_OFF_NAME);
    return name ? name : "";
}

/** [V] 1 while the layout loader's thread has a job, 0 idle, -1 without it. */
static __inline s32 wwhd_layoutLoaderBusy(void) {
    u32 p;
    if (!wwhd_regionResolved)
        return -1;
    p = *WWHD_AT_DATA(u32, wwhd_map->layoutLoader);
    if (!p)
        return -1;
    return *WWHD_AT(const u8, p + WWHD_LAYOUTLOADER_OFF_BUSY) != 0;
}

/** [V] A const SafeString over text, as the game builds them. */
static __inline wwhd_safeString_t wwhd_safeString(const char* text) {
    wwhd_safeString_t s;
    s.str  = text;
    s.vtbl = WWHD_AT_DATA(u8, wwhd_map->safeStringVtbl);
    return s;
}

#ifdef WWHD_ENABLE_GAME_CALLS
typedef void* (*dComIfG_getStageRes_t)(const char* arc, const char* file);
typedef s32   (*dComIfG_syncStageRes_t)(const char* arc);
typedef u32   (*wwhd_resHash_t)(const char* text, s32 length);
typedef s32   (*wwhd_resLoader_request_t)(void* loader, const wwhd_safeString_t* path,
                                          const wwhd_safeString_t* arc, s32 front);
typedef void  (*wwhd_resLoader_forget_t)(void* loader, u32 hash);

/** [V] A file of the current stage's archive, or NULL when it is not loaded. */
static __inline void* dComIfG_getStageRes(const char* arc, const char* file) {
    if (!wwhd_textResolved || !wwhd_regionResolved)
        return (void*)0;
    return WWHD_FN(dComIfG_getStageRes_t, wwhd_map->dComIfG_getStageRes)(arc, file);
}

/** [V] 1 while the current stage's arc is queued or loading, 0 once it is
 *  not; -1 without a loader. */
static __inline s32 dComIfG_syncStageRes(const char* arc) {
    if (!wwhd_textResolved || !wwhd_getResLoader())
        return -1;
    return WWHD_FN(dComIfG_syncStageRes_t, wwhd_map->dComIfG_syncStageRes)(arc);
}

/** [V] The loader's key for path and arc: the hash of "path_arc". */
static __inline u32 wwhd_resKey(const char* path, const char* arc) {
    char key[128];
    s32 n = 0;
    const char* p;
    for (p = path; *p && n < (s32)sizeof(key) - 2; ++p)
        key[n++] = *p;
    key[n++] = '_';
    for (p = arc; *p && n < (s32)sizeof(key) - 1; ++p)
        key[n++] = *p;
    key[n] = '\0';
    return WWHD_FN(wwhd_resHash_t, wwhd_map->resHash)(key, n);
}

/** [V] Forgets path/arc and queues it again as a fresh request, for an archive
 *  the loader counted as done but never registered. 0 when the ring is full. */
static __inline s32 wwhd_resLoaderRequeue(const char* path, const char* arc) {
    void* loader = wwhd_getResLoader();
    wwhd_safeString_t sp, sa;
    if (!wwhd_textResolved || !loader)
        return 0;
    WWHD_FN(wwhd_resLoader_forget_t, wwhd_map->resLoader_forget)(loader, wwhd_resKey(path, arc));
    sp = wwhd_safeString(path);
    sa = wwhd_safeString(arc);
    return WWHD_FN(wwhd_resLoader_request_t, wwhd_map->resLoader_request)(loader, &sp, &sa, 0);
}
#endif /* WWHD_ENABLE_GAME_CALLS */

#endif /* LIBWWHD_M_DO_RES_LOADER_H */
