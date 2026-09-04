#ifndef LIBWWHD_D_A_PLAYER_DEMO_H
#define LIBWWHD_D_A_PLAYER_DEMO_H

#include "libwwhd/wwhd_types.h"

/**
 * libwwhd - Link's demo state (daPy_demo_c, inside d_a_player_main.cpp)
 *
 * "Demo" here means a scripted player animation state, not a full cutscene -
 * opening a chest, reading a letter, boarding the boat. It is the state the
 * chest- and door-storage speedrun glitches manipulate, which is why it is
 * mapped in its own header rather than left inside the actor.
 *
 * WHERE IT LIVES: daPy_lk_c + 0x0420, so mDemoMode is at + 0x0430.
 *
 * Three independent confirmations of that offset:
 *   1. 37 distinct functions write the exact daPy_demo_c field-and-width
 *      pattern through +0x0420 - halfwords at +0x00/+0x02/+0x04, words at
 *      +0x08/+0x0C/+0x10 and a FLOAT at +0x14. The float is decisive: mStick
 *      is the only f32 in the struct. Every other candidate base in the binary
 *      had 1-3 writers; this one had 37.
 *   2. +0x0430 is compared against 23 distinct small constants across 29
 *      player functions, which is what mDemoMode == DEMO_*_e compiles to.
 *   3. GameCube has mDemo at daPy_lk_c + 0x304, and 0x304 + 0x11C = 0x420 -
 *      exactly the actor-prefix shift that dKy_tevstr_c's growth accounts for.
 *
 * ON THE NAMES: every name below is transcribed from the GameCube decomp and
 * is NOT behaviourally confirmed in WWHD. What IS confirmed per value is
 * whether HD's player code actually uses it - see the markers. WWHD renumbered
 * at least one process enum (see WWHD_PROC_SHIP), so treat a name as a strong
 * hypothesis rather than a reading, and confirm before relying on one.
 */

/** [V] daPy_demo_c. Field widths read out of the 37 writer functions. */
typedef struct daPy_demo_c {
    /* 0x00 */ u16 mDemoType;       /* [V] width; name [I] */
    /* 0x02 */ s16 mDemoMoveAngle;  /* [V] width; name [I] */
    /* 0x04 */ s16 mTimer;          /* [V] width; name [I] */
    /* 0x06 */ s16 mParam2;         /* [P] width [V]: 10 halfword stores
                                     *     at mDemo+0x06; name [I] */
    /* 0x08 */ s32 mParam0;         /* [V] width; name [I] */
    /* 0x0C */ s32 mParam1;         /* [V] width; name [I] */
    /* 0x10 */ u32 mDemoMode;       /* [V] the state itself */
    /* 0x14 */ f32 mStick;          /* [V] width; name [I]. The f32 here is
                                     *     what identified the struct. */
} daPy_demo_c;
WWHD_ASSERT_OFFSET(daPy_demo_c, mDemoMode, 0x10);
WWHD_ASSERT_OFFSET(daPy_demo_c, mStick,    0x14);
WWHD_ASSERT_SIZE  (daPy_demo_c,            0x18);

/** [V] Offset of mDemo inside daPy_lk_c. */
#define WWHD_DAPY_OFF_DEMO 0x0420

/**
 * Demo modes.
 *
 * [V] marks a value HD's player code demonstrably uses - it appears in an
 * equality test or a literal write at daPy_lk_c+0x430. [I] marks a value taken
 * from the GameCube enum that HD was not observed using; it is very likely
 * still correct, since the ranges agree exactly, but it was not seen.
 *
 * Re-checked with an independently written scan, which found a strict subset of
 * the values already marked [V] and contradicted none of them. It also turned
 * up one out-of-range value, 87, from a +0x430 access on an object that is not
 * the player - the contamination this technique has to be scoped against, and
 * the reason the [V] set is built from player code only.
 *
 * THE RANGE IS THE SAME AS GAMECUBE. The player code partitions this value at
 * 75 (`cmplwi mode, 0x4B` with bge/blt), handling 0..74 as ordinary modes and
 * anything above separately - and GameCube's enum is exactly 0..74. So HD added
 * no new animation modes. It does use one value outside the enum entirely; see
 * daPyDemo_UNKW_00.
 */
