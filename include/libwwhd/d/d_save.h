#ifndef LIBWWHD_D_SAVE_H
#define LIBWWHD_D_SAVE_H

#include "libwwhd/wwhd_types.h"

/**
 * libwwhd - save data (d_save.cpp)
 *
 * The strongest-evidence area in libwwhd: 71 anchored functions naming their
 * own source file, line and bounds expression, plus a full sweep of every
 * offset WWHD reaches through the game info root. Confidence markers here are
 * the result of that sweep: [?] means the sweep found nothing touching the
 * field, not that the field is padding. Every save structure checked is
 * byte-identical to the GameCube layout, in all three regions. The divergence
 * in this subsystem is in where the object lives (d_com_inf_game.h), not in its
 * shape. docs/SUBSYSTEMS.md (Save) records the derivation.
 */

/* Bounds constants - all [V], read from the WWHD assert compares. */
#define dSv_STAGE_MAX            0x10  /* [V] cmp >0x0F  d_save.cpp:0xD77 */
#define dSv_ZONE_MAX             0x20  /* [V] cmp >0x1F  d_save.cpp:0xDD2 */
#define dSv_ROOM_MAX             64    /* [V] cmp >0x3F  d_save.cpp:0xDD0 */

#define dSv_MEMORY_SWITCH        0x80  /* [V] branch <0x80  d_save.cpp:0xDBF */
#define dSv_DAN_SWITCH           0x40  /* [V] branch <0xC0  d_save.cpp:0xDBF */
#define dSv_ZONE_SWITCH          0x30  /* [V] total cmp >0xEF = 0x80+0x40+0x30 */

#define dSv_MEMORY_ITEM          0x40  /* [V] branch <0x40  d_save.cpp:0xE5B */
#define dSv_ZONE_ITEM            0x10  /* [V] total cmp >0x4F = 0x40+0x10 */

#define dSv_memBit_ITEM_MAX      0x20  /* [V] cmp >0x1F  d_save.cpp:0x98A */
#define dSv_zoneBit_SWITCH_MAX   0x30  /* [V] cmp >0x2F  d_save.cpp:0xBA3 (48) */
#define dSv_zoneActor_ACTOR_MAX  512   /* [V] the assert text hides the
                                        *     number, but the compare is
                                        *     >0x1FF  d_save.cpp:0xC29     */
#define dSv_OCEAN_GRID_MAX       0x32  /* [V] cmp >0x31  d_save.cpp:0xA96 */
#define dSv_OCEAN_BIT_MAX        16    /* [V] cmp >0x0F  d_save.cpp:0xA97 */

/* Sea chart grid. The 7x7 board is 49 squares; d_map.h owns the geometry. */
#define dSv_FMAP_GRID_MAX        49    /* [V] cmp >0x30  d_save.cpp:0x7FB */
#define dSv_CHART_MAX            61    /* [V] the item table carries exactly 61
                                        *     chart give-stubs, and the chart
                                        *     list UI bounds its index at 61 */

/* [V] Switch ids the router silently ignores (d_save.cpp:0xDBF). */
#define dSv_SWITCH_NONE_A        (-1)
#define dSv_SWITCH_NONE_B        255
/* [V] Item ids the router silently ignores (d_save.cpp:0xE5B). */
#define dSv_ITEM_NONE_A          (-1)
#define dSv_ITEM_NONE_B          127

/* Player status, inventory, bag and collectables.
 *
 * Confirmed at four independent points by the acquisition gate, which
 * addresses these off the heap gameInfo pointer: save+0x004 mRupee,
 * save+0x03C mPlayerItem, save+0x04A the bottle slots, save+0x076 mBagItem.
 * All four land on their GameCube offsets, so dSv_player_c is treated as
 * GC-identical. Every other member has since been swept for directly, and
 * carries whatever the sweep found. */

/* Access WIDTH confirms this prefix. A register-tracked scan of every use of
 * the heap gameInfo pointer shows halfword access at save+0x00/0x02/0x04 and
 * byte access from 0x09 on, with nothing ever touching 0x01/0x03/0x05 - which
 * is what these types predict and what a different layout would break. */
