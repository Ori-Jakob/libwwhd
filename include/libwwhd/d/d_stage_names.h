#ifndef LIBWWHD_D_STAGE_NAMES_H
#define LIBWWHD_D_STAGE_NAMES_H

#include "libwwhd/wwhd_types.h"

/**
 * libwwhd - English names for stage codes and sea rooms.
 *
 * The binary carries no English stage names; the codes are the archive names
 * under content/Common/Stage. The names here are the community's, as the
 * randomizer and the speedrun guides use them, and they are [P]: a name is a
 * reading of what the code is used for, never something the game states. A
 * code that is not listed, or is listed only by category, comes back NULL so
 * a caller shows the code itself rather than a guess.
 *
 * The island names ARE the game's: dIsleRoom_*_e in the GameCube decomp, one
 * per sea room, matching the sea chart's own labels.
 */

typedef struct wwhd_stageName_t {
    const char* code;
    const char* name;
} wwhd_stageName_t;

static const wwhd_stageName_t wwhd_stageNames[] = {
    /* the sea and its framings */
    { "sea",     "The Great Sea" },
    { "sea_T",   "Title Screen" },
    { "sea_E",   "Ending" },
    { "ENDumi",  "Ending" },
    { "A_umikz", "The Great Sea (escape from Forsaken Fortress)" },
    /* Outset Island */
    { "LinkRM",  "Link's House" },
    { "LinkUG",  "Link's House (under the floor)" },
    { "Ojhous",  "Orca's House" },
    { "Ojhous2", "Sturgeon's House" },
    { "Omasao",  "Mesa's House" },
    { "Onobuta", "Abe and Rose's House" },
    { "A_mori",  "Forest of Fairies" },
    { "Pjavdou", "Jabun's Cavern" },
    { "Cave09",  "Savage Labyrinth" },
    /* Windfall Island */
    { "Obombh",  "Bomb Shop" },
    { "Ocmera",  "Lenzo's House" },
    { "Opub",    "Cafe Bar" },
    { "Pfigure", "Nintendo Gallery" },
    { "figureA", "Nintendo Gallery (room A)" },
    { "figureB", "Nintendo Gallery (room B)" },
    { "figureC", "Nintendo Gallery (room C)" },
    { "figureD", "Nintendo Gallery (room D)" },
    { "figureE", "Nintendo Gallery (room E)" },
    { "figureF", "Nintendo Gallery (room F)" },
    { "figureG", "Nintendo Gallery (room G)" },
    /* Dragon Roost */
    { "Adanmae", "Dragon Roost Island (pond)" },
    { "Atorizk", "Dragon Roost Island (Rito Aerie)" },
    { "M_NewD2", "Dragon Roost Cavern" },
    { "M_Dra09", "Dragon Roost Cavern (mini-boss)" },
    { "M_DragB", "Dragon Roost Cavern (Gohma)" },
    /* Forest Haven */
    { "Omori",   "Forest Haven" },
    { "kindan",  "Forbidden Woods" },
    { "kinMB",   "Forbidden Woods (mini-boss)" },
    { "kinBOSS", "Forbidden Woods (Kalle Demos)" },
    /* Tower of the Gods and Hyrule */
    { "Siren",   "Tower of the Gods" },
    { "SirenMB", "Tower of the Gods (mini-boss)" },
    { "SirenB",  "Tower of the Gods (Gohdan)" },
    { "Hyrule",  "Hyrule Castle" },
    { "Hyroom",  "Hyrule Castle (interior)" },
    { "kenroom", "Master Sword Chamber" },
    /* Earth and Wind */
    { "Edaichi", "Earth Temple (entrance)" },
    { "M_Dai",   "Earth Temple" },
    { "M_DaiMB", "Earth Temple (mini-boss)" },
    { "M_DaiB",  "Earth Temple (Jalhalla)" },
    { "Ekaze",   "Wind Temple (entrance)" },
    { "kaze",    "Wind Temple" },
    { "kazeMB",  "Wind Temple (mini-boss)" },
    { "kazeB",   "Wind Temple (Molgera)" },
    /* Forsaken Fortress */
    { "MajyuE",  "Forsaken Fortress" },
    { "majroom", "Forsaken Fortress (interior)" },
    { "ma2room", "Forsaken Fortress (interior, second visit)" },
    { "ma3room", "Forsaken Fortress (interior, third visit)" },
    { "Mjtower", "Forsaken Fortress (tower)" },
    { "M2tower", "Forsaken Fortress (Helmaroc King)" },
    { "M2ganon", "Forsaken Fortress (Ganondorf)" },
    /* Ganon's Tower */
    { "GTower",  "Ganon's Tower" },
    { "GanonA",  "Ganon's Tower (A)" },
    { "GanonB",  "Ganon's Tower (B)" },
    { "GanonC",  "Ganon's Tower (C)" },
    { "GanonD",  "Ganon's Tower (D)" },
    { "GanonE",  "Ganon's Tower (E)" },
    { "GanonJ",  "Ganon's Tower (maze)" },
    { "GanonK",  "Ganon's Tower (Puppet Ganon)" },
    { "GanonL",  "Ganon's Tower (L)" },
    { "GanonM",  "Ganon's Tower (M)" },
    { "GanonN",  "Ganon's Tower (Ganondorf)" },
    { "Xboss0",  "Ganon's Tower (Gohma refight)" },
    { "Xboss1",  "Ganon's Tower (Kalle Demos refight)" },
    { "Xboss2",  "Ganon's Tower (Jalhalla refight)" },
    { "Xboss3",  "Ganon's Tower (Molgera refight)" },
    /* islands and ships */
    { "Abesso",  "Cabana" },
    { "Abship",  "Submarine" },
    { "PShip",   "Pirate Ship" },
    { "PShip2",  "Pirate Ship" },
    { "PShip3",  "Pirate Ship" },
    { "ShipD",   "Ghost Ship" },
    { "MiniHyo", "Ice Ring Isle (cave)" },
    { "MiniKaz", "Fire Mountain (cave)" },
    { "kazan",   "Fire Mountain (interior)" },
    { "WarpD",   "Diamond Steppe Island (warp maze)" },
    { "Fairy01", "Great Fairy Fountain" },
    { "Fairy02", "Great Fairy Fountain" },
    { "Fairy03", "Great Fairy Fountain" },
    { "Fairy04", "Great Fairy Fountain" },
    { "Fairy05", "Great Fairy Fountain" },
    { "Fairy06", "Great Fairy Fountain" },
    /* development */
    { "DmSpot0", "Debug stage" },
    { "E3ROOP",  "E3 demo" },
    { "ITest61", "Test stage" },
    { "ITest62", "Test stage" },
    { "ITest63", "Test stage" },
};

