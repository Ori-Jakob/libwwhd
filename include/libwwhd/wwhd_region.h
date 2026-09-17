#ifndef LIBWWHD_REGION_H
#define LIBWWHD_REGION_H

#include "libwwhd/wwhd_types.h"
#include "libwwhd/wwhd_map.h"

/**
 * libwwhd - which build are we attached to.
 *
 * THE CONTRACT: nothing in libwwhd may be used until wwhd_selectRegion() has
 * returned non-zero. Until then wwhd_map is NULL and every accessor that
 * reaches game memory returns NULL or a zero value. This mirrors the existing
 * wwhd_dataResolved / wwhd_textResolved discipline rather than inventing a
 * second one, and it exists because the alternative - defaulting to USA - would
 * have a EUR player writing to arbitrary memory. 43 of 49 EUR addresses and 47
 * of 49 JAP addresses differ from USA, so a wrong table is not a near miss.
 *
 * ORDER MATTERS. The probe reads .text, which the console relocates, so:
 *
 *     wwhd_textDelta = infos[i].textOffset;   from OSDynLoad_GetRPLInfo
 *     wwhd_dataDelta = infos[i].dataOffset;
 *     wwhd_textResolved = wwhd_dataResolved = 1;
 *     if (!wwhd_selectRegion(titleId)) { disable game features; }
 *
 * Taking the deltas from the loader rather than probing a known table first is
 * required here: any table libwwhd could probe is itself region-specific.
 */

WWHD_SHARED const wwhd_map_t*        wwhd_map = 0;
WWHD_SHARED const wwhd_regionInfo_t* wwhd_regionInfo_p = 0;
WWHD_SHARED u32                      wwhd_regionResolved = 0;

/** [V] Wii U title IDs, low word. The high word is 0x00050000 for all three.
 *  These only order the probe candidates; the probe is what decides. */
#define WWHD_TITLELO_USA 0x10143500u  /* BCZE */
#define WWHD_TITLELO_EUR 0x10143600u  /* BCZP */
#define WWHD_TITLELO_JAP 0x10143400u  /* BCZJ */

/** [V] The Randomizer: the BCZE image with code appended, under its own title
 *  ID. Selected as WWHD_REGION_RANDO, which shares the USA table and differs
 *  only in textEnd - see the note above wwhd_regionInfo in wwhd_map.h. The
 *  probe cannot tell it from USA, so the title ID is what picks it. */
#define WWHD_TITLELO_RANDO 0x10143599u

/** The title the host is running, as OSGetTitleID reports it, or 0 when the
 *  host has not said. Published by the host at application start; libwwhd
 *  only reads it. */
WWHD_SHARED u64 wwhd_titleId = 0;

/** Non-zero when the running title is the Randomizer. */
static __inline int wwhd_isRandomizer(void) {
    return (u32)(wwhd_titleId & 0xFFFFFFFFu) == WWHD_TITLELO_RANDO;
}

/** Non-zero when `r` is the build actually mapped at the current text delta. */
static __inline int wwhd_probeRegion(wwhd_region_e r) {
    const wwhd_regionInfo_t* ri;
    const u32* p;
    int i;
    if (r <= WWHD_REGION_NONE || r >= WWHD_REGION_COUNT)
        return 0;
    if (!wwhd_textResolved)
        return 0;
    ri = &wwhd_regionInfo[r];
    p  = WWHD_AT_TEXT(const u32, ri->probeAddr);
    for (i = 0; i < WWHD_PROBE_WORDS; i++) {
        if ((p[i] & wwhd_probeMask[i]) != wwhd_probeWord[i])
            return 0;
    }
    return 1;
}

/**
 * Identify the running build and publish its table.
 *
 * `titleIdLo` is the low word of the Wii U title ID, or 0 if the caller does
 * not have it. It only orders the candidates; the probe is what decides, so a
 * title update that shifts code is refused rather than mismapped.
 *
 * Returns the selected region, or WWHD_REGION_NONE, in which case wwhd_map is
 * left NULL and the caller must keep game features disabled.
 */
static __inline wwhd_region_e wwhd_selectRegion(u32 titleIdLo) {
    wwhd_region_e first = WWHD_REGION_NONE;
    int i;

    if (titleIdLo != 0u) {
        if (titleIdLo == WWHD_TITLELO_USA)        first = WWHD_REGION_USA;
        else if (titleIdLo == WWHD_TITLELO_RANDO) first = WWHD_REGION_RANDO;
        else if (titleIdLo == WWHD_TITLELO_EUR)   first = WWHD_REGION_EUR;
        else if (titleIdLo == WWHD_TITLELO_JAP)   first = WWHD_REGION_JAP;
    }
    if (first != WWHD_REGION_NONE && wwhd_probeRegion(first)) {
        wwhd_map = wwhd_mapTable[first];
        wwhd_regionInfo_p = &wwhd_regionInfo[first];
        wwhd_regionResolved = 1u;
        return first;
    }
    for (i = WWHD_REGION_USA; i < WWHD_REGION_COUNT; i++) {
        if (wwhd_probeRegion((wwhd_region_e)i)) {
            wwhd_map = wwhd_mapTable[i];
            wwhd_regionInfo_p = &wwhd_regionInfo[i];
            wwhd_regionResolved = 1u;
            return (wwhd_region_e)i;
        }
    }
    wwhd_map = (const wwhd_map_t*)0;
    wwhd_regionInfo_p = (const wwhd_regionInfo_t*)0;
    wwhd_regionResolved = 0u;
    return WWHD_REGION_NONE;
}

/** "BCZE" / "BCZP" / "BCZJ", or "?" before a region is selected. */
static __inline const char* wwhd_regionCode(void) {
    return (wwhd_regionResolved && wwhd_regionInfo_p) ? wwhd_regionInfo_p->code : "?";
}

/** "USA" / "EUR" / "JAP", or "?" before a region is selected. */
static __inline const char* wwhd_regionName(void) {
    if (!wwhd_regionResolved || !wwhd_regionInfo_p)
        return "?";
    switch (wwhd_regionInfo_p->region) {
    case WWHD_REGION_USA:   return "USA";
    case WWHD_REGION_EUR:   return "EUR";
    case WWHD_REGION_JAP:   return "JAP";
    case WWHD_REGION_RANDO: return "RANDO";
    default:                return "?";
    }
}

/**
 * Adjust a dComIfG_play_c member offset for the running build.
 *
 * The struct in d_com_inf_game.h carries USA/EUR offsets. JAP drops 4 bytes
 * before +0x4810, so members at or after that sit 4 lower. Reach those through
 * the accessors in d_com_inf_game.h rather than as struct members.
 */
static __inline u32 wwhd_playOfs(u32 usaOfs) {
    if (!wwhd_regionResolved || !wwhd_regionInfo_p || usaOfs < WWHD_PLAY_TAIL_FROM)
        return usaOfs;
    return (u32)((s32)usaOfs + wwhd_regionInfo_p->playTailShift);
}

#endif /* LIBWWHD_REGION_H */