typedef struct dSv_player_status_a_c {
    /* 0x00 */ u16 mMaxLife;         /* [V] offset+width; quarter-hearts.
                                      *     24 loads / 4 stores */
    /* 0x02 */ u16 mLife;            /* [V] offset+width; quarter-hearts.
                                      *     42 loads / 7 stores - the
                                      *     current-vs-max asymmetry is what
                                      *     tells these two apart, together
                                      *     with the GameCube order. No clamp
                                      *     site was found to prove it. */
    /* 0x04 */ u16 mRupee;           /* [V] */
    /* 0x06 */ u16 _unk_06;          /* [?] never touched */
    /* 0x08 */ u8  _unk_08;          /* [?] never touched */
    /* 0x09 */ u8  mSelectItem[5];   /* [P] width [V]; X/Y/Z assignments */
    /* 0x0E */ u8  mSelectEquip[4];  /* [V] 0 sword id, 1 shield id, 2 the
                                      *     bracelets id; 0xFF for none. The
                                      *     eight equipment give stubs route
                                      *     through the equip setter at
                                      *     0x02522398, which stores the id
                                      *     at (this + 0x0E + category).
                                      *     [3] is [?]. See d_item.h.      */
    /* 0x12 */ u8  mWalletSize;      /* [V] selects the rupee cap; see
                                      *     dSv_walletCapacity() below */
    /* 0x13 */ u8  mMaxMagic;        /* [V] capped at 32 by the meter */
    /* 0x14 */ u8  mMagic;           /* [V] floored at 0, clamped to
                                      *     mMaxMagic by the meter */
    /* 0x15 */ u8  _unk_15;          /* [?] never touched */
    /* 0x16 */ u8  _unk_16;          /* [?] never touched */
    /* 0x17 */ u8  _pad_17;          /* [?] never touched */
} dSv_player_status_a_c;
WWHD_ASSERT_OFFSET(dSv_player_status_a_c, mRupee,      0x04);
WWHD_ASSERT_OFFSET(dSv_player_status_a_c, mWalletSize, 0x12);
WWHD_ASSERT_OFFSET(dSv_player_status_a_c, mMaxMagic,   0x13);
WWHD_ASSERT_OFFSET(dSv_player_status_a_c, mMagic,      0x14);
WWHD_ASSERT_SIZE  (dSv_player_status_a_c, 0x18);

/**
 * [V] Rupee capacity per wallet size, read out of the rupee meter's own clamp
 * at 0x0259C454, which switches on mWalletSize and writes the ceiling straight
 * back into mRupee:
 *
 *     walletSize == 0 -> 500      walletSize == 1 -> 1000      else -> 5000
 *
 * THE GAMECUBE VALUE FOR THE STARTING WALLET IS 200. WWHD raised it to 500, so
 * this is one more place the decomp would have been wrong. Corroborated by
 * 0x025931CC, which re-lays out the rupee digits when mWalletSize changes -
 * three digits for the 500 wallet, four for the others.
 *
 * The numbers are gameplay constants compiled into the routine, read from the
 * USA build; EUR and JAP were not re-read.
 */
static __inline u16 dSv_walletCapacity(u8 walletSize) {
    if (walletSize == 0) return (u16)500;
    if (walletSize == 1) return (u16)1000;
    return (u16)5000;
}

/** [V] Ceiling the magic meter enforces on mMaxMagic (0x0259B698). */
#define dSv_MAGIC_MAX 32

typedef struct dSv_player_status_b_c {
    /* 0x00 */ u64 mDateIPL;         /* [P] 8 bytes live, read as two words */
    /* 0x08 */ f32 _unk_08;          /* [?] never touched */
    /* 0x0C */ f32 mTime;            /* [P] 4 bytes, 29 reads / 19 writes */
    /* 0x10 */ u16 mDate;            /* [P] halfword, 7 fns */
    /* 0x12 */ s16 mTactWindAngleX;  /* [P] halfword, 5 fns */
    /* 0x14 */ s16 mTactWindAngleY;  /* [P] halfword, 5 fns */
    /* 0x16 */ u8  _pad_16[2];       /* [?] never touched */
} dSv_player_status_b_c;
WWHD_ASSERT_SIZE(dSv_player_status_b_c, 0x18);

/* [V] Where a loaded file resumes. The setter asserts the name (d_save.cpp:191)
 * and dComIfGs_gameStart (0x02522198) queues the warp from it: the name, the
 * point from +0x09 and the room from +0x08, layer -1, wipe 0. */
typedef struct dSv_player_return_place_c {
    /* 0x00 */ char mName[8];        /* [V] the setter asserts
                                      *     strlen(name) <= 7  d_save.cpp:191 */
    /* 0x08 */ s8   mRoomNo;         /* [V] the room gameStart warps to */
    /* 0x09 */ u8   mPoint;          /* [V] the spawn point it passes */
    /* 0x0A */ u8   _unk_0A;         /* [?] never touched */
    /* 0x0B */ u8   _unk_0B;         /* [?] never touched */
} dSv_player_return_place_c;
WWHD_ASSERT_SIZE(dSv_player_return_place_c, 0x0C);

/** [V] Inventory slots. 0xFF means empty. +0x0E..+0x11 are the bottles. */
typedef struct dSv_player_item_c {
    /* 0x00 */ u8 mItems[21];        /* [V] */
} dSv_player_item_c;
WWHD_ASSERT_SIZE(dSv_player_item_c, 0x15);

typedef struct dSv_player_get_item_c {
    /* 0x00 */ u8 mItemFlags[21];    /* [V] one byte per inventory slot; bit
                                      *     n is tier n of that slot's item
                                      *     (d_item.h), set by onItem at
                                      *     0x025B5CCC. Byte 0 bits 1..7 and
                                      *     bytes 1..2 double as the bottle-
                                      *     contents flags (0x025B5DBC).
                                      *     Length 21 is [P]                */
} dSv_player_get_item_c;
WWHD_ASSERT_SIZE(dSv_player_get_item_c, 0x15);

