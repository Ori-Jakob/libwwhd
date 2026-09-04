#ifndef LIBWWHD_D_KANKYO_H
#define LIBWWHD_D_KANKYO_H

#include "libwwhd/wwhd_types.h"
#include "libwwhd/wwhd_region.h"
#include "libwwhd/f_op/f_op_actor.h"

/**
 * libwwhd - environment lighting and the day-night cycle (d_kankyo.cpp)
 *
 * The GameCube dScnKy_env_light_c, kept in WWHD as a lazily constructed static
 * behind one accessor with a construct-once guard. Only what has been read out
 * of code is named; the interior is left as an explicit gap rather than padded
 * with guesses. docs/SUBSYSTEMS.md (Environment) records how mTime was pinned down.
 */

/** [V] One time-band record. The colour solve walks at most 11 of these,
 *  comparing mTime against each {lo, hi} pair to pick a band. */
typedef struct dKy_timeBand_c {
    /* 0x0 */ f32 mLo;        /* [V] */
    /* 0x4 */ f32 mHi;        /* [V] */
    /* 0x8 */ u8  mIdxA;      /* [V] */
    /* 0x9 */ u8  mIdxB;      /* [V] second index read at +0x09 */
    /* 0xA */ u8  _pad_0A[2]; /* [?] */
} dKy_timeBand_c;
WWHD_ASSERT_SIZE(dKy_timeBand_c, 0xC);

/**
 * dScnKy_env_light_c.
 *
 * Size is not known - the construct-once guard sits 0x194C past the base, which
 * bounds it but does not give it - so the struct stops after the last confirmed
 * field and is deliberately not size-asserted.
 */
typedef struct dScnKy_env_light_c {
    /* 0x0000 */ u8          _unk_0000[0x08];   /* [?] */
    /* 0x0008 */ wwhd_gptr_t mpColorTableA;     /* [V] */
    /* 0x000C */ wwhd_gptr_t mpColorTableB;     /* [V] */
    /* 0x0010 */ wwhd_gptr_t mpTimeBandTable;   /* [V] dKy_timeBand_c* */
    /* 0x0014 */ u8          _unk_0014[0x09B4 - 0x0014]; /* [?] */
    /* 0x09B4 */ cXyz        mWindVec;          /* [V] wind direction and
                                                 *     strength as one vector */
    /* 0x09C0 */ wwhd_gptr_t mpWindVecOverride; /* [V] cXyz*, when set it wins */
    /* 0x09C4 */ u8          _unk_09C4[0x1020 - 0x09C4]; /* [?] see the note */
    /* 0x1020 */ f32         mTime;             /* [V] 0..360 over one day */
} dScnKy_env_light_c;
WWHD_ASSERT_OFFSET(dScnKy_env_light_c, mpTimeBandTable,   0x0010);
WWHD_ASSERT_OFFSET(dScnKy_env_light_c, mWindVec,          0x09B4);
WWHD_ASSERT_OFFSET(dScnKy_env_light_c, mpWindVecOverride, 0x09C0);
WWHD_ASSERT_OFFSET(dScnKy_env_light_c, mTime,             0x1020);

/*
 * The wind block starts as the GameCube GB_WIND_INFLUENCE - mWindVec at +0x00
 * and mpWindVecOverride at +0x0C both land exactly - but it DIVERGES after
 * that. Where GameCube has an s16 angle pair at +0x28/+0x2A, WWHD has eight
 * consecutive floats running 0x09D8..0x09F4. mWindPower is therefore NOT at the
 * GameCube offset and is deliberately not declared: guessing it would be a
 * write into an unknown float. Use mWindVec, whose magnitude is the strength.
 */

/** [V] One full day on mTime's scale. Same as the GameCube build. */
#define WWHD_ENVLIGHT_DAY_UNITS 360.0f

/** [V] Non-zero once the environment object has been constructed. */
static __inline int dKy_isReady(void) {
    if (!wwhd_regionResolved)
        return 0;
    return *WWHD_AT_DATA(u32, wwhd_map->envLightGuard) != 0u;
}

/** [V] The environment singleton, or NULL before it is built. */
static __inline dScnKy_env_light_c* dKy_getEnvlight(void) {
    if (!dKy_isReady())
        return (dScnKy_env_light_c*)0;
    return WWHD_AT_DATA(dScnKy_env_light_c, wwhd_map->envLight);
}

