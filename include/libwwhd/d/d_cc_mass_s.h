#ifndef LIBWWHD_D_CC_MASS_S_H
#define LIBWWHD_D_CC_MASS_S_H

#include "libwwhd/wwhd_types.h"

/**
 * libwwhd - mass collision (d_cc_mass_s.cpp)
 *
 * The second collision system, and the one that carries every piece of
 * vegetation in the game. Grass, trees and flowers are drawn and updated in
 * batches by d_grass.cpp, d_tree.cpp and d_flower.cpp, and none of them owns
 * a cCcD_Obj. Instead the batch manager sets one shared cylinder attribute
 * for the whole species and then asks, per instance and per frame, whether
 * anything hit a cylinder of that size at that position. So a viewer that
 * walks the cCcS lists finds no vegetation at all: it was never registered.
 *
 * dCcMassS_Mng::Chk(cXyz* pos, fopAc_ac_c** hitActor, dCcMassS_HitInf* inf)
 * is that question. It writes the position into the shared cylinder, checks
 * it against the attack, body and area objects the frame registered, and
 * returns a bitmask. The shared radius and height come from an earlier
 * SetAttr call, which is inlined into each batch manager: grass uses 40 x 80,
 * trees 40 x 200, flowers 30 x 50.
 *
 * [V] Layout verified against USA 0x025170D8 (EUR 0x025170DC, JAP 0x025170E0,
 *     all three byte-identical, prologue stwu r1,-0xA0(r1) = 0x9421FF60):
 *     the function writes the argument position through +0x110, calls the
 *     AABB rebuild on +0xF0, tests the flag byte at +0x128, walks the object
 *     array at +0x44 with its count at +0x40 and the area array at +0xC0
 *     with its count at +0xBC, and forms a camera height as
 *     pos.y + *(f32*)(this + 0x120). Every one of those matches the GameCube
 *     dCcMassS_Mng member for member, so the class did not change size.
 */

/** [V] The shared cylinder attribute (cCcD_CylAttr) inside the manager. */
#define WWHD_CCMASS_OFF_CYL_ATTR   0x0F0
/** [V] Its cM3dGCyl geometry: centre, then radius and height at the same
 *  relative offsets a cCcD_Obj shape uses (+0x00, +0x0C, +0x10). */
#define WWHD_CCMASS_OFF_CYL_CENTER 0x110
#define WWHD_CCMASS_OFF_CYL_RADIUS 0x11C
#define WWHD_CCMASS_OFF_CYL_HEIGHT 0x120

/** [V] Bits of the value Chk returns. */
#define WWHD_CCMASS_HIT_AT   0x01   /* an attack shape reached the instance */
#define WWHD_CCMASS_HIT_CO   0x02   /* a body shape overlaps it */
#define WWHD_CCMASS_HIT_AREA 0x04   /* it is inside a registered area */

/** [V] Radius of the cylinder the manager is currently testing with. */
static __inline f32 dCcMassS_getRadius(const void* mng) {
    return mng ? *(const f32*)((const u8*)mng + WWHD_CCMASS_OFF_CYL_RADIUS) : 0.0f;
}

/** [V] Height of the cylinder the manager is currently testing with. */
static __inline f32 dCcMassS_getHeight(const void* mng) {
    return mng ? *(const f32*)((const u8*)mng + WWHD_CCMASS_OFF_CYL_HEIGHT) : 0.0f;
}

/** [V] Centre of that cylinder. Chk has just written the queried position
 *  here, so during the call it is the instance being tested. */
static __inline const cXyz* dCcMassS_getCenter(const void* mng) {
    return mng ? (const cXyz*)((const u8*)mng + WWHD_CCMASS_OFF_CYL_CENTER)
               : (const cXyz*)0;
}

#endif /* LIBWWHD_D_CC_MASS_S_H */
