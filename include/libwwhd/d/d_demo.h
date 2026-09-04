#ifndef LIBWWHD_D_DEMO_H
#define LIBWWHD_D_DEMO_H

#include "libwwhd/wwhd_types.h"
#include "libwwhd/wwhd_region.h"

/**
 * libwwhd - the cutscene system and its subsystem pointers (d_demo.cpp)
 *
 * d_demo is the richest unmapped area left in the binary: 61 anchored
 * functions. This header is the foothold rather than the map - it publishes the
 * subsystem pointers the demo initialiser wires up, all read out of one
 * function (d_demo.cpp:1741-1767) that asserts each one non-null in turn:
 *
 *     m_mesgControl != (0)   :1741      m_particle  != (0)   :1758
 *     m_system      != (0)   :1745      m_message   != (0)   :1761
 *     m_control     != (0)   :1747      m_factory   != (0)   :1764
 *     m_stage       != (0)   :1749      m_object    != (0)   :1767
 *     m_audio       != (0)   :1753
 *
 * The INTERIORS of these objects are unmapped, so only the pointers are
 * published. They are still useful: each is a live root to work outward from,
 * and the message controller is the way in to the dialogue state machine.
 *
 * FOR "IS DIALOGUE ON SCREEN", USE THE EVENT MODE INSTEAD. dEvt_control_c
 * carries dEvtMode_TALK_e and is already verified, so dEvt_getEventMode() ==
 * dEvtMode_TALK_e answers that today without any of this. The message
 * controller matters for WHICH message and what page, which is not mapped yet.
 */

/** [V] JMSControl* - the message controller. NULL before the demo system is
 *  built. Allocated with 0x74 bytes; interior [?]. */
static __inline wwhd_gptr_t dDemo_getMesgControl(void) {
    if (!wwhd_regionResolved)
        return 0u;
    return *WWHD_AT_DATA(wwhd_gptr_t, wwhd_map->mesgControl);
}

/** [V] JStudio control for the running demo, or 0. */
static __inline wwhd_gptr_t dDemo_getControl(void) {
    if (!wwhd_regionResolved)
        return 0u;
    return *WWHD_AT_DATA(wwhd_gptr_t, wwhd_map->demoControl);
}

/** [V] JStudio object for the running demo, or 0. */
static __inline wwhd_gptr_t dDemo_getObject(void) {
    if (!wwhd_regionResolved)
        return 0u;
    return *WWHD_AT_DATA(wwhd_gptr_t, wwhd_map->demoObject);
}

/** [V] JPAEmitterManager*, the particle system root. The same object
 *  dComIfG_play_c::mpParticleMng points at - which is how that field's name
 *  stopped being a guess. */
static __inline wwhd_gptr_t dDemo_getParticleMng(void) {
    if (!wwhd_regionResolved)
        return 0u;
    return *WWHD_AT_DATA(wwhd_gptr_t, wwhd_map->jpaEmitterMng);
}

#endif /* LIBWWHD_D_DEMO_H */
