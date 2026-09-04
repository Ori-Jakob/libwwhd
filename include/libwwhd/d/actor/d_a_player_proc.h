#ifndef LIBWWHD_D_A_PLAYER_PROC_H
#define LIBWWHD_D_A_PLAYER_PROC_H

#include "libwwhd/wwhd_types.h"

/**
 * libwwhd - Link's player procedures (d_a_player_main.cpp and its .inc files)
 *
 * mCurProc is "what Link is currently doing" - the action state. It sits at
 * daPy_lk_c + 0x65F0, with its 12-byte pointer-to-member at +0x65F4.
 *
 * THE TABLE IS THE EVIDENCE. The proc setter builds `0x10036DF0 + id * 0xC`,
 * so there is a real table indexed by procedure id. It holds 222 entries of
 * {s16 thisOffset = 0, s16 vtableIndex = -1, u32 fn, u32 flags} and then
 * terminates in zeros. 222 also matches the highest id the player code ever
 * compares against (221), found by an independent scan.
 *
 * THE NAMES come from the GameCube enum, which has 219 entries (0..218) to
 * WWHD's 222. They are not applied blindly: HD lays each procedure family out
 * as a contiguous run of functions, so the two were aligned on group
 * boundaries.
 *
 *   ids 15..18   one address run  <-> GameCube CRAWL_START..CRAWL_END
 *   ids 19..22   another run      <-> GameCube WHIDE_READY..WHIDE_PEEP
 *   ids 12..14 and 23..25 return to the earlier run, as GameCube does
 *   ids 163..168 one run          <-> GameCube BOTTLE_* and FOOD_*
 *   id  169 opens the d_a_player_dproc.inc region (anchor 0x0241F9DC)
 *                                 <-> GameCube DEMO_TOOL_e, the first DEMO_*
 *
 * Four boundaries landing on identical ids is what justifies transcribing the
 * names for 0..218. The three extra procedures are APPENDED at 219..221, 
 * continuing the demo-procedure address run past GameCube's last entry, so
 * nothing is displaced and no name shifts.
 *
 * A name here is still an inference, not a reading: no procedure was confirmed
 * behaviourally. The ID and its FUNCTION are [V]; the NAME is [P].
 *
 * 152 of the 222 ids are demonstrably tested by HD player code, including all
 * three additions (219..221), so those are real procedures rather than padding.
 */

/** [V] One procedure-table entry. The same shape as dCamera_algEntry_c plus
 *  per-procedure flags: mVtableIndex is always -1, so mFn is a plain function
 *  pointer rather than a vtable slot. */
typedef struct daPy_procEntry_c {
    /* 0x0 */ s16         mThisOffset;  /* [V] always 0 */
    /* 0x2 */ s16         mVtableIndex; /* [V] always -1 */
    /* 0x4 */ wwhd_gptr_t mFn;          /* [V] the procedure function */
    /* 0x8 */ u32         mFlags;       /* [V] per-procedure flags; meaning [?] */
} daPy_procEntry_c;
WWHD_ASSERT_OFFSET(daPy_procEntry_c, mFn, 0x4);
WWHD_ASSERT_SIZE  (daPy_procEntry_c,      0xC);

/** [V] Entries in the table, and so the number of procedures. */
#define WWHD_DAPY_PROC_MAX 222