typedef struct dSv_player_item_record_c {
    /* 0x0 */ u16 mTimer;            /* [V] resetTimer writes a halfword here */
    /* 0x2 */ u8  mPictureNum;       /* [I] never touched */
    /* 0x3 */ u8  mArrowNum;         /* [V] arrows carried */
    /* 0x4 */ u8  mBombNum;          /* [V] bombs carried */
    /* 0x5 */ u8  mBottleNum[3];     /* [I] never touched */
} dSv_player_item_record_c;
WWHD_ASSERT_OFFSET(dSv_player_item_record_c, mArrowNum, 0x3);
WWHD_ASSERT_OFFSET(dSv_player_item_record_c, mBombNum,  0x4);
WWHD_ASSERT_SIZE  (dSv_player_item_record_c, 0x8);

typedef struct dSv_player_item_max_c {
    /* 0x0 */ u8 mReserved1Num;      /* [I] never touched */
    /* 0x1 */ u8 mArrowNum;          /* [V] quiver capacity */
    /* 0x2 */ u8 mBombNum;           /* [V] bomb bag capacity */
    /* 0x3 */ u8 _unk_03[5];         /* [?] never touched */
} dSv_player_item_max_c;
WWHD_ASSERT_OFFSET(dSv_player_item_max_c, mArrowNum, 0x1);
WWHD_ASSERT_OFFSET(dSv_player_item_max_c, mBombNum,  0x2);
WWHD_ASSERT_SIZE  (dSv_player_item_max_c, 0x8);

/** [V] offset (save+0x076). setBeastItem scans 8 bytes from +0x00 for a 0xFF
 *  slot, which is what verifies the first array; the other two follow it. */
typedef struct dSv_player_bag_item_c {
    /* 0x00 */ u8 mBeast[8];         /* [V] 8-slot scan for 0xFF */
    /* 0x08 */ u8 mBait[8];          /* [V] setBaitItem at 0x025B6E98 scans
                                      *     it for 0xFF and seeds the count  */
    /* 0x10 */ u8 mReserve[8];       /* [V] setReserveItem at 0x025B75AC,
                                      *     the same scan                    */
} dSv_player_bag_item_c;
WWHD_ASSERT_SIZE(dSv_player_bag_item_c, 0x18);

typedef struct dSv_player_get_bag_item_c {
    /* 0x0 */ u32 mReserveFlags;     /* [V] onReserve: *this |= 1<<no, no<32 */
    /* 0x4 */ u8  mBeastFlags;       /* [V] onBeast:  (this+4), no<8 */
    /* 0x5 */ u8  mBaitFlags;        /* [V] onBait:   (this+5), no<8 */
    /* 0x6 */ u8  _unk_06[0xC - 0x6];/* [?] never touched */
} dSv_player_get_bag_item_c;
WWHD_ASSERT_SIZE(dSv_player_get_bag_item_c, 0x0C);

typedef struct dSv_player_bag_item_record_c {
    /* 0x00 */ u8 mBeastNum[8];      /* [V] all 8 bytes read individually */
    /* 0x08 */ u8 mBaitNum[8];       /* [V] the bait consumer indexes
                                      *     (this + 0x08 + itemNo - 0x24),
                                      *     itemNo - 0x24 bounded to < 8    */
    /* 0x10 */ u8 mReserveNum[8];    /* [P] */
} dSv_player_bag_item_record_c;
WWHD_ASSERT_SIZE(dSv_player_bag_item_record_c, 0x18);

/**
 * Trophies and quest collectables. Every named byte was read out of WWHD from
 * the accessor that owns it. Those accessors take the struct directly, so the
 * offset IS the store displacement and nothing here is inferred.
 *
 *   mCollect  on/off/isCollect(idx, bit) -> (this + idx)
 *   mTact     +0x09    mTriforce +0x0A    mSymbol +0x0B
 */
typedef struct dSv_player_collect_c {
    /* 0x0 */ u8 mCollect[8];        /* [V] 0..4: sword, shield, bracelets,
                                      *     Pirate's Charm, Hero's Charm bits,
                                      *     written by the give stubs through
                                      *     onCollect (d_item.h). 5..7 [?];
                                      *     the length 8 is GC's           */
    /* 0x8 */ u8 _unk_08;            /* [?] */
    /* 0x9 */ u8 mTact;              /* [V] */
    /* 0xA */ u8 mTriforce;          /* [V] Triforce SHARDS held, 8 bits. Not
                                      *     the charts - those live in
                                      *     dSv_player_map_c::mTriforceMap  */
    /* 0xB */ u8 mSymbol;            /* [V] */
    /* 0xC */ u8 _unk_0C;            /* [?] */
} dSv_player_collect_c;
WWHD_ASSERT_OFFSET(dSv_player_collect_c, mTact,     0x9);
WWHD_ASSERT_OFFSET(dSv_player_collect_c, mTriforce, 0xA);
WWHD_ASSERT_OFFSET(dSv_player_collect_c, mSymbol,   0xB);
WWHD_ASSERT_SIZE  (dSv_player_collect_c, 0x0D);

