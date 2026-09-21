#ifndef LIBWWHD_D_STAGE_ROOM_H
#define LIBWWHD_D_STAGE_ROOM_H

#include "libwwhd/wwhd_types.h"
#include "libwwhd/wwhd_region.h"
#include "libwwhd/d/d_stage.h"
#include "libwwhd/d/d_com_inf_game.h"

/**
 * libwwhd - room streaming inside a stage (d_stage.cpp, dStage_roomControl_c)
 *
 * A stage's rooms are not all resident. Every frame the player hands the
 * polygon under his feet to dStage_RoomCheck (USA 0x025C35E8), which reads the
 * polygon's room number and, when it differs from mStayNo, calls
 * zoneCountCheck (0x025C33E8) to move the stay room over, then looks the room
 * up in the stage's RTBL - one entry per room listing the rooms that must be
 * resident while standing in it - and hands that list to loadRoom (0x025C34AC).
 * loadRoom marks resident rooms missing from the list for destruction, waits
 * for those to go, and then creates a room scene for each listed room that is
 * not loaded yet. It is a state machine: it returns 0 while any room is still
 * loading or unloading and has to be called again each frame.
 *
 * So room streaming is driven purely by where Link's ground polygon is. A tool
 * that moves Link into a room whose collision is not resident has to do the
 * room check's job itself first, or Link falls through a room that is not
 * there yet. These helpers expose that job. All three functions were read in
 * every build; EUR sits 0x40 below USA, JAP 0x14, and the static state moves
 * 8 bytes lower on JAP with the rest of the room table.
 *
 * The room status flags, from loadRoom and the room scene (d_s_room.cpp):
 * loadRoom sets LOADING (or LOADING|NO_BG when the entry does not ask for the
 * collision) on create and UNLOADING on the rooms it drops; the room scene
 * swaps LOADING for LOADED when its create phases finish, and clears the
 * record when it is deleted.
 */
#define WWHD_ROOM_FLAG_LOADED    0x01u  /* [V] */
#define WWHD_ROOM_FLAG_LOADING   0x02u  /* [V] */
#define WWHD_ROOM_FLAG_UNLOADING 0x04u  /* [V] */
#define WWHD_ROOM_FLAG_NO_BG     0x08u  /* [V] */
#define WWHD_ROOM_FLAG_DRAW      0x10u  /* [V] */

/** [V] One RTBL entry: the rooms to have resident while standing in a room.
 *  Each byte of the list carries the room number in bits 0..5 and 0x80 when
 *  that room's collision is wanted. GameCube-identical. */
typedef struct roomRead_data_class {
    /* 0x0 */ u8          num;        /* [V] entries in mRooms */
    /* 0x1 */ u8          _unk_1;     /* [?] */
    /* 0x2 */ u8          mTimePass;  /* [V] low two bits go to m_time_pass */
    /* 0x3 */ u8          _pad_3;     /* [?] */
    /* 0x4 */ wwhd_gptr_t mRooms;     /* [V] const u8* */
} roomRead_data_class;
WWHD_ASSERT_SIZE(roomRead_data_class, 0x8);

/** [V] The RTBL: entries indexed by room number, which is how the reverb
 *  lookup (dComIfGp_getReverb) and the room check both use it. */
typedef struct roomRead_class {
    /* 0x0 */ s32         num;       /* [V] */
    /* 0x4 */ wwhd_gptr_t mEntries;  /* [V] roomRead_data_class** */
} roomRead_class;
WWHD_ASSERT_SIZE(roomRead_class, 0x8);

#define WWHD_ROOMREAD_ROOM_MASK 0x3Fu  /* [V] */
#define WWHD_ROOMREAD_BG_BIT    0x80u  /* [V] */

/** [V] The stage directory object embedded at play+0x3EB0 answers getRoom()
 *  through vtable slot 0x64: the room check calls
 *  (*(*(play+0x3EB0))+0x64)(play+0x3EB0) in every build. */
#define WWHD_STAGEDT_VT_GETROOM 0x64

/** [V] dComIfG_play_c::mRoomCtrl, the `this` handed to loadRoom and
 *  zoneCountCheck: static+0x51CC in the room check, so play+0x3F2C. Neither
 *  routine reads it - all their state is static - but it is passed as the game
 *  does. Below the JAP boundary. */
#define WWHD_PLAY_OFF_ROOMCTRL 0x3F2C

typedef roomRead_class* (*dStage_getRoom_t)(void* stageDt);
typedef int  (*dStage_loadRoom_t)(void* roomCtrl, int roomCount, const u8* rooms);
typedef void (*dStage_zoneCountCheck_t)(void* roomCtrl, int roomNo);

/** [V] The room control object, or NULL. */
static __inline void* dStage_getRoomControl(void) {
    return dComIfGp_playAt(WWHD_PLAY_OFF_ROOMCTRL);
}