#define WWHD_STAGE_NAME_COUNT \
    ((int)(sizeof(wwhd_stageNames) / sizeof(wwhd_stageNames[0])))

/** [P] The island on a sea room, from the GameCube dIsleRoom_*_e names. */
static const char* const wwhd_islandNames[49] = {
    "Forsaken Fortress",      "Star Island",            "Northern Fairy Island",
    "Gale Isle",              "Crescent Moon Island",   "Seven-Star Isles",
    "Overlook Island",        "Four-Eye Reef",          "Mother and Child Isles",
    "Spectacle Island",       "Windfall Island",        "Pawprint Isle",
    "Dragon Roost Island",    "Flight Control Platform","Western Fairy Island",
    "Rock Spire Isle",        "Tingle Island",          "Northern Triangle Island",
    "Eastern Fairy Island",   "Fire Mountain",          "Star Belt Archipelago",
    "Three-Eye Reef",         "Greatfish Isle",         "Cyclops Reef",
    "Six-Eye Reef",           "Tower of the Gods",      "Eastern Triangle Island",
    "Thorned Fairy Island",   "Needle Rock Isle",       "Islet of Steel",
    "Stone Watcher Island",   "Southern Triangle Island","Private Oasis",
    "Bomb Island",            "Bird's Peak Rock",       "Diamond Steppe Island",
    "Five-Eye Reef",          "Shark Island",           "Southern Fairy Island",
    "Ice Ring Isle",          "Forest Haven",           "Cliff Plateau Isles",
    "Horseshoe Island",       "Outset Island",          "Headstone Island",
    "Two-Eye Reef",           "Angular Isles",          "Boating Course",
    "Five-Star Isles",
};

static __inline int wwhd_stageCodeEquals(const char* a, const char* b) {
    int i;
    for (i = 0; i < 8; i++) {
        if (a[i] != b[i])
            return 0;
        if (a[i] == '\0')
            return 1;
    }
    return 1;
}

/** [P] The English name for a stage code, or NULL when there is none. */
static __inline const char* wwhd_stageName(const char* code) {
    int i;
    if (!code || !code[0])
        return (const char*)0;
    for (i = 0; i < WWHD_STAGE_NAME_COUNT; i++)
        if (wwhd_stageCodeEquals(wwhd_stageNames[i].code, code))
            return wwhd_stageNames[i].name;
    return (const char*)0;
}

/** [P] The island on sea room 1..49, or NULL. Room 0 is the sea floor. */
static __inline const char* wwhd_islandName(int seaRoom) {
    if (seaRoom < 1 || seaRoom > 49)
        return (const char*)0;
    return wwhd_islandNames[seaRoom - 1];
}

/**
 * [P] A place name for a stage and room: the island for a sea room, the
 * stage's English name otherwise, and the code itself when nothing is known.
 * Writes at most `cap` bytes including the terminator.
 */
static __inline void wwhd_placeName(const char* code, int room, char* out, int cap) {
    const char* name;
    int n = 0;
    const char* src;
    if (!out || cap <= 0)
        return;
    name = (const char*)0;
    if (code && wwhd_stageCodeEquals(code, "sea"))
        name = wwhd_islandName(room);
    if (!name)
        name = wwhd_stageName(code);
    src = name ? name : (code ? code : "");
    while (src[n] && n < cap - 1) {
        out[n] = src[n];
        n++;
    }
    out[n] = '\0';
}

#endif /* LIBWWHD_D_STAGE_NAMES_H */