/**
 * Charts and the sea chart grid. Two unrelated things share this struct.
 *
 * CHARTS are three parallel 128-bit banks over a 1-based chart id, and the
 * three answer three different questions. Each bank offset was read from its
 * own accessor pair in WWHD:
 *
 *   mGetMap      +0x10   you own the chart
 *   mOpenMap     +0x20   the chart has been opened / deciphered
 *   mCompleteMap +0x30   the treasure it marks has been salvaged
 *
 * They are independent, not a ladder. Picking a chart up sets Get and CLEARS
 * the other two, so a re-obtained chart is salvageable again.
 *
 * The SEA GRID is one byte per square in mFmapBits; d_map.h owns the geometry.
 * Only bits 0 and 1 are ever written, and WWHD reads only bit 0 - bit 1 was
 * the Tingle Tuner copy, and its reader is gone from this build.
 *
 * docs/SUBSYSTEMS.md (Charts and the sea chart) has the derivation.
 */
typedef struct dSv_player_map_c {
    /* 0x00 */ u32 _unk_00[4];        /* [?] a fourth 128-bit bank. Zeroed by
                                       *     init and touched by nothing else
                                       *     in the whole binary.            */
    /* 0x10 */ u32 mGetMap[4];        /* [V] chart owned */
    /* 0x20 */ u32 mOpenMap[4];       /* [V] chart opened */
    /* 0x30 */ u32 mCompleteMap[4];   /* [V] treasure salvaged */
    /* 0x40 */ u8  mFmapBits[dSv_FMAP_GRID_MAX];  /* [V] one byte per square */
    /* 0x71 */ u8  _unk_71[16];       /* [?] zeroed by init, never read */
    /* 0x81 */ u8  mTriforceMap;      /* [V] Triforce Chart DECIPHERED, 8 bits.
                                       *     WWHD presets it - see
                                       *     WWHD_TRIFORCE_MAP_PRESET below. */
    /* 0x82 */ u8  _unk_82[2];        /* [?] */
} dSv_player_map_c;
WWHD_ASSERT_OFFSET(dSv_player_map_c, mGetMap,      0x10);
WWHD_ASSERT_OFFSET(dSv_player_map_c, mOpenMap,     0x20);
WWHD_ASSERT_OFFSET(dSv_player_map_c, mCompleteMap, 0x30);
WWHD_ASSERT_OFFSET(dSv_player_map_c, mFmapBits,    0x40);
WWHD_ASSERT_OFFSET(dSv_player_map_c, mTriforceMap, 0x81);
WWHD_ASSERT_SIZE  (dSv_player_map_c, 0x84);

/**
 * [V] Salvage bits, one u16 per ocean grid. Separate from the sea chart: this
 * is where a salvaged spot is remembered, written by the salvage actor for
 * every treasure whose reward is not a chart.
 */
typedef struct dSv_ocean_c {
    /* 0x00 */ u16 mSvBits[dSv_OCEAN_GRID_MAX];  /* [V] grid*2, 16 bits each */
} dSv_ocean_c;
WWHD_ASSERT_SIZE(dSv_ocean_c, 0x64);

/**
 * [V] The event flag bank - 256 bytes addressed by a PACKED id, not an index.
 *
 * The high byte of an id selects the byte and the low byte is the mask, so
 * 0x3E02 means "byte 0x3E, bit 1". A "register" is the same encoding used as a
 * bitfield: the mask names the field and the value is already shifted into
 * place, which is why getEventReg returns a masked value rather than a small
 * integer. Minigame results are stored this way.
 */
typedef struct dSv_event_c {
    /* 0x000 */ u8 mFlags[0x100];    /* [V] */
} dSv_event_c;
WWHD_ASSERT_SIZE(dSv_event_c, 0x100);

typedef struct dSv_player_c {
    /* 0x000 */ dSv_player_status_a_c        mPlayerStatusA;  /* [V] */
    /* 0x018 */ dSv_player_status_b_c        mPlayerStatusB;  /* [P] */
    /* 0x030 */ dSv_player_return_place_c    mReturnPlace;    /* [P] */
    /* 0x03C */ dSv_player_item_c            mPlayerItem;     /* [V] */
    /* 0x051 */ dSv_player_get_item_c        mGetItem;        /* [V] */
    /* 0x066 */ dSv_player_item_record_c     mItemRecord;     /* [V] */
    /* 0x06E */ dSv_player_item_max_c        mItemMax;        /* [P] */
    /* 0x076 */ dSv_player_bag_item_c        mBagItem;        /* [V] */
    /* 0x08E */ u8                           _pad_08E[2];     /* [?] */
    /* 0x090 */ dSv_player_get_bag_item_c    mGetBagItem;     /* [V] */
    /* 0x09C */ dSv_player_bag_item_record_c mBagItemRecord;  /* [V] */
    /* 0x0B4 */ dSv_player_collect_c         mCollect;        /* [V] */
    /* 0x0C1 */ u8                           _pad_0C1[3];     /* [?] */
    /* 0x0C4 */ dSv_player_map_c             mMap;            /* [V] */
    /* 0x148 */ u8                           mInfo[0x1A4 - 0x148];   /* [P] live;
                                     *  +0x12 death count, +0x58 clear count,
                                     *  +0x59 random salvage point           */
    /* 0x1A4 */ u8                           mConfig[0x1A9 - 0x1A4]; /* [P] */
    /* 0x1A9 */ u8                           _pad_1A9[3];            /* [?] */
    /* 0x1AC */ u8                           mPriest[0x1BC - 0x1AC]; /* [V] the
                                     *  setter writes cXyz +0x00, u16 +0x0C,
                                     *  u8 +0x0E and u8 +0x0F - 0x10 bytes,
                                     *  and a direct scan finds exactly those */
    /* 0x1BC */ u8                           mStatusC[0x37C - 0x1BC];/* [P] an
                                     *  ARRAY: the only offsets touched inside
                                     *  it are 0x1D4, 0x244, 0x2B4 and 0x324 -
                                     *  four entries of stride 0x70, which is
                                     *  exactly 0x37C - 0x1BC                */
    /* 0x37C */ u8                           _pad_37C[4];            /* [?] */
} dSv_player_c;
WWHD_ASSERT_OFFSET(dSv_player_c, mPlayerStatusA, 0x000);
WWHD_ASSERT_OFFSET(dSv_player_c, mPlayerItem,    0x03C);
WWHD_ASSERT_OFFSET(dSv_player_c, mBagItem,       0x076);
WWHD_ASSERT_OFFSET(dSv_player_c, mCollect,       0x0B4);
WWHD_ASSERT_SIZE  (dSv_player_c, 0x380);

