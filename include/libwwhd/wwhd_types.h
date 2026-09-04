#ifndef LIBWWHD_TYPES_H
#define LIBWWHD_TYPES_H

#include <stddef.h>

/**
 * libwwhd - base types, layout assertions and the memory hook.
 *
 * The Legend of Zelda: The Wind Waker HD (Wii U, cking.rpx).
 * PowerPC 32-bit big-endian (Espresso).
 *
 * Every structure is a real, complete, castable C struct: known fields named,
 * everything else an explicit _unk_XXXX byte array, so the struct is the right
 * size and every named field lands on its real offset. Each is followed by
 * WWHD_ASSERT_OFFSET / WWHD_ASSERT_SIZE, making a layout mistake a compile
 * error rather than a silently wrong read.
 */

typedef unsigned char      u8;
typedef signed char        s8;
typedef unsigned short     u16;
typedef signed short       s16;
typedef unsigned int       u32;
typedef signed int         s32;
typedef unsigned long long u64;
typedef signed long long   s64;
typedef float              f32;
typedef double             f64;
typedef int                BOOL;

/** Guest (Wii U virtual) address, as seen in cking.rpx. */
typedef u32 wwhd_addr_t;

/**
 * A pointer STORED INSIDE a game structure.
 *
 * The game is 32-bit, so every pointer field in it is 4 bytes. A native void*
 * would become 8 bytes on a 64-bit host and shift every following member, so
 * libwwhd never uses one inside a game struct. Dereference with WWHD_AT:
 *     J3DModel* m = WWHD_AT(J3DModel, ship->mpModelMain);
 */
typedef u32 wwhd_gptr_t;

/* --- Confidence markers ---------------------------------------------------
 *   [V] Verified  - read directly out of WWHD code or data. Trust it.
 *   [P] Probable  - one inference step from something verified, typically a
 *                   GameCube field delta applied to a verified WWHD anchor.
 *                   Confirm before relying on it for a write.
 *   [I] Inferred  - from the GameCube decomp, NOT confirmed in WWHD.
 *                   A hypothesis. Never write memory through an [I] field.
 *   [?] Unknown   - padding, or a field whose meaning is unconfirmed.
 *
 * Markers describe the OFFSET. Where a field's offset is solid but its name
 * is a guess, the comment says so explicitly.
 * ------------------------------------------------------------------------ */

/* --- Compile-time layout assertions -------------------------------------- */
#define WWHD_CAT2_(a, b) a##b
#define WWHD_CAT2(a, b)  WWHD_CAT2_(a, b)

#ifdef __COUNTER__
#define WWHD_SA_UNIQUE __COUNTER__
#else
#define WWHD_SA_UNIQUE __LINE__
#endif

/** Fails to compile if `cond` is false. */
#define WWHD_STATIC_ASSERT(cond) \
    typedef char WWHD_CAT2(wwhd_static_assert_, WWHD_SA_UNIQUE)[(cond) ? 1 : -1]

/** Assert that `member` of `type` sits at `off`. */
#define WWHD_ASSERT_OFFSET(type, member, off) \
    WWHD_STATIC_ASSERT(offsetof(type, member) == (off))

/** Assert the total size of `type`. */
#define WWHD_ASSERT_SIZE(type, size) \
    WWHD_STATIC_ASSERT(sizeof(type) == (size))

/* --- Guest memory hook ----------------------------------------------------
 * How a guest address becomes something you can dereference.
 *
 * The default is identity, which is correct for code running inside the game
 * (Wii U code injection / a loaded module). An external tool - an emulator
 * plugin, a debugger bridge, a savestate parser - should define WWHD_PTR
 * before including any libwwhd header:
 *
 *     #define WWHD_PTR(addr) (g_memBase + ((addr) - 0x02000000u))
 *     #include "libwwhd/libwwhd.h"
 *
 * NOTE: the game is big-endian. If your host is little-endian you must also
 * byte-swap on access; libwwhd does not do it for you, because doing it
 * silently would be worse than making you decide.
 * ------------------------------------------------------------------------ */
#ifndef WWHD_PTR
#define WWHD_PTR(addr) ((void *)(size_t)(addr))
#endif

/** Cast a guest address straight to a typed pointer. */
#define WWHD_AT(type, addr) ((type *)WWHD_PTR(addr))

