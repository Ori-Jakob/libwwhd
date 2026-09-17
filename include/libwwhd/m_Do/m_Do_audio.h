#ifndef LIBWWHD_M_DO_AUDIO_H
#define LIBWWHD_M_DO_AUDIO_H

#include "libwwhd/wwhd_types.h"
#include "libwwhd/wwhd_map.h"
#include "libwwhd/wwhd_region.h"

/**
 * libwwhd - the audio manager (the object behind wwhd_map->audioMgr)
 *
 * One heap object the game keeps a pointer to at wwhd_map->audioMgr. Every
 * BGM decision goes through it. The offsets below were read out of four of
 * its routines in USA; the audio module sits at identical addresses in all
 * three builds and the wrappers in the 0x025E1xxx region load the same
 * pointer word in each, so the layout is taken as shared:
 *
 *   request  0x0202677C  picks the track for (stage index, room, layer),
 *                        stores the index at +0x290, the track through
 *                        0x020264F8 at +0x298, and sets +0x29C
 *   apply    0x02026E04  copies +0x290 to +0x294, swaps the wave banks
 *                        named at +0x2A2 and +0x2A5, clears +0x29C
 *   stop     0x02021F28  fades the handles at +0x78/+0x7C/+0x80 and writes
 *                        -1 to the playing id at +0x88
 *   busy     0x0202796C  reports the two bank sets' load state
 *
 * A stage index is the stage name's position in the 120-entry table at
 * 0x1018E6D0 plus one, 0 for a name that is not there; 0x75 has its own
 * bank-freeing path in 0x02027814. 0x020264F8 keeps a track that is already
 * playing rather than restarting it when the new request names the same id,
 * which is how the ocean theme survives a sea-to-sea change.
 */
#define WWHD_AUDIO_OFF_BGM_PLAYING     0x088 /* [V] u32 track id, -1 for none  */
#define WWHD_AUDIO_OFF_STAGE_REQUESTED 0x290 /* [V] s32 stage index requested  */
#define WWHD_AUDIO_OFF_STAGE_CURRENT   0x294 /* [V] s32 stage index applied    */
#define WWHD_AUDIO_OFF_BGM_PENDING     0x298 /* [V] u32 track id requested     */
#define WWHD_AUDIO_OFF_REQUEST_PENDING 0x29C /* [V] u8, 1 until applied        */
#define WWHD_AUDIO_OFF_BGM_RESTART     0x29E /* [V] u8, 1 = the track restarts */
#define WWHD_AUDIO_OFF_BANK_SET        0x2A2 /* [V] u8 set, +1 previous, +2 changed */
#define WWHD_AUDIO_OFF_BANK_SET2       0x2A5 /* [V] u8 set, +1 previous, +2 changed */

/** [V] The audio manager, or NULL before it exists or before a region. */
static __inline u8* mDoAud_getMgr(void) {
    u32 p;
    if (!wwhd_regionResolved)
        return (u8*)0;
    p = *WWHD_AT_DATA(u32, wwhd_map->audioMgr);
    return p ? WWHD_AT(u8, p) : (u8*)0;
}

static __inline u32 mDoAud_word(u32 ofs, u32 fallback) {
    const u8* m = mDoAud_getMgr();
    return m ? *(const u32*)(m + ofs) : fallback;
}

static __inline u8 mDoAud_byte(u32 ofs, u8 fallback) {
    const u8* m = mDoAud_getMgr();
    return m ? m[ofs] : fallback;
}

/** [V] Track id playing now, or 0xFFFFFFFF when none (also the stopped state). */
static __inline u32 mDoAud_getPlayingBgm(void) {
    return mDoAud_word(WWHD_AUDIO_OFF_BGM_PLAYING, 0xFFFFFFFFu);
}

/** [V] Track id of the last request, whether or not it has been applied. */
static __inline u32 mDoAud_getPendingBgm(void) {
    return mDoAud_word(WWHD_AUDIO_OFF_BGM_PENDING, 0xFFFFFFFFu);
}

/** [V] Stage index of the last request, or -1 without a manager. */
static __inline s32 mDoAud_getRequestedStage(void) {
    return (s32)mDoAud_word(WWHD_AUDIO_OFF_STAGE_REQUESTED, 0xFFFFFFFFu);
}

/** [V] Stage index the manager has applied, or -1 without a manager. */
static __inline s32 mDoAud_getCurrentStage(void) {
    return (s32)mDoAud_word(WWHD_AUDIO_OFF_STAGE_CURRENT, 0xFFFFFFFFu);
}

/** [V] Non-zero while a stage request waits for the apply step. */
static __inline int mDoAud_isRequestPending(void) {
    return mDoAud_byte(WWHD_AUDIO_OFF_REQUEST_PENDING, 0) != 0;
}

/** [V] Non-zero when the pending track will restart rather than continue. */
static __inline int mDoAud_isRestartWanted(void) {
    return mDoAud_byte(WWHD_AUDIO_OFF_BGM_RESTART, 0) != 0;
}

/** [V] The two wave-bank set indices the current track uses. */
static __inline u8 mDoAud_getBankSet(void)  { return mDoAud_byte(WWHD_AUDIO_OFF_BANK_SET, 0); }
static __inline u8 mDoAud_getBankSet2(void) { return mDoAud_byte(WWHD_AUDIO_OFF_BANK_SET2, 0); }

/** [V] Non-zero while a bank set differs from the one before the request. */
static __inline int mDoAud_bankSetChanged(void)  { return mDoAud_byte(WWHD_AUDIO_OFF_BANK_SET + 2, 0) != 0; }
static __inline int mDoAud_bankSet2Changed(void) { return mDoAud_byte(WWHD_AUDIO_OFF_BANK_SET2 + 2, 0) != 0; }

#endif /* LIBWWHD_M_DO_AUDIO_H */
