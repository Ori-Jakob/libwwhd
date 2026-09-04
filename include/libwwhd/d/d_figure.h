#ifndef LIBWWHD_D_FIGURE_H
#define LIBWWHD_D_FIGURE_H

#include "libwwhd/wwhd_types.h"
#include "libwwhd/d/d_save.h"
#include "libwwhd/d/d_save_bits.h"
#include "libwwhd/d/d_save_access.h"

/**
 * libwwhd - the Nintendo Gallery's figurines
 *
 * Ownership is not an item and not a bank of its own. It is seventeen bytes of
 * the persistent event bank (dSv_event_c), addressed through a table of
 * event-register ids. The gallery's own test, 0x0229FE28 in USA, is
 *
 *     if (no < 134 && (no >> 3) < 17)
 *         return getEventReg(mEvent, regTable[no >> 3]) & (1 << (no & 7));
 *
 * with regTable the seventeen u16s at 0x101C2478 (USA), every one a full-byte
 * register (mask 0xFF). The setter at 0x0229FF88 is the second sighting: it
 * reads the same register, ORs in 1 << (no & 7), writes it back through
 * setEventReg, and raises event bit 0x3A01 for good measure. The owned count
 * at 0x0229FEAC runs the test over all 134, and Carlov's dialogue selector at
 * 0x022A2208 asks it about the figurine he is being offered. The 134 is also
 * the figurine data table's length: 134 records of 0x12 bytes at 0x10054718,
 * bounds-checked at 0x025BD64C.
 *
 * The register ids are event-bank addresses, not code addresses. The table is
 * byte-identical in all three builds (0x101C2478 in USA and EUR, 0x101C2498 in
 * JAP), so its contents are embedded here rather than reached through a map
 * slot.
 */

/** [V] Figurines the gallery knows, numbered 0..133 in the game's own order. */
#define WWHD_FIGURE_MAX 134

/** [V] Event registers holding the figurine bits, indexed by figurine >> 3. */
static const u16 wwhd_figureEventReg[17] = {
    0x95FF, 0x94FF, 0x93FF, 0x92FF, 0x91FF, 0x90FF, 0x8FFF, 0x8EFF, 0x8DFF,
    0x8CFF, 0xB1FF, 0x9CFF, 0x84FF, 0x83FF, 0x82FF, 0x81FF, 0x80FF,
};

/** [V] The packed event id of one figurine's bit: its register's byte with
 *  the single bit as the mask, which is what the event on/off/is routines
 *  take. 0 for a number the gallery does not know. */
static __inline u16 dSv_figureEventBit(int no) {
    if (no < 0 || no >= WWHD_FIGURE_MAX)
        return (u16)0;
    return (u16)((wwhd_figureEventReg[no >> 3] & 0xFF00u) | (1u << (no & 7)));
}

/** [V] Is figurine 0..133 owned. The gallery's test, bit for bit. */
static __inline int dSv_isFigure(int no) {
    u16 id = dSv_figureEventBit(no);
    return id != 0 && dSv_event_isEventBit(dComIfGs_getEvent(), id);
}

/** [V] Grant or take away figurine 0..133. */
static __inline void dSv_setFigure(int no, int on) {
    u16 id = dSv_figureEventBit(no);
    dSv_event_c* e = dComIfGs_getEvent();
    if (!e || !id)
        return;
    if (on)
        dSv_event_onEventBit(e, id);
    else
        dSv_event_offEventBit(e, id);
}

/** [V] Figurines owned, 0..134. Mirrors the count at 0x0229FEAC. */
static __inline int dSv_getFigureNum(void) {
    int i, n = 0;
    for (i = 0; i < WWHD_FIGURE_MAX; i++)
        if (dSv_isFigure(i))
            n++;
    return n;
}

#endif /* LIBWWHD_D_FIGURE_H */
