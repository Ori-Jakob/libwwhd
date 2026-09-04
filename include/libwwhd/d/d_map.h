#ifndef LIBWWHD_D_MAP_H
#define LIBWWHD_D_MAP_H

#include "libwwhd/wwhd_types.h"

/**
 * libwwhd - the sea chart grid (d_map.cpp)
 *
 * The Great Sea is a 7x7 board of square sectors. A sector is identified two
 * ways and they are NOT the same number:
 *
 *   grid no    0..48, row-major from the north-west corner. This is what the
 *              save block stores and what every helper here takes.
 *   grid pos   (x, y) in -3..3, centred on the board. This is what
 *              world-space code works in. The centre square (0,0) is grid 24,
 *              Six-Eye Reef - not Greatfish Isle, which sits at (-2, 0).
 *
 *   gridNo = (x + 3) + (y + 3) * 7
 *
 * The island index enum is the grid number: sector 43 IS Outset Island, so
 * dIsleIdx_e doubles as the sector list. Names come from the GameCube decomp
 * and are unverified as strings, but three of the VALUES are confirmed in WWHD
 * by the save initialiser, which presets exactly sectors 0, 10 and 43 to
 * visited - the three islands the game opens with, and the same three GameCube
 * presets by name.
 *
 * Whether a sector is revealed is one bit in dSv_player_map_c::mFmapBits; see
 * d_save_bits.h. Charts are a separate system that happens to share that
 * struct.
 */

/** [V] Sectors on the sea chart. */
#define WWHD_SEA_GRID_MAX 49
/** [V] Sectors per row. */
#define WWHD_SEA_GRID_W   7
/** [V] Grid positions run -3..3 on both axes. */
#define WWHD_SEA_GRID_HALF 3

/**
 * Sea sectors, in grid order. [V] for the three the initialiser presets;
 * [P] for the rest, which carry the GameCube names against a verified
 * numbering rather than a verified string.
 */
typedef enum dIsleIdx_e {
    dIsleIdx_ForsakenFortress_e       = 0,  /* [V] preset visited */
    dIsleIdx_StarIsland_e             = 1,
    dIsleIdx_NorthernFairyIsland_e    = 2,
    dIsleIdx_GaleIsle_e               = 3,
    dIsleIdx_CrescentMoonIsland_e     = 4,
    dIsleIdx_SevenStarIsles_e         = 5,
    dIsleIdx_OverlookIsland_e         = 6,
    dIsleIdx_FourEyeReef_e            = 7,
    dIsleIdx_MotherandChildIsles_e    = 8,
    dIsleIdx_SpectacleIsland_e        = 9,
    dIsleIdx_WindfallIsland_e         = 10, /* [V] preset visited */
    dIsleIdx_PawprintIsle_e           = 11,
    dIsleIdx_DragonRoostIsland_e      = 12,
    dIsleIdx_FlightControlPlatform_e  = 13,
    dIsleIdx_WesternFairyIsland_e     = 14,
    dIsleIdx_RockSpireIsle_e          = 15,
    dIsleIdx_TingleIsland_e           = 16,
    dIsleIdx_NorthernTriangleIsland_e = 17,
    dIsleIdx_EasternFairyIsland_e     = 18,
    dIsleIdx_FireMountain_e           = 19,
    dIsleIdx_StarBeltArchipelago_e    = 20,
    dIsleIdx_ThreeEyeReef_e           = 21,
    dIsleIdx_GreatfishIsle_e          = 22,
    dIsleIdx_CyclopsReef_e            = 23,
    dIsleIdx_SixEyeReef_e             = 24, /* [V] grid pos (0,0) */
    dIsleIdx_ToweroftheGods_e         = 25,
    dIsleIdx_EasternTriangleIsland_e  = 26,
    dIsleIdx_ThornedFairyIsland_e     = 27,
    dIsleIdx_NeedleRockIsle_e         = 28,
    dIsleIdx_IsletofSteel_e           = 29,
    dIsleIdx_StoneWatcherIsland_e     = 30,
    dIsleIdx_SouthernTriangleIsland_e = 31,
    dIsleIdx_PrivateOasis_e           = 32,
    dIsleIdx_BombIsland_e             = 33,
    dIsleIdx_BirdsPeakRock_e          = 34,
    dIsleIdx_DiamondSteppeIsland_e    = 35,
    dIsleIdx_FiveEyeReef_e            = 36,
    dIsleIdx_SharkIsland_e            = 37,
    dIsleIdx_SouthernFairyIsland_e    = 38,
    dIsleIdx_IceRingIsle_e            = 39,
    dIsleIdx_ForestHaven_e            = 40,
    dIsleIdx_CliffPlateauIsles_e      = 41,
    dIsleIdx_HorseshoeIsland_e        = 42,
    dIsleIdx_OutsetIsland_e           = 43, /* [V] preset visited */
    dIsleIdx_HeadstoneIsland_e        = 44,
    dIsleIdx_TwoEyeReef_e             = 45,
    dIsleIdx_AngularIsles_e           = 46,
    dIsleIdx_BoatingCourse_e          = 47,
    dIsleIdx_FiveStarIsles_e          = 48,
    dIsleIdx_COUNT_e                  = 49
} dIsleIdx_e;

/** [V] Non-zero when a grid number names a real sector. */
static __inline int dMap_gridNoValid(int gridNo) {
    return gridNo >= 0 && gridNo < WWHD_SEA_GRID_MAX;
}

/** [V] Grid position to grid number. Returns -1 when either axis is off the
 *  board, where the game asserts instead. */
static __inline int dMap_gridPos2GridNo(int gridX, int gridY) {
    if (gridX < -WWHD_SEA_GRID_HALF || gridX > WWHD_SEA_GRID_HALF ||
        gridY < -WWHD_SEA_GRID_HALF || gridY > WWHD_SEA_GRID_HALF)
        return -1;
    return (gridX + WWHD_SEA_GRID_HALF) +
           (gridY + WWHD_SEA_GRID_HALF) * WWHD_SEA_GRID_W;
}

/** [V] Grid number back to grid position. Leaves the outputs alone and returns
 *  zero when the number is off the board. */
static __inline int dMap_gridNo2GridPos(int gridNo, int* gridX, int* gridY) {
    if (!dMap_gridNoValid(gridNo) || !gridX || !gridY)
        return 0;
    *gridX = (gridNo % WWHD_SEA_GRID_W) - WWHD_SEA_GRID_HALF;
    *gridY = (gridNo / WWHD_SEA_GRID_W) - WWHD_SEA_GRID_HALF;
    return 1;
}

#endif /* LIBWWHD_D_MAP_H */