/** [V] One room's status flags, or 0 when unavailable. */
static __inline u8 dStage_getRoomFlags(int roomNo) {
    dStage_roomStatus_c* r = dStage_getRoomStatus(roomNo);
    return r ? r->mFlags : (u8)0;
}

/** [V] Non-zero when the room's scene has finished creating - collision
 *  included when the RTBL asked for it. */
static __inline int dStage_isRoomLoaded(int roomNo) {
    return (dStage_getRoomFlags(roomNo) & WWHD_ROOM_FLAG_LOADED) != 0;
}

/** [V] Non-zero while any room is still loading or unloading; loadRoom does
 *  nothing and returns 0 in that state. */
static __inline int dStage_isRoomStreamingBusy(void) {
    dStage_roomStatus_c* t = dStage_getRoomStatusTable();
    int i;
    if (!t)
        return 0;
    for (i = 0; i < WWHD_ROOM_MAX; ++i)
        if (t[i].mFlags & (WWHD_ROOM_FLAG_LOADING | WWHD_ROOM_FLAG_UNLOADING))
            return 1;
    return 0;
}

#ifdef WWHD_ENABLE_GAME_CALLS

/** [V] The loaded stage's RTBL, or NULL before a stage is loaded. Calls the
 *  stage directory's virtual getRoom(), as the room check does. */
static __inline roomRead_class* dStage_getRoomReadTable(void) {
    dStage_dt_c* dt = dComIfGp_getStageDt();
    wwhd_gptr_t fn;
    dStage_getRoom_t getRoom;
    if (!dt || !dt->_vtable || !wwhd_textResolved)
        return (roomRead_class*)0;
    fn = *WWHD_AT(wwhd_gptr_t, dt->_vtable + WWHD_STAGEDT_VT_GETROOM);
    if (!fn)
        return (roomRead_class*)0;
    getRoom = (dStage_getRoom_t)WWHD_PTR(fn);
    return getRoom(dt);
}

/** [V] The RTBL entry for a room, or NULL if the table has none for it. */
static __inline roomRead_data_class* dStage_getRoomReadEntry(int roomNo) {
    roomRead_class* table = dStage_getRoomReadTable();
    wwhd_gptr_t* entries;
    if (!table || roomNo < 0 || roomNo >= table->num || !table->mEntries)
        return (roomRead_data_class*)0;
    entries = WWHD_AT(wwhd_gptr_t, table->mEntries);
    if (!entries[roomNo])
        return (roomRead_data_class*)0;
    return WWHD_AT(roomRead_data_class, entries[roomNo]);
}

/** [V] Does this entry's list name the room? The room's own entry always
 *  should; a missing self-reference means the table cannot bring it in. */
static __inline int dStage_roomReadEntryHas(const roomRead_data_class* e, int roomNo) {
    const u8* rooms;
    int i;
    if (!e || !e->mRooms)
        return 0;
    rooms = WWHD_AT(const u8, e->mRooms);
    for (i = 0; i < (int)e->num; ++i)
        if ((rooms[i] & WWHD_ROOMREAD_ROOM_MASK) == (u8)roomNo)
            return 1;
    return 0;
}

/**
 * [V] One step of the room streaming state machine for `roomNo`'s RTBL entry:
 * drops resident rooms the entry does not list, then creates the listed rooms
 * that are missing. Returns 1 when the list is satisfied or the creates were
 * issued, 0 while rooms are still loading or unloading, -1 when the stage has
 * no entry for the room. Call it every frame until dStage_isRoomLoaded() and
 * !dStage_isRoomStreamingBusy(), exactly as the player's room check does.
 */
static __inline int dStage_loadRoomFor(int roomNo) {
    roomRead_data_class* e = dStage_getRoomReadEntry(roomNo);
    void* ctrl = dStage_getRoomControl();
    dStage_loadRoom_t load;
    if (!e || !ctrl || !wwhd_regionResolved || !wwhd_textResolved)
        return -1;
    load = WWHD_FN(dStage_loadRoom_t, wwhd_map->dStage_loadRoom);
    return load(ctrl, (int)e->num, WWHD_AT(const u8, e->mRooms));
}

/** [V] Make `roomNo` the stay room, with the zone bookkeeping the room check
 *  does when Link's ground polygon changes room. The player's own room check
 *  performs this as soon as he stands in the room, so a tool rarely needs it. */
static __inline void dStage_zoneCountCheck(int roomNo) {
    void* ctrl = dStage_getRoomControl();
    dStage_zoneCountCheck_t fn;
    if (!ctrl || !wwhd_regionResolved || !wwhd_textResolved)
        return;
    fn = WWHD_FN(dStage_zoneCountCheck_t, wwhd_map->dStage_zoneCountCheck);
    fn(ctrl, roomNo);
}

#endif /* WWHD_ENABLE_GAME_CALLS */

#endif /* LIBWWHD_D_STAGE_ROOM_H */