/** Procedure ids. Value and function [V]; name [P] - see the header note. */
enum daPy_proc_e {
    daPyProc_SCOPE_e                             =   0, /* 02415BA4 */
    daPyProc_SUBJECTIVITY_e                      =   1, /* 0241701C */
    daPyProc_CALL_e                              =   2, /* 024172D4 */
    daPyProc_CONTROLL_WAIT_e                     =   3, /* 02417388 */
    daPyProc_WAIT_e                              =   4, /* 024177A8 */
    daPyProc_FREE_WAIT_e                         =   5, /* 02417BFC */
    daPyProc_MOVE_e                              =   6, /* 024198D4 */
    daPyProc_ATN_MOVE_e                          =   7, /* 02419BF0 */
    daPyProc_ATN_ACTOR_WAIT_e                    =   8, /* 02419E00 */
    daPyProc_ATN_ACTOR_MOVE_e                    =   9, /* 02419E70 */
    daPyProc_SIDE_STEP_e                         =  10, /* 0241A604 */
    daPyProc_SIDE_STEP_LAND_e                    =  11, /* 0241A718 */
    daPyProc_CROUCH_DEFENSE_e                    =  12, /* 0241A798 */
    daPyProc_CROUCH_DEFENSE_SLIP_e               =  13, /* 0241AB6C */
    daPyProc_CROUCH_e                            =  14, /* 0241AC94 */
    daPyProc_CRAWL_START_e                       =  15, /* 0242C688 */
    daPyProc_CRAWL_MOVE_e                        =  16, /* 0242C80C */
    daPyProc_CRAWL_AUTO_MOVE_e                   =  17, /* 0242D1A4 */
    daPyProc_CRAWL_END_e                         =  18, /* 0242D7EC */
    daPyProc_WHIDE_READY_e                       =  19, /* 0242AA58 */
    daPyProc_WHIDE_WAIT_e                        =  20, /* 0242AD20 */
    daPyProc_WHIDE_MOVE_e                        =  21, /* 0242AFB8 */
    daPyProc_WHIDE_PEEP_e                        =  22, /* 0242B648 */
    daPyProc_WAIT_TURN_e                         =  23, /* 0241AF64 */
    daPyProc_MOVE_TURN_e                         =  24, /* 0241B050 */
    daPyProc_SLIP_e                              =  25, /* 0241B0C0 */
    daPyProc_SLIDE_FRONT_e                       =  26, /* 0241B32C */
    daPyProc_SLIDE_BACK_e                        =  27, /* 0241B578 */
    daPyProc_SLIDE_FRONT_LAND_e                  =  28, /* 0241B774 */
    daPyProc_SLIDE_BACK_LAND_e                   =  29, /* 0241B83C */
    daPyProc_FRONT_ROLL_e                        =  30, /* 0241BAB8 */
    daPyProc_FRONT_ROLL_CRASH_e                  =  31, /* 0241BC90 */
    daPyProc_NOCK_BACK_END_e                     =  32, /* 0241BD70 */
    daPyProc_SIDE_ROLL_e                         =  33, /* 0241BE78 */
    daPyProc_BACK_JUMP_e                         =  34, /* 0241C094 */
    daPyProc_BACK_JUMP_LAND_e                    =  35, /* 0241C140 */
    daPyProc_AUTO_JUMP_e                         =  36, /* 0241D79C */
    daPyProc_LAND_e                              =  37, /* 0241DB84 */
    daPyProc_LAND_DAMAGE_e                       =  38, /* 0241DC30 */
    daPyProc_FALL_e                              =  39, /* 0241DD5C */
    daPyProc_SLOW_FALL_e                         =  40, /* 0241DF20 */
    daPyProc_SMALL_JUMP_e                        =  41, /* 0241DFA0 */
    daPyProc_VERTICAL_JUMP_e                     =  42, /* 0241E034 */
    daPyProc_HANG_START_e                        =  43, /* 02428730 */
    daPyProc_HANG_FALL_START_e                   =  44, /* 0242888C */
    daPyProc_HANG_UP_e                           =  45, /* 02428AEC */
    daPyProc_HANG_WAIT_e                         =  46, /* 02428BF4 */
    daPyProc_HANG_MOVE_e                         =  47, /* 02428CB4 */
    daPyProc_HANG_CLIMB_e                        =  48, /* 02429100 */
    daPyProc_HANG_WALL_CATCH_e                   =  49, /* 02429160 */
    daPyProc_PUSH_PULL_WAIT_e                    =  50, /* 0243D1C4 */
    daPyProc_PUSH_MOVE_e                         =  51, /* 0243D424 */
    daPyProc_PULL_MOVE_e                         =  52, /* 0243D578 */
    daPyProc_SWIM_UP_e                           =  53, /* 0242F45C */
    daPyProc_SWIM_WAIT_e                         =  54, /* 0242F590 */
    daPyProc_SWIM_MOVE_e                         =  55, /* 0242F70C */
    daPyProc_LADDER_UP_START_e                   =  56, /* 02427974 */
    daPyProc_LADDER_UP_END_e                     =  57, /* 02427A74 */
    daPyProc_LADDER_DOWN_START_e                 =  58, /* 02427B20 */
    daPyProc_LADDER_DOWN_END_e                   =  59, /* 02427C40 */
    daPyProc_LADDER_MOVE_e                       =  60, /* 02427CA8 */
    daPyProc_CLIMB_UP_START_e                    =  61, /* 02429D8C */
    daPyProc_CLIMB_DOWN_START_e                  =  62, /* 02429E8C */
    daPyProc_CLIMB_MOVE_UP_DOWN_e                =  63, /* 02429F84 */
    daPyProc_CLIMB_MOVE_SIDE_e                   =  64, /* 0242A06C */
    daPyProc_CUT_A_e                             =  65, /* 0243F790 */
    daPyProc_CUT_F_e                             =  66, /* 0243FAFC */
    daPyProc_CUT_R_e                             =  67, /* 0243FF44 */
    daPyProc_CUT_L_e                             =  68, /* 024402B0 */
    daPyProc_CUT_EA_e                            =  69, /* 0244061C */
    daPyProc_CUT_EB_e                            =  70, /* 024408B0 */
    daPyProc_CUT_EX_A_e                          =  71, /* 02440B38 */
    daPyProc_CUT_EX_B_e                          =  72, /* 02440E5C */
    daPyProc_CUT_EX_MJ_e                         =  73, /* 02441104 */
    daPyProc_CUT_KESA_e                          =  74, /* 02441450 */
    daPyProc_WEAPON_NORMAL_SWING_e               =  75, /* 0243EA44 */
    daPyProc_WEAPON_SIDE_SWING_e                 =  76, /* 0243ED14 */
    daPyProc_WEAPON_FRONT_SWING_READY_e          =  77, /* 0243F058 */
    daPyProc_WEAPON_FRONT_SWING_e                =  78, /* 0243F1A8 */
    daPyProc_WEAPON_FRONT_SWING_END_e            =  79, /* 0243F2B0 */
    daPyProc_WEAPON_THROW_e                      =  80, /* 0243F34C */
    daPyProc_HAMMER_SIDE_SWING_e                 =  81, /* 0243C61C */
    daPyProc_HAMMER_FRONT_SWING_READY_e          =  82, /* 0243C954 */
    daPyProc_HAMMER_FRONT_SWING_e                =  83, /* 0243CA98 */
    daPyProc_HAMMER_FRONT_SWING_END_e            =  84, /* 0243CE98 */
    daPyProc_CUT_TURN_e                          =  85, /* 02441750 */
    daPyProc_CUT_ROLL_e                          =  86, /* 02441DC0 */
    daPyProc_CUT_ROLL_END_e                      =  87, /* 02442134 */
    daPyProc_CUT_TURN_CHARGE_e                   =  88, /* 0244240C */
    daPyProc_CUT_TURN_MOVE_e                     =  89, /* 02442470 */
    daPyProc_CUT_REVERSE_e                       =  90, /* 02442878 */
    daPyProc_JUMP_CUT_e                          =  91, /* 02442C1C */
    daPyProc_JUMP_CUT_LAND_e                     =  92, /* 02442DC0 */
    daPyProc_BT_JUMP_e                           =  93, /* 0242FCA0 */
    daPyProc_BT_JUMP_CUT_e                       =  94, /* 0242FDD0 */
    daPyProc_BT_SLIDE_e                          =  95, /* 0242FFD8 */
    daPyProc_BT_ROLL_e                           =  96, /* 024302C0 */
    daPyProc_BT_ROLL_CUT_e                       =  97, /* 024304C8 */
    daPyProc_BT_VERTICAL_JUMP_e                  =  98, /* 024307E4 */
    daPyProc_BT_VERTICAL_JUMP_CUT_e              =  99, /* 02430988 */
    daPyProc_BT_VERTICAL_JUMP_LAND_e             = 100, /* 02430C00 */
    daPyProc_GUARD_CRASH_e                       = 101, /* 0241E0D4 */
    daPyProc_DAMAGE_e                            = 102, /* 0241E150 */
    daPyProc_POLY_DAMAGE_e                       = 103, /* 0241E428 */
    daPyProc_LARGE_DAMAGE_e                      = 104, /* 0241E7D4 */
    daPyProc_LARGE_DAMAGE_UP_e                   = 105, /* 0241EA50 */
    daPyProc_LARGE_DAMAGE_WALL_e                 = 106, /* 0241EBB4 */
    daPyProc_LAVA_DAMAGE_e                       = 107, /* 0241EBFC */
    daPyProc_ELEC_DAMAGE_e                       = 108, /* 0241ECB0 */
    daPyProc_GUARD_SLIP_e                        = 109, /* 0241EDB8 */
    daPyProc_GRAB_READY_e                        = 110, /* 0242DAC8 */
    daPyProc_GRAB_UP_e                           = 111, /* 0242DCE0 */
    daPyProc_GRAB_MISS_e                         = 112, /* 0242DF08 */
    daPyProc_GRAB_THROW_e                        = 113, /* 0242E010 */
    daPyProc_GRAB_PUT_e                          = 114, /* 0242E264 */
    daPyProc_GRAB_WAIT_e                         = 115, /* 0242E604 */
    daPyProc_GRAB_HEAVY_WAIT_e                   = 116, /* 0242E9D4 */
    daPyProc_GRAB_REBOUND_e                      = 117, /* 0242EC50 */
    daPyProc_ROPE_SUBJECT_e                      = 118, /* 024350A0 */
    daPyProc_ROPE_READY_e                        = 119, /* 024352F4 */
    daPyProc_ROPE_SWING_e                        = 120, /* 02435580 */
    daPyProc_ROPE_HANG_WAIT_e                    = 121, /* 02436AB4 */
    daPyProc_ROPE_UP_e                           = 122, /* 02436E6C */
    daPyProc_ROPE_DOWN_e                         = 123, /* 02437000 */
    daPyProc_ROPE_SWING_START_e                  = 124, /* 02437134 */
    daPyProc_ROPE_MOVE_e                         = 125, /* 02437398 */
    daPyProc_ROPE_THROW_CATCH_e                  = 126, /* 02437530 */
    daPyProc_ROPE_UP_HANG_e                      = 127, /* 02437630 */
    daPyProc_BOOMERANG_SUBJECT_e                 = 128, /* 024376D8 */
    daPyProc_BOOMERANG_MOVE_e                    = 129, /* 02437810 */
    daPyProc_BOOMERANG_CATCH_e                   = 130, /* 02437900 */
    daPyProc_HOOKSHOT_SUBJECT_e                  = 131, /* 02437C0C */
    daPyProc_HOOKSHOT_MOVE_e                     = 132, /* 02437D60 */
    daPyProc_HOOKSHOT_FLY_e                      = 133, /* 02437F00 */
    daPyProc_SHIP_READY_e                        = 134, /* 0243213C */
    daPyProc_SHIP_JUMP_RIDE_e                    = 135, /* 024322EC */
    daPyProc_SHIP_STEER_e                        = 136, /* 02432334 */
    daPyProc_SHIP_PADDLE_e                       = 137, /* 02432378 */
    daPyProc_SHIP_SCOPE_e                        = 138, /* 024327A4 */
    daPyProc_SHIP_BOOMERANG_e                    = 139, /* 02432B28 */
    daPyProc_SHIP_HOOKSHOT_e                     = 140, /* 02433104 */
    daPyProc_SHIP_BOW_e                          = 141, /* 0243333C */
    daPyProc_SHIP_CANNON_e                       = 142, /* 024334EC */
    daPyProc_SHIP_CRANE_e                        = 143, /* 02433570 */
    daPyProc_SHIP_GET_OFF_e                      = 144, /* 02433BB8 */
    daPyProc_SHIP_RESTART_e                      = 145, /* 02433D88 */
    daPyProc_FAN_SWING_e                         = 146, /* 024386C4 */
    daPyProc_FAN_GLIDE_e                         = 147, /* 02438E78 */
    daPyProc_BOW_SUBJECT_e                       = 148, /* 024379C0 */
    daPyProc_BOW_MOVE_e                          = 149, /* 02437A8C */
    daPyProc_VOMIT_READY_e                       = 150, /* 0243B9C4 */
    daPyProc_VOMIT_WAIT_e                        = 151, /* 0243BA94 */
    daPyProc_VOMIT_JUMP_e                        = 152, /* 0243BC78 */
    daPyProc_VOMIT_LAND_e                        = 153, /* 0243BEE4 */
    daPyProc_TACT_WAIT_e                         = 154, /* 0243A094 */
    daPyProc_TACT_PLAY_e                         = 155, /* 0243AC64 */
    daPyProc_TACT_PLAY_END_e                     = 156, /* 0243B200 */
    daPyProc_TACT_PLAY_ORIGINAL_e                = 157, /* 0243B74C */
    daPyProc_ICE_SLIP_FALL_e                     = 158, /* 0241F1AC */
    daPyProc_ICE_SLIP_FALL_UP_e                  = 159, /* 0241F240 */
    daPyProc_ICE_SLIP_ALMOST_FALL_e              = 160, /* 0241F2D4 */
    daPyProc_BOOTS_EQUIP_e                       = 161, /* 0241F618 */
    daPyProc_NOT_USE_e                           = 162, /* 0241F79C */
    daPyProc_BOTTLE_DRINK_e                      = 163, /* 0243D838 */
    daPyProc_BOTTLE_OPEN_e                       = 164, /* 0243DBFC */
    daPyProc_BOTTLE_SWING_e                      = 165, /* 0243E634 */
    daPyProc_BOTTLE_GET_e                        = 166, /* 0243E948 */
    daPyProc_FOOD_THROW_e                        = 167, /* 0243F4DC */
    daPyProc_FOOD_SET_e                          = 168, /* 0243F670 */
    daPyProc_DEMO_TOOL_e                         = 169, /* 0241FD7C */
    daPyProc_DEMO_TALK_e                         = 170, /* 02420F08 */
    daPyProc_DEMO_DAMAGE_e                       = 171, /* 02421200 */
    daPyProc_DEMO_HOLDUP_e                       = 172, /* 024213E4 */
    daPyProc_DEMO_OPEN_TREASURE_e                = 173, /* 02421678 */
    daPyProc_DEMO_GET_ITEM_e                     = 174, /* 02421948 */
    daPyProc_DEMO_UNEQUIP_e                      = 175, /* 02421E38 */
    daPyProc_DEMO_LAVA_DAMAGE_e                  = 176, /* 02421E80 */
    daPyProc_DEMO_FREEZE_DAMAGE_e                = 177, /* 02421F1C */
    daPyProc_DEMO_DEAD_e                         = 178, /* 02422104 */
    daPyProc_DEMO_LOOK_AROUND_e                  = 179, /* 02422564 */
    daPyProc_DEMO_SALUTE_e                       = 180, /* 02422664 */
    daPyProc_DEMO_LOOK_AROUND2_e                 = 181, /* 02422790 */
    daPyProc_DEMO_TALISMAN_PICKUP_e              = 182, /* 024228A8 */
    daPyProc_DEMO_TALISMAN_WAIT_e                = 183, /* 02422BA8 */
    daPyProc_DEMO_SURPRISED_e                    = 184, /* 02422F14 */
    daPyProc_DEMO_TURN_BACK_e                    = 185, /* 02423124 */
    daPyProc_DEMO_LOOK_UP_e                      = 186, /* 02423230 */
    daPyProc_DEMO_QUAKE_WAIT_e                   = 187, /* 02423340 */
    daPyProc_DEMO_DANCE_e                        = 188, /* 0242342C */
    daPyProc_DEMO_CAUGHT_e                       = 189, /* 02423554 */
    daPyProc_DEMO_LOOK_WAIT_e                    = 190, /* 02423590 */
    daPyProc_DEMO_PUSH_PULL_WAIT_e               = 191, /* 02423740 */
    daPyProc_DEMO_PUSH_MOVE_e                    = 192, /* 02423854 */
    daPyProc_DEMO_DOOR_OPEN_e                    = 193, /* 024239D8 */
    daPyProc_DEMO_NOD_e                          = 194, /* 02423B6C */
    daPyProc_DEMO_PRESENT_e                      = 195, /* 02423D10 */
    daPyProc_DEMO_WIND_CHANGE_e                  = 196, /* 02424000 */
    daPyProc_DEMO_STAND_ITEM_PUT_e               = 197, /* 02424240 */
    daPyProc_DEMO_VORCANO_FAIL_e                 = 198, /* 024243F8 */
    daPyProc_DEMO_SLIGHT_SURPRISED_e             = 199, /* 024245AC */
    daPyProc_DEMO_SMILE_e                        = 200, /* 02424818 */
    daPyProc_DEMO_BOSS_WARP_e                    = 201, /* 024249DC */
    daPyProc_DEMO_AGB_USE_e                      = 202, /* 02424B58 */
    daPyProc_DEMO_LOOK_TURN_e                    = 203, /* 02424C3C */
    daPyProc_DEMO_LETTER_OPEN_e                  = 204, /* 02424DB8 */
    daPyProc_DEMO_LETTER_READ_e                  = 205, /* 02424F1C */
    daPyProc_DEMO_REDEAD_STOP_e                  = 206, /* 024250D0 */
    daPyProc_DEMO_REDEAD_CATCH_e                 = 207, /* 02425194 */
    daPyProc_DEMO_GET_DANCE_e                    = 208, /* 024252E4 */
    daPyProc_DEMO_BOTTLE_OPEN_FAIRY_e            = 209, /* 024255EC */
    daPyProc_DEMO_WARP_SHORT_e                   = 210, /* 0242583C */
    daPyProc_DEMO_OPEN_SALVAGE_TREASURE_e        = 211, /* 0242595C */
    daPyProc_DEMO_SURPRISED_WAIT_e               = 212, /* 02425B24 */
    daPyProc_DEMO_POWER_UP_WAIT_e                = 213, /* 02425B80 */
    daPyProc_DEMO_POWER_UP_e                     = 214, /* 02425D68 */
    daPyProc_DEMO_SHIP_SIT_e                     = 215, /* 02425FD8 */
    daPyProc_DEMO_LAST_COMBO_e                   = 216, /* 024261A0 */
    daPyProc_DEMO_HAND_UP_e                      = 217, /* 02426324 */
    daPyProc_DEMO_ICE_SLIP_e                     = 218, /* 024263EC */
    daPyProc_UNKW_00_e                           = 219, /* 024265A0  [V] HD addition */
    daPyProc_UNKW_01_e                           = 220, /* 02426964  [V] HD addition */
    daPyProc_UNKW_02_e                           = 221, /* 0242705C  [V] HD addition */
    daPyProc_MAX_e                               = 222
};


