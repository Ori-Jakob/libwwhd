#ifndef LIBWWHD_H
#define LIBWWHD_H

/**
 * libwwhd - reference structures for The Legend of Zelda: The Wind Waker HD
 * (Wii U, cking.rpx). Umbrella header.
 *
 *     #include "libwwhd/libwwhd.h"
 *
 *     if (!wwhd_selectRegion(titleIdLo))
 *         return;                        // unknown build: keep features off
 *
 *     cXyz p = { 0.0f, 500.0f, 0.0f };
 *     daPy_teleport(&p);
 *     dComIfGs_getPlayerSave()->mPlayerStatusA.mRupee = 500;
 *
 * Every structure is a complete, castable C struct: known fields are named,
 * everything else is an explicit _unk_XXXX byte array, and each struct is
 * followed by static assertions on its member offsets and, where the size is
 * actually known, its total size. A layout mistake is a compile error.
 *
 * Read the confidence marker on a field before writing to it:
 *   [V] verified in WWHD  [P] probable  [I] GC-only, unconfirmed  [?] unknown
 *
 * NOTHING HERE WORKS UNTIL wwhd_selectRegion() SUCCEEDS. Addresses are per
 * build and 43 of 49 differ between USA and EUR, so there is no safe default;
 * accessors return NULL until a region is chosen. See wwhd_region.h.
 *
 * Configuration, before including this header:
 *   WWHD_PTR(addr)          how a guest address becomes a host pointer.
 *                           Default is identity - correct for code running
 *                           inside the game. External tools must override.
 *   WWHD_ENABLE_GAME_CALLS  opt in to helpers that CALL game routines
 *                           (currently get_daShip). In-game only.
 */

#include "libwwhd/wwhd_types.h"
#include "libwwhd/wwhd_map.h"
#include "libwwhd/wwhd_region.h"

#include "libwwhd/m_Do/m_Do_display.h"
#include "libwwhd/m_Do/m_Do_audio.h"

#include "libwwhd/SSystem/c_bg_w.h"
#include "libwwhd/SSystem/c_counter.h"

#include "libwwhd/f_op/f_op_actor.h"
#include "libwwhd/f_op/f_op_scene.h"

#include "libwwhd/d/d_save.h"
#include "libwwhd/d/d_save_bits.h"
#include "libwwhd/d/d_save_event.h"
#include "libwwhd/d/d_map.h"
#include "libwwhd/d/d_event.h"
#include "libwwhd/d/d_com_inf_game.h"
#include "libwwhd/d/d_save_access.h"
#include "libwwhd/d/d_item.h"
#include "libwwhd/d/d_save_item_access.h"
#include "libwwhd/d/d_figure.h"
#include "libwwhd/d/d_stage.h"
#include "libwwhd/d/d_stage_names.h"
#include "libwwhd/d/d_camera.h"
#include "libwwhd/d/d_demo.h"
#include "libwwhd/d/d_kankyo.h"
#include "libwwhd/d/d_meter.h"
#include "libwwhd/d/d_msg.h"

#include "libwwhd/d/actor/d_a_player_demo.h"
#include "libwwhd/d/actor/d_a_player_proc.h"
#include "libwwhd/d/actor/d_a_player.h"
#include "libwwhd/d/d_cc_s.h"
#include "libwwhd/d/d_cc_mass_s.h"
#include "libwwhd/d/actor/d_a_ship.h"
#include "libwwhd/d/actor/d_a_title.h"

#endif /* LIBWWHD_H */