/* Per-stage memory bits. [V] and identical to GameCube:
 *   mSwitch @0x04  d_save.cpp:0x94D writes (0x04 + (no>>5)*4), no < 128
 *   mItem   @0x14  d_save.cpp:0x98A writes (0x14 + (no>>5)*4), no < 32
 *   size    0x24   array stride in the gameInfo constructor */
typedef struct dSv_memBit_c {
    /* 0x00 */ u32 mTbox;            /* [V] onTbox: *this |= 1<<no, no<32 */
    /* 0x04 */ u32 mSwitch[4];       /* [V] 128 switch bits */
    /* 0x14 */ u32 mItem[1];         /* [V] 32 item bits */
    /* 0x18 */ u32 mVisitedRoom[2];  /* [V] (0x18 + (no>>5)*4), no<64 */
    /* 0x20 */ u8  mKeyNum;          /* [P] the only byte between two verified
                                      *     neighbours; never touched itself */
    /* 0x21 */ u8  mDungeonItem;     /* [V] (this+0x21) |= 1<<no, no<6.
                                      *     bit 0 map, 1 compass, 2 big key,
                                      *     from the 0x4C..0x4E give stubs  */
    /* 0x22 */ u8  _pad_22[2];       /* [?] */
} dSv_memBit_c;
WWHD_ASSERT_OFFSET(dSv_memBit_c, mSwitch, 0x04);
WWHD_ASSERT_OFFSET(dSv_memBit_c, mItem,   0x14);
WWHD_ASSERT_SIZE  (dSv_memBit_c, 0x24);

/** [V] A bare wrapper over dSv_memBit_c. */
typedef struct dSv_memory_c {
    /* 0x00 */ dSv_memBit_c mMembit;
} dSv_memory_c;
WWHD_ASSERT_SIZE(dSv_memory_c, 0x24);

/**
 * Scratch switch bank for the current dungeon.
 * [V] mSwitch @0x04, 64 bits: writes (0x04 + (no>>5)*4), no < 64.
 * [V] Sits at dSv_info_c +0x79C with mZone following at +0x7A8, so 0x0C.
 */
typedef struct dSv_danBit_c {
    /* 0x00 */ s8  mStageNo;         /* [V] init compares the incoming stage
                                      *     against it, then stores it      */
    /* 0x01 */ u8  mGbaRupeeCount;   /* [P] cleared by init alongside the
                                      *     switch words; name is GC's      */
    /* 0x02 */ u8  _pad_02[2];       /* [?] */
    /* 0x04 */ u32 mSwitch[2];       /* [V] 64 switch bits */
} dSv_danBit_c;
WWHD_ASSERT_OFFSET(dSv_danBit_c, mSwitch, 0x04);
WWHD_ASSERT_SIZE  (dSv_danBit_c, 0x0C);

/**
 * [V] mSwitch @0x00 as u16[3]: writes (base + (no>>4)*2), no < 48
 * [V] mItem   @0x06 as u16   : writes (base + 6),         no < 16
 */
typedef struct dSv_zoneBit_c {
    /* 0x00 */ u16 mSwitch[3];       /* [V] 48 switch bits */
    /* 0x06 */ u16 mItem;            /* [V] 16 item bits */
} dSv_zoneBit_c;
WWHD_ASSERT_OFFSET(dSv_zoneBit_c, mItem, 0x06);
WWHD_ASSERT_SIZE  (dSv_zoneBit_c, 0x08);

typedef struct dSv_zoneActor_c {
    /* 0x00 */ u32 mActorFlags[16];  /* [V] (this + (id>>5)*4), id<512 */
} dSv_zoneActor_c;
WWHD_ASSERT_SIZE(dSv_zoneActor_c, 0x40);

/**
 * [V] stride 0x4C (gameInfo ctor: 0x20 elements x 0x4C).
 * [V] mZoneBit at +0x02 - both the switch and item routers are called on
 *     (mZone_base + zoneId*0x4C + 0x02).
 */