/** Short display names, indexed by procedure id. Same provenance as the enum:
 *  the id is [V], the name is [P]. */
static const char* const daPy_procNames[WWHD_DAPY_PROC_MAX] = {
    "SCOPE", "SUBJECTIVITY", "CALL",
    "CONTROLL_WAIT", "WAIT", "FREE_WAIT",
    "MOVE", "ATN_MOVE", "ATN_ACTOR_WAIT",
    "ATN_ACTOR_MOVE", "SIDE_STEP", "SIDE_STEP_LAND",
    "CROUCH_DEFENSE", "CROUCH_DEFENSE_SLIP", "CROUCH",
    "CRAWL_START", "CRAWL_MOVE", "CRAWL_AUTO_MOVE",
    "CRAWL_END", "WHIDE_READY", "WHIDE_WAIT",
    "WHIDE_MOVE", "WHIDE_PEEP", "WAIT_TURN",
    "MOVE_TURN", "SLIP", "SLIDE_FRONT",
    "SLIDE_BACK", "SLIDE_FRONT_LAND", "SLIDE_BACK_LAND",
    "FRONT_ROLL", "FRONT_ROLL_CRASH", "NOCK_BACK_END",
    "SIDE_ROLL", "BACK_JUMP", "BACK_JUMP_LAND",
    "AUTO_JUMP", "LAND", "LAND_DAMAGE",
    "FALL", "SLOW_FALL", "SMALL_JUMP",
    "VERTICAL_JUMP", "HANG_START", "HANG_FALL_START",
    "HANG_UP", "HANG_WAIT", "HANG_MOVE",
    "HANG_CLIMB", "HANG_WALL_CATCH", "PUSH_PULL_WAIT",
    "PUSH_MOVE", "PULL_MOVE", "SWIM_UP",
    "SWIM_WAIT", "SWIM_MOVE", "LADDER_UP_START",
    "LADDER_UP_END", "LADDER_DOWN_START", "LADDER_DOWN_END",
    "LADDER_MOVE", "CLIMB_UP_START", "CLIMB_DOWN_START",
    "CLIMB_MOVE_UP_DOWN", "CLIMB_MOVE_SIDE", "CUT_A",
    "CUT_F", "CUT_R", "CUT_L",
    "CUT_EA", "CUT_EB", "CUT_EX_A",
    "CUT_EX_B", "CUT_EX_MJ", "CUT_KESA",
    "WEAPON_NORMAL_SWING", "WEAPON_SIDE_SWING", "WEAPON_FRONT_SWING_READY",
    "WEAPON_FRONT_SWING", "WEAPON_FRONT_SWING_END", "WEAPON_THROW",
    "HAMMER_SIDE_SWING", "HAMMER_FRONT_SWING_READY", "HAMMER_FRONT_SWING",
    "HAMMER_FRONT_SWING_END", "CUT_TURN", "CUT_ROLL",
    "CUT_ROLL_END", "CUT_TURN_CHARGE", "CUT_TURN_MOVE",
    "CUT_REVERSE", "JUMP_CUT", "JUMP_CUT_LAND",
    "BT_JUMP", "BT_JUMP_CUT", "BT_SLIDE",
    "BT_ROLL", "BT_ROLL_CUT", "BT_VERTICAL_JUMP",
    "BT_VERTICAL_JUMP_CUT", "BT_VERTICAL_JUMP_LAND", "GUARD_CRASH",
    "DAMAGE", "POLY_DAMAGE", "LARGE_DAMAGE",
    "LARGE_DAMAGE_UP", "LARGE_DAMAGE_WALL", "LAVA_DAMAGE",
    "ELEC_DAMAGE", "GUARD_SLIP", "GRAB_READY",
    "GRAB_UP", "GRAB_MISS", "GRAB_THROW",
    "GRAB_PUT", "GRAB_WAIT", "GRAB_HEAVY_WAIT",
    "GRAB_REBOUND", "ROPE_SUBJECT", "ROPE_READY",
    "ROPE_SWING", "ROPE_HANG_WAIT", "ROPE_UP",
    "ROPE_DOWN", "ROPE_SWING_START", "ROPE_MOVE",
    "ROPE_THROW_CATCH", "ROPE_UP_HANG", "BOOMERANG_SUBJECT",
    "BOOMERANG_MOVE", "BOOMERANG_CATCH", "HOOKSHOT_SUBJECT",
    "HOOKSHOT_MOVE", "HOOKSHOT_FLY", "SHIP_READY",
    "SHIP_JUMP_RIDE", "SHIP_STEER", "SHIP_PADDLE",
    "SHIP_SCOPE", "SHIP_BOOMERANG", "SHIP_HOOKSHOT",
    "SHIP_BOW", "SHIP_CANNON", "SHIP_CRANE",
    "SHIP_GET_OFF", "SHIP_RESTART", "FAN_SWING",
    "FAN_GLIDE", "BOW_SUBJECT", "BOW_MOVE",
    "VOMIT_READY", "VOMIT_WAIT", "VOMIT_JUMP",
    "VOMIT_LAND", "TACT_WAIT", "TACT_PLAY",
    "TACT_PLAY_END", "TACT_PLAY_ORIGINAL", "ICE_SLIP_FALL",
    "ICE_SLIP_FALL_UP", "ICE_SLIP_ALMOST_FALL", "BOOTS_EQUIP",
    "NOT_USE", "BOTTLE_DRINK", "BOTTLE_OPEN",
    "BOTTLE_SWING", "BOTTLE_GET", "FOOD_THROW",
    "FOOD_SET", "DEMO_TOOL", "DEMO_TALK",
    "DEMO_DAMAGE", "DEMO_HOLDUP", "DEMO_OPEN_TREASURE",
    "DEMO_GET_ITEM", "DEMO_UNEQUIP", "DEMO_LAVA_DAMAGE",
    "DEMO_FREEZE_DAMAGE", "DEMO_DEAD", "DEMO_LOOK_AROUND",
    "DEMO_SALUTE", "DEMO_LOOK_AROUND2", "DEMO_TALISMAN_PICKUP",
    "DEMO_TALISMAN_WAIT", "DEMO_SURPRISED", "DEMO_TURN_BACK",
    "DEMO_LOOK_UP", "DEMO_QUAKE_WAIT", "DEMO_DANCE",
    "DEMO_CAUGHT", "DEMO_LOOK_WAIT", "DEMO_PUSH_PULL_WAIT",
    "DEMO_PUSH_MOVE", "DEMO_DOOR_OPEN", "DEMO_NOD",
    "DEMO_PRESENT", "DEMO_WIND_CHANGE", "DEMO_STAND_ITEM_PUT",
    "DEMO_VORCANO_FAIL", "DEMO_SLIGHT_SURPRISED", "DEMO_SMILE",
    "DEMO_BOSS_WARP", "DEMO_AGB_USE", "DEMO_LOOK_TURN",
    "DEMO_LETTER_OPEN", "DEMO_LETTER_READ", "DEMO_REDEAD_STOP",
    "DEMO_REDEAD_CATCH", "DEMO_GET_DANCE", "DEMO_BOTTLE_OPEN_FAIRY",
    "DEMO_WARP_SHORT", "DEMO_OPEN_SALVAGE_TREASURE", "DEMO_SURPRISED_WAIT",
    "DEMO_POWER_UP_WAIT", "DEMO_POWER_UP", "DEMO_SHIP_SIT",
    "DEMO_LAST_COMBO", "DEMO_HAND_UP", "DEMO_ICE_SLIP",
    "UNKW_00", "UNKW_01", "UNKW_02"
};

/** Name for a procedure id, or "?" when it is out of range. Never NULL, so a
 *  HUD can print it without a null check. */
static __inline const char* daPy_procName(s32 proc) {
    if (proc < 0 || proc >= WWHD_DAPY_PROC_MAX)
        return "?";
    return daPy_procNames[proc];
}

/** Procedure id for a name, or -1 if unknown. The inverse of daPy_procName;
 *  the comparison is exact and case-sensitive. Names are the short form, so
 *  "WAIT" rather than "daPyProc_WAIT_e". */
static __inline s32 daPy_procId(const char* name) {
    s32 i;
    const char* a;
    const char* b;
    if (!name)
        return -1;
    for (i = 0; i < WWHD_DAPY_PROC_MAX; i++) {
        a = daPy_procNames[i];
        b = name;
        while (*a && *a == *b) { a++; b++; }
        if (*a == 0 && *b == 0)
            return i;
    }
    return -1;
}

#endif /* LIBWWHD_D_A_PLAYER_PROC_H */