enum daPy_demoMode_e {
    daPyDemo_UNK00_e          = 0,   /* [V] also DEMO_NEW_ANM0_e on GameCube */
    daPyDemo_N_WAIT_e         = 1,   /* [V] */
    daPyDemo_N_WALK_e         = 2,   /* [V] */
    daPyDemo_N_DASH_e         = 3,   /* [V] */
    daPyDemo_INIT_WAIT_e      = 4,   /* [I] */
    daPyDemo_WAIT_TURN_e      = 5,   /* [V] */
    daPyDemo_N_TALK_e         = 6,   /* [V] */
    daPyDemo_DAM_ACTION_e     = 7,   /* [I] */
    daPyDemo_E_TALK_e         = 8,   /* [V] */
    daPyDemo_LDAM_e           = 9,   /* [V] */
    daPyDemo_OPEN_TREASURE_e  = 10,  /* [V] chest opening - the chest-storage
                                      *     state. See the note below. */
    daPyDemo_GET_ITEM_e       = 11,  /* [I] */
    daPyDemo_UNEQUIP_e        = 12,  /* [I] */
    daPyDemo_HOLDUP_e         = 13,  /* [I] */
    daPyDemo_KEEP_e           = 14,  /* [V] */
    daPyDemo_LAROUND_e        = 15,  /* [I] */
    daPyDemo_BJUMP_e          = 16,  /* [I] */
    daPyDemo_PFALL_e          = 17,  /* [V] */
    daPyDemo_UNK_018_e        = 18,  /* [V] */
    daPyDemo_SALUTE_e         = 19,  /* [I] */
    daPyDemo_L_AROUND2_e      = 20,  /* [I] */
    daPyDemo_T_PICKUP_e       = 21,  /* [I] */
    daPyDemo_T_WAIT_e         = 22,  /* [I] */
    daPyDemo_A_WAIT_e         = 23,  /* [V] */
    daPyDemo_SURPRISED_e      = 24,  /* [I] */
    daPyDemo_TBACK_e          = 25,  /* [I] */
    daPyDemo_LOOKUP_e         = 26,  /* [I] */
    daPyDemo_UNK_027_e        = 27,  /* [I] */
    daPyDemo_UNK_028_e        = 28,  /* [I] */
    daPyDemo_UNK_029_e        = 29,  /* [I] */
    daPyDemo_UNK_030_e        = 30,  /* [V] tested next to OPEN_TREASURE in the
                                      *     same condition, as on GameCube */
    daPyDemo_LWAIT_e          = 31,  /* [I] */
    daPyDemo_PWAIT_032_e      = 32,  /* [I] */
    daPyDemo_PUSH_e           = 33,  /* [I] */
    daPyDemo_TACT_e           = 34,  /* [I] */
    daPyDemo_DOOR_e           = 35,  /* [I] door transition - the door-storage
                                      *     state. NOT observed in HD; see the
                                      *     note below before relying on it. */
    daPyDemo_NOD_e            = 36,  /* [I] */
    daPyDemo_PRESENT_e        = 37,  /* [V] */
    daPyDemo_WCHANGE_e        = 38,  /* [I] */
    daPyDemo_SHIP_e           = 39,  /* [I] */
    daPyDemo_SIPUT_e          = 40,  /* [I] */
    daPyDemo_UNK_041_e        = 41,  /* [I] */
    daPyDemo_KM_WAIT_e        = 42,  /* [V] */
    daPyDemo_CUT_ROLL_e       = 43,  /* [V] */
    daPyDemo_POS_INIT_e       = 44,  /* [I] */
    daPyDemo_UNK_045_e        = 45,  /* [I] */
    daPyDemo_PGET_e           = 46,  /* [I] */
    daPyDemo_VOLCANO_e        = 47,  /* [I] */
    daPyDemo_WARP_e           = 48,  /* [I] */
    daPyDemo_S_SURP_e         = 49,  /* [I] */
    daPyDemo_SMILE_e          = 50,  /* [I] */
    daPyDemo_UNK_051_e        = 51,  /* [I] */
    daPyDemo_AGB_e            = 52,  /* [V] written as a literal */
    daPyDemo_LTURN_e          = 53,  /* [I] */
    daPyDemo_LET_OPEN_e       = 54,  /* [I] */
    daPyDemo_LET_READ_e       = 55,  /* [I] */
    daPyDemo_GRAB_PUT_e       = 56,  /* [V] */
    daPyDemo_RD_STOP_e        = 57,  /* [V] */
    daPyDemo_RD_CATCH_e       = 58,  /* [I] */
    daPyDemo_GET_DANCE_e      = 59,  /* [I] */
    daPyDemo_BO_FAIRY_e       = 60,  /* [I] */
    daPyDemo_BO_OPEN_e        = 61,  /* [V] */
    daPyDemo_S_WARP_e         = 62,  /* [I] */
    daPyDemo_OPEN_SBOX_e      = 63,  /* [I] salvage chest */
    daPyDemo_SFALL_e          = 64,  /* [V] */
    daPyDemo_FOODSET_e        = 65,  /* [I] */
    daPyDemo_SWAIT_e          = 66,  /* [I] */
    daPyDemo_PWAIT_067_e      = 67,  /* [I] */
    daPyDemo_BOW_MINIGAME_e   = 68,  /* [V] */
    daPyDemo_SHIPSIT_e        = 69,  /* [I] */
    daPyDemo_UNK_070_e        = 70,  /* [I] */
    daPyDemo_SHIPOFF_e        = 71,  /* [I] */
    daPyDemo_UNK_072_e        = 72,  /* [I] */
    daPyDemo_FOODTHROW_e      = 73,  /* [I] */
    daPyDemo_UNK_074_e        = 74,  /* [I] */

