#ifndef LIBWWHD_C_COUNTER_H
#define LIBWWHD_C_COUNTER_H

#include "libwwhd/wwhd_types.h"
#include "libwwhd/wwhd_region.h"

/**
 * libwwhd - the game frame counter (c_counter.cpp)
 *
 * This is the frame count a tool actually wants. It is advanced once per game
 * frame by cCt_Counter(0), called from fapGm_Execute after the process-manager
 * pass, so it does NOT tick while the game is paused or when a frame is
 * presented twice - which is exactly where an overlay's own present count goes
 * wrong.
 *
 * Found by shape, not by anchor: cCt_Counter is the only function in the whole
 * binary that compares r3 against 1 and stores to two adjacent .bss words. Its
 * single caller matches fapGm_Execute exactly, with fapGm_After sitting
 * immediately before it as the function pointer passed to fpcM_Management.
 * Same address in all three builds, confirmed by identical bytes.
 */

/** [V] counter_class - identical to the GameCube layout. */
typedef struct counter_class {
    /* 0x00 */ u32 mCounter0;  /* [V] free-running; +1 every game frame */
    /* 0x04 */ s32 mCounter1;  /* [V] +1 per frame, reset by cCt_Counter(1) */
    /* 0x08 */ u32 mTimer;     /* [P] read by many actors for cheap periodics;
                                *     not written by cCt_Counter */
} counter_class;
WWHD_ASSERT_OFFSET(counter_class, mCounter1, 0x04);
WWHD_ASSERT_OFFSET(counter_class, mTimer,    0x08);
WWHD_ASSERT_SIZE  (counter_class,            0x0C);

/** [V] The counter, or NULL before a region is selected. */
static __inline counter_class* cCt_getCounter(void) {
    if (!wwhd_regionResolved)
        return (counter_class*)0;
    return WWHD_AT_DATA(counter_class, wwhd_map->gCounter);
}

/** [V] Game frames elapsed, or 0 when unavailable. */
static __inline u32 cCt_getFrameCount(void) {
    counter_class* c = cCt_getCounter();
    return c ? c->mCounter0 : 0u;
}

/** [P] The periodic timer actors index sin/cos tables with. */
static __inline u32 cCt_getTimer(void) {
    counter_class* c = cCt_getCounter();
    return c ? c->mTimer : 0u;
}

#endif /* LIBWWHD_C_COUNTER_H */