typedef struct dSv_zone_c {
    /* 0x00 */ s8              mRoomNo;      /* [V] the constructor stores
                                                *     0xFF here last        */
    /* 0x01 */ u8              _pad_01;      /* [?] */
    /* 0x02 */ dSv_zoneBit_c   mZoneBit;     /* [V] */
    /* 0x0A */ u8              _pad_0A[2];   /* [?] */
    /* 0x0C */ dSv_zoneActor_c mZoneActor;   /* [V] the constructor clears
                                                *     0x40 bytes from +0x0C */
} dSv_zone_c;
WWHD_ASSERT_OFFSET(dSv_zone_c, mZoneBit, 0x02);
WWHD_ASSERT_SIZE  (dSv_zone_c, 0x4C);

/* dSv_save_c - the serialisable save block.
 *   [V] mMemory[0x10] @0x380 stride 0x24; getSave/putSave copy 0x24 bytes
 *       between save+0x380+stage*0x24 and dSv_info_c::mMemory.
 *   [V] total 0x778 - dSv_info_c::mMemory begins there. */
typedef struct dSv_save_c {
    /* 0x000 */ dSv_player_c mPlayer;                   /* [V] */
    /* 0x380 */ dSv_memory_c mMemory[dSv_STAGE_MAX];    /* [V] */
    /* 0x5C0 */ dSv_ocean_c  mOcean;                     /* [V] */
    /* 0x624 */ dSv_event_c  mEvent;                     /* [V] */
    /* 0x724 */ u8           mReserve[0x774 - 0x724];   /* [I] dSv_reserve_c;
                                                            *     never touched */
    /* 0x774 */ u8           _pad_774[4];               /* [?] never touched */
} dSv_save_c;
WWHD_ASSERT_OFFSET(dSv_save_c, mMemory, 0x380);
WWHD_ASSERT_SIZE  (dSv_save_c, 0x778);

/* dSv_restart_c - where the game resumes, and the scene-transition record.
 *
 * [V] Confirmed by dComIfGp_setNextStage, which writes three of these fields
 *     through the heap gameInfo pointer. All three land on their GameCube
 *     offsets with matching types, which also pins mRestart inside dSv_info_c:
 *
 *       heap+0x115C  sth   -> save+0x113C -> mRestart+0x14  mStartCode
 *       heap+0x1170  stfs  -> save+0x1150 -> mRestart+0x28  mLastSpeedF
 *       heap+0x1174  stw   -> save+0x1154 -> mRestart+0x2C  mLastMode
 *
 *     mLastSpeedF and mLastMode are written adjacently, which is the GameCube
 *     setLastSceneInfo(speed, mode) pair.
 *
 * [V] The other side of the record is the player init at 0x025C1384, which
 *     for a spawn point of -1 builds Link's create parameters from it: the
 *     parameters word from +0x24, the position from +0x18..+0x20 and the angle
 *     from +0x16, then clears +0x24 - GameCube's dStage_playerInit line for
 *     line. -3 reads the turn-restart record at +0x130 (a cXyz at +0x00, a
 *     halfword at +0x10, a word at +0x0C) the same way. That is the second
 *     sighting for mRestartAngle and mRestartParam. */
typedef struct dSv_restart_c {
    /* 0x00 */ s8   mRestartRoom;      /* [P] byte, 6 fns */
    /* 0x01 */ s8   mOption;           /* [P] byte, 3 fns */
    /* 0x02 */ s8   mOptionRoomNo;     /* [P] byte, 4 fns */
    /* 0x03 */ u8   _pad_03;           /* [?] */
    /* 0x04 */ s16  mOptionPoint;      /* [P] halfword */
    /* 0x06 */ s16  mOptionRoomAngleY; /* [P] halfword, 4 fns */
    /* 0x08 */ cXyz mOptionRoomPos;    /* [V] three consecutive 4-byte reads
                                        *     at +0x08/+0x0C/+0x10          */
    /* 0x14 */ s16  mStartCode;        /* [V] spawn point written on a warp */
    /* 0x16 */ s16  mRestartAngle;     /* [V] the -1 spawn facing */
    /* 0x18 */ cXyz mRestartPos;       /* [V] three consecutive 4-byte reads
                                        *     at +0x18/+0x1C/+0x20          */
    /* 0x24 */ u32  mRestartParam;     /* [V] Link's create parameters for a
                                        *     -1 spawn: room in bits 0..5,
                                        *     start mode in bits 12..15    */
    /* 0x28 */ f32  mLastSpeedF;       /* [V] */
    /* 0x2C */ u32  mLastMode;         /* [V] */
} dSv_restart_c;
WWHD_ASSERT_OFFSET(dSv_restart_c, mStartCode,  0x14);
WWHD_ASSERT_OFFSET(dSv_restart_c, mLastSpeedF, 0x28);
WWHD_ASSERT_OFFSET(dSv_restart_c, mLastMode,   0x2C);
WWHD_ASSERT_SIZE  (dSv_restart_c,              0x30);