    /** [V] The partition, not a mode: the player code tests
     *  `cmplwi mode, 0x4B` and treats anything at or above it separately.
     *  Equal to GameCube's enum size, which is how we know HD added no modes. */
    daPyDemo_MODE_MAX_e       = 75,

    /** [V] The one value HD uses that is NOT in the GameCube enum. Tested for
     *  equality, above the 75 partition, so it is a sentinel rather than an
     *  animation. Meaning unknown - hence the name. */
    daPyDemo_UNKW_00_e        = 0x200
};

/** [V] Non-zero when `mode` is an ordinary animation mode rather than a
 *  sentinel. Mirrors the partition the player code itself applies. */
static __inline int daPy_isNormalDemoMode(u32 mode) {
    return mode < (u32)daPyDemo_MODE_MAX_e;
}


/* --- Lookups ------------------------------------------------------------
 * Same provenance as the enum: the VALUE is [V] where HD was seen using it,
 * the NAME is transcribed from GameCube throughout.
 * ---------------------------------------------------------------------- */

/** Short display names indexed by mode. Covers the ordinary 0..74 range;
 *  sentinels such as daPyDemo_UNKW_00_e are outside it by design. */
static const char* const daPy_demoModeNames[daPyDemo_MODE_MAX_e] = {
    "UNK00", "N_WAIT", "N_WALK", "N_DASH",
    "INIT_WAIT", "WAIT_TURN", "N_TALK", "DAM_ACTION",
    "E_TALK", "LDAM", "OPEN_TREASURE", "GET_ITEM",
    "UNEQUIP", "HOLDUP", "KEEP", "LAROUND",
    "BJUMP", "PFALL", "UNK_018", "SALUTE",
    "L_AROUND2", "T_PICKUP", "T_WAIT", "A_WAIT",
    "SURPRISED", "TBACK", "LOOKUP", "UNK_027",
    "UNK_028", "UNK_029", "UNK_030", "LWAIT",
    "PWAIT_032", "PUSH", "TACT", "DOOR",
    "NOD", "PRESENT", "WCHANGE", "SHIP",
    "SIPUT", "UNK_041", "KM_WAIT", "CUT_ROLL",
    "POS_INIT", "UNK_045", "PGET", "VOLCANO",
    "WARP", "S_SURP", "SMILE", "UNK_051",
    "AGB", "LTURN", "LET_OPEN", "LET_READ",
    "GRAB_PUT", "RD_STOP", "RD_CATCH", "GET_DANCE",
    "BO_FAIRY", "BO_OPEN", "S_WARP", "OPEN_SBOX",
    "SFALL", "FOODSET", "SWAIT", "PWAIT_067",
    "BOW_MINIGAME", "SHIPSIT", "UNK_070", "SHIPOFF",
    "UNK_072", "FOODTHROW", "UNK_074"
};

/** Name for a demo mode. Never NULL, so a HUD can print it unchecked.
 *  Returns "SENTINEL" for a value above the partition and "?" otherwise. */
static __inline const char* daPy_demoModeName(u32 mode) {
    if (mode == (u32)daPyDemo_UNKW_00_e)
        return "UNKW_00";
    if (!daPy_isNormalDemoMode(mode))
        return "SENTINEL";
    return daPy_demoModeNames[mode];
}

/** Mode id for a name, or -1 if unknown. The inverse of daPy_demoModeName;
 *  the comparison is exact and case-sensitive. */
static __inline s32 daPy_demoModeId(const char* name) {
    s32 i;
    const char* a;
    const char* b;
    if (!name)
        return -1;
    for (i = 0; i < (s32)daPyDemo_MODE_MAX_e; i++) {
        a = daPy_demoModeNames[i];
        b = name;
        while (*a && *a == *b) { a++; b++; }
        if (*a == 0 && *b == 0)
            return i;
    }
    return -1;
}

#endif /* LIBWWHD_D_A_PLAYER_DEMO_H */
