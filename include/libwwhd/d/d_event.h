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

typedef struct dEvt_control_c {
    /* 0x000 */ u8 _unk_000[0xC2]; /* [?] event orders and actor IDs */
    /* 0x0C2 */ u8 mMode;           /* [V] dEvt_control_c::runCheck is mMode != 0 */
    /* 0x0C3 */ u8 _unk_0C3[0xF4 - 0xC3]; /* [?] */
} dEvt_control_c;
WWHD_ASSERT_OFFSET(dEvt_control_c, mMode, 0xC2);
WWHD_ASSERT_SIZE  (dEvt_control_c, 0xF4);

typedef struct dEvent_manager_c {
    /* 0x000 */ u8  _unk_000[0x20]; /* [?] event list */
    /* 0x020 */ u32 mCameraPlay;    /* [V] nonzero while the event owns camera */
    /* 0x024 */ u8  _unk_024[0x540 - 0x24]; /* [?] */
} dEvent_manager_c;
WWHD_ASSERT_OFFSET(dEvent_manager_c, mCameraPlay, 0x20);
WWHD_ASSERT_SIZE  (dEvent_manager_c, 0x540);

#endif /* LIBWWHD_D_EVENT_H */