/* dSv_turnRestart_c - the record a spawn point of -3 resumes from: Link AND
 * the King of Red Lions. The Song of Passing (dStage_turnRestart, 0x025C3DE4)
 * and the fall-out-of-the-world recovery (dStage_escapeRestart, 0x025C3ED0)
 * both fill it and queue a -3 warp.
 *
 * [V] Every field from three unrelated sites agreeing on the offsets:
 *     the setter at 0x025B998C writes +0x00 (pos), +0x0C (param), +0x10
 *     (angle), +0x12 (room), +0x13 (zero), +0x24 (ship pos), +0x30 (ship
 *     angle) and +0x34 (has ship); dStage_playerInit (0x025C1384) reads
 *     +0x00/+0x0C/+0x10 for a -3 spawn; the two -3 warps read +0x12 for the
 *     room; and dStage_setShipPos (0x025C23E0) reads +0x24/+0x30/+0x34 to
 *     place the boat, then clears +0x34. GameCube-identical. */
typedef struct dSv_turnRestart_c {
    /* 0x00 */ cXyz mPosition;      /* [V] */
    /* 0x0C */ u32  mParam;         /* [V] Link's create parameters */
    /* 0x10 */ s16  mAngleY;        /* [V] */
    /* 0x12 */ s8   mRoomNo;        /* [V] */
    /* 0x13 */ u8   _pad_13;        /* [V] zeroed by the setter */
    /* 0x14 */ u8   _unk_14[0x10];  /* [?] never touched */
    /* 0x24 */ cXyz mShipPos;       /* [V] */
    /* 0x30 */ s16  mShipAngleY;    /* [V] */
    /* 0x32 */ u8   _pad_32[2];     /* [?] */
    /* 0x34 */ u32  mHasShip;       /* [V] non-zero: the boat is placed at
                                     *     mShipPos on the reload         */
} dSv_turnRestart_c;
WWHD_ASSERT_OFFSET(dSv_turnRestart_c, mParam,      0x0C);
WWHD_ASSERT_OFFSET(dSv_turnRestart_c, mAngleY,     0x10);
WWHD_ASSERT_OFFSET(dSv_turnRestart_c, mRoomNo,     0x12);
WWHD_ASSERT_OFFSET(dSv_turnRestart_c, mShipPos,    0x24);
WWHD_ASSERT_OFFSET(dSv_turnRestart_c, mShipAngleY, 0x30);
WWHD_ASSERT_OFFSET(dSv_turnRestart_c, mHasShip,    0x34);
WWHD_ASSERT_SIZE  (dSv_turnRestart_c,              0x38);

/* dSv_info_c - live save state, the root of all save access.
 *   [V] size 0x12A0   the gameInfo constructor memsets exactly this
 *   [V] mMemory @0x0778  switch ids <0x80 and item ids <0x40 route here
 *   [V] mDan    @0x079C  switch ids 0x80..0xBF route here
 *   [V] mZone   @0x07A8  stride 0x4C, 0x20 entries
 *   [V] mRestart@0x1128  see dSv_restart_c above
 *   [V] mTurnRestart@0x1258  see dSv_turnRestart_c above */
typedef struct dSv_info_c {
    /* 0x0000 */ dSv_save_c   mSavedata;                /* [V] */
    /* 0x0778 */ dSv_memory_c mMemory;                  /* [V] */
    /* 0x079C */ dSv_danBit_c mDan;                     /* [V] */
    /* 0x07A8 */ dSv_zone_c   mZone[dSv_ZONE_MAX];      /* [V] */
    /* 0x1128 */ dSv_restart_c mRestart;                 /* [V] */
    /* 0x1158 */ dSv_event_c  mTmp;                     /* [V] a second event
                                                        *     bank; the same
                                                        *     five routines
                                                        *     are called on it */
    /* 0x1258 */ dSv_turnRestart_c mTurnRestart;      /* [V] */
    /* 0x1290 */ u8           mDataNum;                 /* [V] the slot the file
                                                        *     select loaded
                                                        *     (0x02721908) and
                                                        *     saves go back to */
    /* 0x1291 */ u8           mNewFile;                 /* [P] byte, write-only */
    /* 0x1292 */ u8           mNoFile;                  /* [P] byte, write-only */
    /* 0x1293 */ u8           _pad_1293[5];             /* [?] */
    /* 0x1298 */ u64          mMemCardCheckID;          /* [I] never touched */
} dSv_info_c;
WWHD_ASSERT_OFFSET(dSv_info_c, mMemory, 0x0778);
WWHD_ASSERT_OFFSET(dSv_info_c, mDan,    0x079C);
WWHD_ASSERT_OFFSET(dSv_info_c, mZone,   0x07A8);
WWHD_ASSERT_SIZE  (dSv_info_c, 0x12A0);

/* Routine addresses live in wwhd_map.h, one row per slot across all three
 * regions: dSv_info_{on,off,is,rev}Switch, dSv_info_{on,is}Item,
 * dSv_info_{get,put}Save, and the leaf banks dSv_{memBit,danBit,zoneBit}_*,
 * dSv_zone_ct and dSv_ocean_onSvBit. Signatures are on the rows there.
 *
 * The chart, collect, ocean and event banks are NOT called. Their routines are
 * three lines of bit arithmetic, so d_save_bits.h reimplements them rather
 * than jumping into the game for a shift and a mask. Each was transcribed from
 * the WWHD routine it mirrors, and the layout test replays it against the
 * offsets read out of that routine. */