/** [V] Raw day-night time, 0..360. Returns 0 before the object exists. */
static __inline f32 dKy_getTimeRaw(void) {
    dScnKy_env_light_c* e = dKy_getEnvlight();
    return e ? e->mTime : 0.0f;
}

/**
 * [V] Day-night time as hours and minutes.
 *
 * Returns 0 without touching the outputs when the environment is not built, or
 * when the raw value is outside [0, 360) - which happens on the frames before
 * the first stage load, so callers should still show a placeholder rather than
 * assume success.
 */
static __inline int dKy_getTimeHM(u8* hour, u8* minute) {
    f32 t, hours;
    if (!dKy_isReady())
        return 0;
    t = dKy_getTimeRaw();
    if (!(t >= 0.0f) || t >= WWHD_ENVLIGHT_DAY_UNITS)
        return 0;
    hours = t * (24.0f / WWHD_ENVLIGHT_DAY_UNITS);
    if (hour)
        *hour = (u8)hours;
    if (minute)
        *minute = (u8)((hours - (f32)(int)hours) * 60.0f);
    return 1;
}


/**
 * [V] The wind vector, or NULL before the environment is built.
 *
 * Found from the accessor at 0x0255F530, which is exactly
 * `return getEnvlight() + 0x9B4` - the game handing out the address of this
 * field. Corroborated by three consecutive stfs writing 0x09B4/B8/BC as a
 * cXyz. Direction is the normalised vector; strength is its magnitude.
 */
static __inline cXyz* dKy_getWindVec(void) {
    dScnKy_env_light_c* e = dKy_getEnvlight();
    return e ? &e->mWindVec : (cXyz*)0;
}

/**
 * [V] The wind vector actually in effect.
 *
 * mpWindVecOverride takes precedence when non-NULL, which is how events and
 * the Wind Waker force a direction. Reading mWindVec alone would miss that and
 * report the ambient wind during a forced one.
 */
static __inline cXyz* dKy_getEffectiveWindVec(void) {
    dScnKy_env_light_c* e = dKy_getEnvlight();
    if (!e)
        return (cXyz*)0;
    if (e->mpWindVecOverride)
        return WWHD_AT(cXyz, e->mpWindVecOverride);
    return &e->mWindVec;
}

/**
 * [?] Point mWindVec along a game angle, keeping its current strength.
 *
 * KNOWN NOT TO TAKE EFFECT. The wind calculation at 0x0257D398 rebuilds the
 * wind into envlight+0x9FC every frame from an angle pair elsewhere in the
 * struct, so this writes a field nothing downstream reads. It is kept because
 * the arithmetic is right and will be reused once the correct input field is
 * identified - see open question 1 in docs/METHODOLOGY.md. Do not build on it.
 *
 * Strength is the vector's magnitude, so the direction is set by rebuilding the
 * unit vector and rescaling. A wind that has decayed to nothing is given unit
 * strength instead, otherwise setting its direction would be a no-op forever.
 *
 * This writes mWindVec, the AMBIENT wind. It does not disturb
 * mpWindVecOverride, so a wind the Wind Waker or an event has forced still
 * wins - which is deliberate: the override belongs to whatever set it, and
 * writing through it would corrupt another system's vector.
 */
static __inline int dKy_setWindDirection(s16 angle) {
    cM_sinCos_c* table;
    cXyz* wind;
    f32 strength;
    wind = dKy_getWindVec();
    table = cM_getSinCosTable();
    if (!wind || !table)
        return 0;
    strength = wind->x * wind->x + wind->y * wind->y + wind->z * wind->z;
    /* Compare the square, so the guard costs no square root on the common
     * path where the wind is already blowing. */
    if (strength < 0.0001f) {
        strength = 1.0f;
    } else {
        f32 g = strength;
        /* Newton steps from a rough seed. This runs every frame and the
         * result only sets a direction, so a fraction of a percent is
         * invisible and the iteration is cheaper than pulling in libm. */
        f32 r = 0.5f * (1.0f + g);
        r = 0.5f * (r + g / r);
        r = 0.5f * (r + g / r);
        r = 0.5f * (r + g / r);
        strength = r;
    }
    {
        const cM_sinCos_c* e = &table[(u16)angle >> 3];
        wind->x = e->sin * strength;
        wind->y = 0.0f;
        wind->z = e->cos * strength;
    }
    return 1;
}

#endif /* LIBWWHD_D_KANKYO_H */
