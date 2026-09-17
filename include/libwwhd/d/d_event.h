#ifndef LIBWWHD_D_EVENT_H
#define LIBWWHD_D_EVENT_H

#include "libwwhd/wwhd_types.h"

/**
 * libwwhd - live event and event-camera state (d_event.cpp).
 *
 * These layouts are verified against the BCZE dComIfG_play_c constructor:
 * dEvt_control_c is built at play + 0x3F30 and dEvent_manager_c at
 * play + 0x4024.  Their internal offsets match the GameCube classes.
 */

enum {
    dEvtMode_NONE_e       = 0,
    dEvtMode_TALK_e       = 1,
    dEvtMode_DEMO_e       = 2,
    dEvtMode_COMPULSORY_e = 3,
};

/**
 * The per-frame check (0x0253FF34 in USA) is the GameCube one line for line:
 * event flag 8, which dComIfGp_event_reset() raises, is latched into
 * mbEndProc, and mbEndProc is only consumed (endProc) while mMode is not
 * NONE. Latched with no event in mode it stays armed and tears down the NEXT
 * event one frame after it starts. That armed state is the "storage" glitch;
 * a stage change clears it through remove().
 *
 * The two partner ids are the actors an event was ordered between. setParam
 * (0x0253F138 in USA) stores getPId(actor1) at +0xC4 and getPId(actor2) at
 * +0xC8, where getPId(NULL) is fpcM_ERROR_PROCESS_ID; talkEnd and demoEnd
 * read both back through convPId (0x0253EE04, fopAcM_SearchByID), and
 * moveApproval lets either partner move during an event (dEvtMove_FORCE_e).
 * The trap: endProc and remove never clear them, so with mMode back at NONE
 * they still name the previous event's partners, and anything that forces a
 * mode without parking them leaves those two actors running.
 */
typedef struct dEvt_control_c {
    /* 0x000 */ u8  _unk_000[0xC2];        /* [?] event orders and actor IDs */
    /* 0x0C2 */ u8  mMode;                 /* [V] dEvt_control_c::runCheck is mMode != 0 */
    /* 0x0C3 */ u8  mbEndProc;             /* [V] end request latched from mEventFlag bit 8 */
    /* 0x0C4 */ u32 mPt1;                  /* [V] partner 1 process id, fpcM_ERROR_PROCESS_ID for none */
    /* 0x0C8 */ u32 mPt2;                  /* [V] partner 2 process id */
    /* 0x0CC */ u8  _unk_0CC[0xE8 - 0xCC]; /* [?] talk/item partner ids, event id, talk button */
    /* 0x0E8 */ u16 mEventFlag;            /* [V] bit 8 = end request, bit 4 = treasure */
    /* 0x0EA */ u8  _unk_0EA[0xF4 - 0xEA]; /* [?] tact free-stick tables */
} dEvt_control_c;
WWHD_ASSERT_OFFSET(dEvt_control_c, mMode,      0xC2);
WWHD_ASSERT_OFFSET(dEvt_control_c, mbEndProc,  0xC3);
WWHD_ASSERT_OFFSET(dEvt_control_c, mPt1,       0xC4);
WWHD_ASSERT_OFFSET(dEvt_control_c, mPt2,       0xC8);
WWHD_ASSERT_OFFSET(dEvt_control_c, mEventFlag, 0xE8);
WWHD_ASSERT_SIZE  (dEvt_control_c, 0xF4);

typedef struct dEvent_manager_c {
    /* 0x000 */ u8  _unk_000[0x20]; /* [?] event list */
    /* 0x020 */ u32 mCameraPlay;    /* [V] nonzero while the event owns camera */
    /* 0x024 */ u8  _unk_024[0x540 - 0x24]; /* [?] */
} dEvent_manager_c;
WWHD_ASSERT_OFFSET(dEvent_manager_c, mCameraPlay, 0x20);
WWHD_ASSERT_SIZE  (dEvent_manager_c, 0x540);

#endif /* LIBWWHD_D_EVENT_H */