/* ========================================================================
 * The save file, cking.sav, and its packed slot
 *
 * [V] The file select's slot load (0x02721908) hands slot i of the raw file
 * buffer to dSv_info_c::card_to_memory (0x025BA7B0), which indexes it as
 * base + i * 0xA94 and copies twenty runs into the live block. memory_to_card
 * (0x025BA9FC) writes the same twenty runs the other way, and the default-slot
 * builder (0x025BAC50) lays them out identically - three functions agreeing
 * on every offset and size. The runs are contiguous in the file and total
 * 0x768 bytes; the rest of the 0xA94 stride, and the trailer after the three
 * slots, belong to the file layer and are not read here. The block's own
 * padding is what the runs skip in memory.
 * ===================================================================== */

#define dSv_SAVEFILE_SLOT_COUNT   3
#define dSv_SAVEFILE_SLOT_STRIDE  0xA94
#define dSv_SAVEFILE_SLOT_PACKED  0x768
#define dSv_SAVEFILE_MIN_SIZE     (dSv_SAVEFILE_SLOT_COUNT * dSv_SAVEFILE_SLOT_STRIDE)

typedef struct dSv_packedRun_t {
    u16 packed;   /* offset inside the slot */
    u16 live;     /* offset inside dSv_save_c */
    u16 size;
} dSv_packedRun_t;

static const dSv_packedRun_t dSv_packedRuns[] = {
    { 0x000, 0x000, 0x018 },  /* mPlayer.mPlayerStatusA */
    { 0x018, 0x018, 0x018 },  /* mPlayer.mPlayerStatusB */
    { 0x030, 0x030, 0x00C },  /* mPlayer.mReturnPlace   */
    { 0x03C, 0x03C, 0x015 },  /* mPlayer.mPlayerItem    */
    { 0x051, 0x051, 0x015 },  /* mPlayer.mGetItem       */
    { 0x066, 0x066, 0x008 },  /* mPlayer.mItemRecord    */
    { 0x06E, 0x06E, 0x008 },  /* mPlayer.mItemMax       */
    { 0x076, 0x076, 0x018 },  /* mPlayer.mBagItem       */
    { 0x08E, 0x090, 0x00C },  /* mPlayer.mGetBagItem    */
    { 0x09A, 0x09C, 0x018 },  /* mPlayer.mBagItemRecord */
    { 0x0B2, 0x0B4, 0x00D },  /* mPlayer.mCollect       */
    { 0x0BF, 0x0C4, 0x084 },  /* mPlayer.mMap           */
    { 0x143, 0x148, 0x05C },  /* mPlayer.mInfo          */
    { 0x19F, 0x1A4, 0x005 },  /* mPlayer.mConfig        */
    { 0x1A4, 0x1AC, 0x010 },  /* mPlayer.mPriest        */
    { 0x1B4, 0x1BC, 0x1C0 },  /* mPlayer.mStatusC[4]    */
    { 0x374, 0x380, 0x240 },  /* mMemory[16]            */
    { 0x5B4, 0x5C0, 0x064 },  /* mOcean                 */
    { 0x618, 0x624, 0x100 },  /* mEvent                 */
    { 0x718, 0x724, 0x050 },  /* mReserve               */
};
#define dSv_PACKED_RUN_COUNT \
    ((int)(sizeof(dSv_packedRuns) / sizeof(dSv_packedRuns[0])))

/** [V] card_to_memory forces the second config byte to 2 after the copy. */
#define dSv_CONFIG_FORCED_INDEX 1
#define dSv_CONFIG_FORCED_VALUE 2

/** [V] Slot `slot` of a file image of `size` bytes, or NULL if it does not
 *  fit. Slot 0 starts at offset 0: the game passes the raw file buffer. */
static __inline const u8* dSv_saveFileSlot(const u8* file, u32 size, int slot) {
    if (!file || slot < 0 || slot >= dSv_SAVEFILE_SLOT_COUNT)
        return (const u8*)0;
    if (size < (u32)(slot + 1) * (u32)dSv_SAVEFILE_SLOT_STRIDE)
        return (const u8*)0;
    return file + (u32)slot * (u32)dSv_SAVEFILE_SLOT_STRIDE;
}

/** [V] Copy the twenty runs of a packed slot into a block, leaving the
 *  block's padding alone - exactly card_to_memory's copies. */
static __inline void dSv_unpackSlot(const u8* slot, dSv_save_c* out) {
    int i;
    u16 n;
    u8* dst = (u8*)out;
    if (!slot || !out)
        return;
    for (i = 0; i < dSv_PACKED_RUN_COUNT; i++) {
        const dSv_packedRun_t* r = &dSv_packedRuns[i];
        for (n = 0; n < r->size; n++)
            dst[r->live + n] = slot[r->packed + n];
    }
}

/** [V] The inverse: memory_to_card's copies. Only the runs are written. */
static __inline void dSv_packSlot(const dSv_save_c* in, u8* slot) {
    int i;
    u16 n;
    const u8* src = (const u8*)in;
    if (!slot || !in)
        return;
    for (i = 0; i < dSv_PACKED_RUN_COUNT; i++) {
        const dSv_packedRun_t* r = &dSv_packedRuns[i];
        for (n = 0; n < r->size; n++)
            slot[r->packed + n] = src[r->live + n];
    }
}

#endif /* LIBWWHD_D_SAVE_H */