/* --- Runtime segment shifts -----------------------------------------------
 * On real hardware the loader relocates BOTH segments, by DIFFERENT amounts.
 * An earlier revision of this header claimed .text stayed at its link address;
 * that was wrong, and calling a map .text address directly crashed the console with
 * an invalid instruction fetch. Measured on a retail Wii U:
 *
 *     .text   segment link 0x02000000  ->  runtime 0x0E000000   delta 0x0C000000
 *     .data   segment link 0x10000000  ->  runtime 0x10502200   delta 0x00502200
 *
 * Take the delta from the loader's own textOffset / dataOffset rather than
 * subtracting a link base: the .text SECTION starts at 0x02000020 while the
 * segment starts at 0x02000000, and computing from the section is off by 0x20.
 *
 * So there are three address kinds and they must not be mixed:
 *
 *     WWHD_AT      (type, addr)   already absolute - a pointer read from memory
 *     WWHD_AT_DATA (type, addr)   .data / .bss     - applies the data delta
 *     WWHD_AT_TEXT (type, addr)   .text            - applies the text delta
 *     WWHD_FN      (type, addr)   .text, as a callable
 *
 * Under an emulator both deltas are usually zero, which is exactly why this
 * class of bug survives testing and only shows up on hardware.
 *
 * The definitions below are weak, so a host that measures the real values can
 * assign to them at startup:
 *
 *     wwhd_dataDelta = measuredDataDelta;
 *     wwhd_textDelta = measuredTextDelta;
 * ------------------------------------------------------------------------ */
/* Mutable state a host assigns at startup.
 *
 * On ELF - devkitPro, which is what actually ships - these are weak so several
 * translation units share one copy and the plugin can write them once. PE/COFF
 * does NOT honour a weak definition's initialiser: the symbols come out aliased
 * and uninitialised, which showed up as wwhd_regionResolved reading 2 and a
 * region pointer of 0x1. On Windows, where libwwhd is only ever built into the
 * single-translation-unit layout test, plain statics are correct instead. */
#if defined(__GNUC__) && !defined(_WIN32)
#define WWHD_SHARED __attribute__((weak))
#elif defined(__GNUC__)
#define WWHD_SHARED static __attribute__((unused))
#else
#define WWHD_SHARED static
#endif

WWHD_SHARED u32 wwhd_dataDelta = 0;
WWHD_SHARED u32 wwhd_dataResolved = 0;
WWHD_SHARED u32 wwhd_textDelta = 0;
WWHD_SHARED u32 wwhd_textResolved = 0;

/** Apply the runtime shift to a .data / .bss address. */
#define WWHD_DATA(addr) ((wwhd_addr_t)((addr) + wwhd_dataDelta))

/** Cast a .data / .bss guest address to a typed pointer. */
#define WWHD_AT_DATA(type, addr) ((type *)WWHD_PTR(WWHD_DATA(addr)))

/** Apply the runtime shift to a .text address. */
#define WWHD_TEXT(addr) ((wwhd_addr_t)((addr) + wwhd_textDelta))

/** Cast a .text guest address to a typed pointer. */
#define WWHD_AT_TEXT(type, addr) ((type *)WWHD_PTR(WWHD_TEXT(addr)))

/**
 * Cast a .text address to a callable function pointer.
 *
 * Calling one of these before wwhd_textResolved is set will jump into unmapped
 * memory, so guard every call site on it.
 */
#define WWHD_FN(type, addr) ((type)WWHD_PTR(WWHD_TEXT(addr)))

/* --- Geometry ------------------------------------------------------------ */

/** [V] 3D float vector, 12 bytes, x/y/z contiguous. */
typedef struct cXyz {
    /* 0x0 */ f32 x;
    /* 0x4 */ f32 y;
    /* 0x8 */ f32 z;
} cXyz;
WWHD_ASSERT_SIZE(cXyz, 0xC);

/** [V] 3D short-angle vector, 6 bytes. 0x10000 == 360 degrees. */
typedef struct csXyz {
    /* 0x0 */ s16 x;
    /* 0x2 */ s16 y;
    /* 0x4 */ s16 z;
} csXyz;
WWHD_ASSERT_SIZE(csXyz, 0x6);

#endif /* LIBWWHD_TYPES_H */
