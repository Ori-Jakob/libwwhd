#ifndef LIBWWHD_D_MSG_H
#define LIBWWHD_D_MSG_H

#include "libwwhd/wwhd_types.h"
#include "libwwhd/wwhd_map.h"
#include "libwwhd/wwhd_region.h"
#include "libwwhd/d/d_com_inf_game.h"

/**
 * libwwhd - the message box: text, choices and how a page is turned.
 *
 * WWHD DID NOT KEEP THE GAMECUBE MESSAGE CODE, and this is the one subsystem
 * where following the decomp's control flow will send you to a function that
 * does not exist. The GameCube runs one dMsg_Execute that switches on
 * msg_class::mStatus and calls dMsg_stopProc, dMsg_outnowProc and friends. WWHD
 * replaced all of it with a named state machine - ::StateID_Stop,
 * ::StateID_AutoStop, ::StateID_OutNow and 32 more - where every state is its
 * own function and every flag branch the GameCube took inside a proc became a
 * separate state. The static initialiser that writes all 35 state descriptors
 * out with their name strings is USA 0x026FFA48; descriptors are 0x30 bytes,
 * id at +0x00 and name at +0x04.
 *
 * WHAT DID SURVIVE IS THE STATUS ENUM. fopMsg_MessageStatus_e below carries the
 * decomp's values unchanged, and WWHD still publishes them into
 * dComIfG_play_c. So the decomp remains the right map for WHAT the box is
 * doing, and only the wrong map for WHICH code is doing it.
 *
 * REACHING IT. The box itself hangs off a message manager the game keeps in
 * .data (USA 0x101F4B5C, EUR/JAP unported, so no map slot), manager +0x954 for
 * the ordinary talk box. Nothing here needs that: the status is published to
 * play every frame and dComIfGp_getMesgStatus() reads it with no pointer
 * chasing and no process search, which is why it is the accessor to reach for.
 * The struct prefixes below exist for a caller that already holds a box - a
 * hook on dMsg_decideCheck gets one in r3.
 */

/**
 * [V] The message status, as published to play and as the decomp names it.
 *
 * Each value marked [V] was read out of a WWHD state function branching on it:
 * the typing state picks the next wait state on 7 / 0x0A / 0x0E / 0x15, the
 * choice-in state splits 8 from 9, and the continue state splits 0x0F from
 * 0x10. The [I] values are GameCube names carried over for completeness and
 * were NOT confirmed here; do not key behaviour on one without checking.
 */
typedef enum fopMsg_MessageStatus_e {
    fopMsgStts_NONE_e          = 0x00, /* [P] no box; what the field reads at
                                        *     rest, and the sentinel this
                                        *     header returns when unresolved  */
    fopMsgStts_MSG_PREPARING_e = 0x01, /* [I] */
    fopMsgStts_BOX_OPENING_e   = 0x02, /* [I] */
    fopMsgStts_SCOPE_OPENING_1_e = 0x03, /* [I] */
    fopMsgStts_SCOPE_OPENING_2_e = 0x04, /* [I] */
    fopMsgStts_MSG_UNK5_e      = 0x05, /* [I] */
    fopMsgStts_MSG_TYPING_e    = 0x06, /* [V] ::StateID_OutNow, text printing */
    fopMsgStts_STOP_e          = 0x07, /* [V] ::StateID_Stop / AutoStop /
                                        *     HandStop - the page is finished
                                        *     and the arrow is bouncing       */
    fopMsgStts_SELECT_2_e      = 0x08, /* [V] ::StateID_Select2               */
    fopMsgStts_SELECT_3_e      = 0x09, /* [V] ::StateID_Select3               */
    fopMsgStts_CLOSE_WAIT_e    = 0x0A, /* [V] ::StateID_CloseWait / Auto /
                                        *     Hand - waiting to shut the box  */
    fopMsgStts_SCOPE_ACTIVE_e  = 0x0B, /* [I] */
    fopMsgStts_SCOPE_DEMO_e    = 0x0C, /* [I] */
    fopMsgStts_SCOPE_WAIT_e    = 0x0D, /* [I] */
    fopMsgStts_MSG_DISPLAYED_e = 0x0E, /* [V] ::StateID_NpcAnswerWait         */
    fopMsgStts_MSG_CONTINUES_e = 0x0F, /* [V] ::StateID_Continue takes it on  */
    fopMsgStts_MSG_ENDS_e      = 0x10, /* [V] ::StateID_Continue -> Close     */
    fopMsgStts_BOX_CLOSING_e   = 0x11, /* [I] */
    fopMsgStts_BOX_CLOSED_e    = 0x12, /* [I] */
    fopMsgStts_MSG_DESTROYED_e = 0x13, /* [I] */
    fopMsgStts_SELECT_YOKO_e   = 0x14, /* [I] horizontal two-choice box       */
    fopMsgStts_INPUT_e         = 0x15, /* [V] ::StateID_Input, number entry   */
    fopMsgStts_TACT_e          = 0x16, /* [I] */
    fopMsgStts_DEMO_e          = 0x17  /* [I] */
} fopMsg_MessageStatus_e;

/**
 * [V] dComIfG_play_c offset of the published status, for dComIfGp_playAt().
 *
 * Two routes agree. dComIfGp_getMesgStatus is a two-instruction pair over
 * gameInfoStatic + 0x5BB2 in USA (0x025F795C) and EUR (0x025F791C), and over
 * + 0x5BAE in JAP (0x025F798C); gameInfoStatic + 0x12A0 is play, so that is
 * play + 0x4912 with JAP four lower - the ordinary WWHD_PLAY_TAIL_FROM shift,
 * which is why this must go through wwhd_playOfs() and not be used raw. The
 * decomp independently puts mMesgStatus at play + 0x492A, and every neighbour
 * in this part of play sits 0x18 lower in WWHD: the decomp's mItemNowRupee at
 * + 0x4926 is the mRupeeDisp this library already verified at + 0x490E.
 */
#define WWHD_PLAY_MESG_STATUS 0x4912u

/**
 * [V] dComIfG_play_c offsets that fell out of matching USA 0x026FEAF4 against
 * the decomp's dMsg_continueProc, condition for condition.
 *
 * WWHD_PLAY_MESG_BGM is the one to know about: it is
 * dComIfGp_checkMesgBgm(), and every waiting state that was read here gates its
 * button on it being clear. While a message jingle is playing the box ignores
 * the player on purpose, so anything supplying presses will appear to stall and
 * should not try to force its way past.
 */
#define WWHD_PLAY_MESG_CANCEL 0x4933u
#define WWHD_PLAY_MESG_BGM    0x4980u

/** [V] Non-zero while a message jingle is holding the box; see above. */
static __inline int dMsg_isBgmHolding(void) {
    const u8* p = (const u8*)dComIfGp_playAt(WWHD_PLAY_MESG_BGM);
    return p && *p != 0;
}

/** [V] dMsgBox_c::mPrintFlags bits, read only by the print step. */
#define WWHD_MSGBOX_PRINT_FAST 0x1u
#define WWHD_MSGBOX_PRINT_SKIP 0x2u

/** [V] dMsgBox_c::mInputFlags bits, read only by the waiting states. */
#define WWHD_MSGBOX_DECIDE 0x1u
#define WWHD_MSGBOX_CANCEL 0x2u

/**
 * [V] The print position the typing state treats as "this page is finished".
 *
 * The print step's own skip branch writes exactly this into mPrintPos, and the
 * typing state stops accumulating once mPrintPos reaches it, so a print step
 * that returns this many characters completes the page in one frame however
 * long the line is - the caller clamps to mLineLimit either way.
 */
#define WWHD_MSG_PRINT_POS_MAX 0x14Fu

/**
 * [V] The talk box. A PREFIX: the total size is unknown, so this may be
 * pointed at but never embedded or allocated.
 *
 * Read out of the nine message state functions, which all take it in r3.
 */
typedef struct dMsgBox_c {
    /* 0x000 */ u8  _unk_000[0x18];             /* [?] process header; a
                                                 *     vtable sits at +0x04  */
    /* 0x018 */ u8  mStateMgr[0x14];            /* [V] the state machine, five
                                                 *     words, driven by the
                                                 *     change-state routine at
                                                 *     USA 0x020063C0 and read
                                                 *     by USA 0x02006478      */
    /* 0x02C */ u8  _unk_02C[0x50 - 0x2C];      /* [?] */
    /* 0x050 */ u32 mPrintFlags;                /* [V] the typing word, read
                                                 *     only by the print step:
                                                 *     WWHD_MSGBOX_PRINT_SKIP
                                                 *     finishes the page,
                                                 *     WWHD_MSGBOX_PRINT_FAST
                                                 *     scales the rate         */
    /* 0x054 */ u32 mInputFlags;                /* [V] the button word, read by
                                                 *     the waiting states.
                                                 *     WWHD_MSGBOX_DECIDE,
                                                 *     WWHD_MSGBOX_CANCEL, and
                                                 *     the 0x10000..0x200000
                                                 *     group moves a choice
                                                 *     cursor. NOT the same
                                                 *     word as mPrintFlags     */
    /* 0x058 */ u32 _unk_058;                   /* [?] */
    /* 0x05C */ s32 mMsgNo;                     /* [P] compared against literal
                                                 *     message numbers (0x5AC,
                                                 *     0x1CFA) by two states   */
    /* 0x060 */ u8  _unk_060[0x84 - 0x60];      /* [?] */
    /* 0x084 */ s32 mSendCount;                 /* [V] bumped on every advance
                                                 *     and mirrored into play
                                                 *     + 0x4932               */
    /* 0x088 */ u8  _unk_088[0x90 - 0x88];      /* [?] */
    /* 0x090 */ s32 mPrintPos;                  /* [V] characters printed so
                                                 *     far, clamped to
                                                 *     mLineLimit and never
                                                 *     past
                                                 *     WWHD_MSG_PRINT_POS_MAX */
    /* 0x094 */ f32 mPrintFrac;                 /* [V] sub-character remainder
                                                 *     the print step carries  */
    /* 0x098 */ s32 mLineNext[10];              /* [V] < 0 on the last line;
                                                 *     that is how a state
                                                 *     knows whether to turn
                                                 *     the page or finish     */
    /* 0x0C0 */ s32 mLineLimit[10];             /* [P] per-line print budget,
                                                 *     read by the typing
                                                 *     state the same way      */
    /* 0x0E8 */ s32 mLine;                      /* [V] index into both arrays */
    /* 0x0EC */ u8  _unk_0EC[0xF4 - 0xEC];      /* [?] */
    /* 0x0F4 */ s32 mChoiceCursor;              /* [V] the highlighted option;
                                                 *     what a choice box
                                                 *     commits on a press     */
    /* 0x0F8 */ u8  _unk_0F8[0x118 - 0xF8];     /* [?] */
    /* 0x118 */ wwhd_gptr_t mpData;             /* [V] dMsgData_c*            */
} dMsgBox_c;
WWHD_ASSERT_OFFSET(dMsgBox_c, mStateMgr,     0x018);
WWHD_ASSERT_OFFSET(dMsgBox_c, mPrintFlags,   0x050);
WWHD_ASSERT_OFFSET(dMsgBox_c, mInputFlags,   0x054);
WWHD_ASSERT_OFFSET(dMsgBox_c, mMsgNo,        0x05C);
WWHD_ASSERT_OFFSET(dMsgBox_c, mSendCount,    0x084);
WWHD_ASSERT_OFFSET(dMsgBox_c, mPrintPos,     0x090);
WWHD_ASSERT_OFFSET(dMsgBox_c, mPrintFrac,    0x094);
WWHD_ASSERT_OFFSET(dMsgBox_c, mLineNext,     0x098);
WWHD_ASSERT_OFFSET(dMsgBox_c, mLineLimit,    0x0C0);
WWHD_ASSERT_OFFSET(dMsgBox_c, mLine,         0x0E8);
WWHD_ASSERT_OFFSET(dMsgBox_c, mChoiceCursor, 0x0F4);
WWHD_ASSERT_OFFSET(dMsgBox_c, mpData,        0x118);

/**
 * [V] The message data the box prints from, the HD descendant of the decomp's
 * fopMsgM_msgDataProc_c.
 *
 * The size is the constructor's own allocation: USA 0x025F65F4 asks the heap for
 * 0x924 bytes and then initialises, by hand, every field named below. That is
 * also what confirms these are real members and not reads that happened to land
 * somewhere plausible.
 *
 * The auto- and hand-send flags land 0x400 above their GameCube offsets and
 * keep their adjacency, which is what identifies them. mStatus moved to the
 * front and is the copy the states actually branch on; the play byte
 * dComIfGp_getMesgStatus() reads is published from it.
 */
typedef struct dMsgData_c {
    /* 0x000 */ u32 mStatus;                     /* [V] fopMsg_MessageStatus_e */
    /* 0x004 */ u8  _unk_004[0x654 - 0x004];     /* [?] */
    /* 0x654 */ s32 mPrintLimit;                 /* [V] the constructor stores
                                                  *     WWHD_MSG_PRINT_POS_MAX
                                                  *     here                   */
    /* 0x658 */ u8  _unk_658[0x697 - 0x658];     /* [?] +0x658 is the "page not
                                                  *     finished yet" flag the
                                                  *     typing state polls     */
    /* 0x697 */ u8  mAutoSendFlag;               /* [V] decomp + 0x297; the
                                                  *     typing state sends the
                                                  *     box to AutoStop on it  */
    /* 0x698 */ u8  mHandSendFlag;               /* [V] decomp + 0x298         */
    /* 0x699 */ u8  _unk_699[0x6AC - 0x699];     /* [?] */
    /* 0x6AC */ s32 mCharWait;                   /* [V] between-character pause;
                                                  *     the print step returns 0
                                                  *     while it runs down     */
    /* 0x6B0 */ u32 _unk_6B0;                    /* [?] */
    /* 0x6B4 */ s32 mWaitTimer;                  /* [V] the auto/hand countdown;
                                                  *     AutoStop advances when
                                                  *     it reaches zero        */
    /* 0x6B8 */ s32 mSelectKind;                 /* [P] 1 when the page carries
                                                  *     a choice, which the
                                                  *     typing state tests
                                                  *     before the status      */
    /* 0x6BC */ u8  _unk_6BC[0x90D - 0x6BC];     /* [?] */
    /* 0x90D */ u8  mForceSend;                  /* [P] treated as a press by
                                                  *     the choice and close
                                                  *     states                 */
    /* 0x90E */ u8  mExternalDrive;              /* [V] set while a demo drives
                                                  *     the box - see
                                                  *     dMsgBox_isExternallyDriven */
    /* 0x90F */ u8  _unk_90F[0x924 - 0x90F];     /* [?] */
} dMsgData_c;
WWHD_ASSERT_OFFSET(dMsgData_c, mStatus,        0x000);
WWHD_ASSERT_OFFSET(dMsgData_c, mAutoSendFlag,  0x697);
WWHD_ASSERT_OFFSET(dMsgData_c, mHandSendFlag,  0x698);
WWHD_ASSERT_OFFSET(dMsgData_c, mWaitTimer,     0x6B4);
WWHD_ASSERT_OFFSET(dMsgData_c, mSelectKind,    0x6B8);
WWHD_ASSERT_OFFSET(dMsgData_c, mForceSend,     0x90D);
WWHD_ASSERT_OFFSET(dMsgData_c, mPrintLimit,    0x654);
WWHD_ASSERT_OFFSET(dMsgData_c, mCharWait,      0x6AC);
WWHD_ASSERT_OFFSET(dMsgData_c, mExternalDrive, 0x90E);
WWHD_ASSERT_SIZE  (dMsgData_c, 0x924);

/**
 * [V] The message manager. A PREFIX; the members are the ones the message-set
 * routine at USA 0x025F7980 dispatches through.
 */
typedef struct dMsgObject_c {
    /* 0x000 */ u8          _unk_000[0x938];
    /* 0x938 */ u32         mCurMsgNo;       /* [V] the message being shown    */
    /* 0x93C */ u8          _unk_93C[0x94C - 0x93C];
    /* 0x94C */ u32         mActiveBox;      /* [V] which box is up; 1 is the
                                              *     ordinary talk box          */
    /* 0x950 */ wwhd_gptr_t mpBoxItem;       /* [V] kinds 2, 3 and 4           */
    /* 0x954 */ wwhd_gptr_t mpBoxTalk;       /* [V] the box that owns the page
                                              *     wait and the choices       */
    /* 0x958 */ u8          _unk_958[0x95C - 0x958];
    /* 0x95C */ wwhd_gptr_t mpBoxTact;       /* [V] kind 0                     */
    /* 0x960 */ wwhd_gptr_t mpBoxSign;       /* [V] kind 5                     */
} dMsgObject_c;
WWHD_ASSERT_OFFSET(dMsgObject_c, mCurMsgNo,  0x938);
WWHD_ASSERT_OFFSET(dMsgObject_c, mActiveBox, 0x94C);
WWHD_ASSERT_OFFSET(dMsgObject_c, mpBoxTalk,  0x954);

/** [V] dMsgObject_c::mActiveBox, as the message-set routine assigns it. */
#define WWHD_MSGBOX_KIND_TACT 0u
#define WWHD_MSGBOX_KIND_TALK 1u
#define WWHD_MSGBOX_KIND_ITEM 2u
#define WWHD_MSGBOX_KIND_ITEM2 3u
#define WWHD_MSGBOX_KIND_ITEM3 4u
#define WWHD_MSGBOX_KIND_SIGN 5u

/* --- Accessors ---------------------------------------------------------- */

/** [V] The message manager, or NULL before it is built. */
static __inline dMsgObject_c* dMsg_getObject(void) {
    u32 p;
    if (!wwhd_regionResolved)
        return (dMsgObject_c*)0;
    p = *WWHD_AT_DATA(u32, wwhd_map->msgObject);
    return p ? WWHD_AT(dMsgObject_c, p) : (dMsgObject_c*)0;
}

/**
 * [V] Whichever box is on screen, of any kind, or NULL.
 *
 * WHAT THE KINDS SHARE, AND WHAT THEY DO NOT. Four separate box classes were
 * read out of their own message-set routines, and three members sit at the same
 * place in every one of them: the state manager at +0x18, the message number at
 * +0x5C, and **mInputFlags at +0x54**. That last is the one that matters, and
 * it is not an assumption: the talk box tests it through USA 0x026FCEC8, the
 * item box reads `0x54(r3)` directly in its own page wait at 0x026BA628, and a
 * third class does both through 0x026B8354 and 0x026B836C. So the decide bit
 * means the same thing on any box this returns.
 *
 * mPrintFlags at +0x50 and the dMsgData_c layout are only verified against the
 * talk box, so a caller wanting either must go through
 * dMsg_activeBoxHasTalkLayout() and dMsg_getActiveBoxData() rather than
 * dMsgBox_getData(), because the data POINTER is at a different offset in every
 * class - see below.
 */
static __inline dMsgBox_c* dMsg_getActiveBox(void) {
    dMsgObject_c* m = dMsg_getObject();
    wwhd_gptr_t p;
    if (!m)
        return (dMsgBox_c*)0;
    switch (m->mActiveBox) {
    case WWHD_MSGBOX_KIND_TACT:  p = m->mpBoxTact; break;
    case WWHD_MSGBOX_KIND_TALK:  p = m->mpBoxTalk; break;
    case WWHD_MSGBOX_KIND_ITEM:
    case WWHD_MSGBOX_KIND_ITEM2:
    case WWHD_MSGBOX_KIND_ITEM3: p = m->mpBoxItem; break;
    case WWHD_MSGBOX_KIND_SIGN:  p = m->mpBoxSign; break;
    default:                     return (dMsgBox_c*)0;
    }
    return p ? WWHD_AT(dMsgBox_c, p) : (dMsgBox_c*)0;
}

/**
 * [V] Non-zero when the box on screen keeps its data pointer at +0x118 - kinds
 * 0 and 1, whose message-set routines (USA 0x026B3F18 and 0x026B699C) both read
 * it there. This says nothing about any other member.
 */
static __inline int dMsg_activeBoxHasTalkLayout(void) {
    dMsgObject_c* m = dMsg_getObject();
    return m && (m->mActiveBox == WWHD_MSGBOX_KIND_TACT ||
                 m->mActiveBox == WWHD_MSGBOX_KIND_TALK);
}

/**
 * [V] Non-zero only for the talk box, the one class whose +0x50 is known to be
 * mPrintFlags. NOTHING ELSE MAY HAVE THAT WORD WRITTEN.
 *
 * The print step at wwhd_map->dMsg_printStep reads `0x50(r3)` and is reached
 * from the talk box's typing state; no other class was shown to use the word
 * for anything, which is not the same as showing it is unused. Kind 0 in
 * particular is a different class - its own fourteen-state block at 0x1049839C
 * has no page wait, its descriptors carry a different vtable from the talk
 * box's, and its states gate on animations through 0x026FF668
 * (`FUN_02005840(this->+0x44->+0xD4, mode)`) rather than on any button. Sharing
 * a data-pointer offset with the talk box says nothing about +0x50, and a build
 * that assumed otherwise set bit 1 there every frame on a class that never asked
 * for it.
 */
static __inline int dMsg_activeBoxHasPrintFlags(void) {
    dMsgObject_c* m = dMsg_getObject();
    return m && m->mActiveBox == WWHD_MSGBOX_KIND_TALK;
}

/**
 * [V] The message data behind whichever box is up, whatever kind it is.
 *
 * EVERY CLASS PUTS THIS POINTER SOMEWHERE ELSE, which is the trap this exists
 * to close: dMsgBox_getData() reads +0x118 and is right only for kinds 0 and 1.
 * Each offset below was read out of that kind's own message-set routine, which
 * is where the box is first handed its data:
 *
 *     kind 0, 1   +0x118   0x026B3F18, 0x026B699C
 *     kind 2,3,4  +0x138   0x026BCBF0
 *     kind 5      +0x8C    0x026AE194, and 0x026AE330 writes data + 0x688
 *
 * The object itself is the same dMsgData_c in every case - the item box writes
 * its +0x69C, the sign box its +0x688, another class its +0x919 and +0x91D, all
 * inside the 0x924 the constructor allocates - so what a caller does with the
 * result is the same regardless of which box handed it over.
 */
static __inline dMsgData_c* dMsg_getActiveBoxData(void) {
    dMsgObject_c* m = dMsg_getObject();
    const dMsgBox_c* box = dMsg_getActiveBox();
    u32 ofs;
    wwhd_gptr_t p;
    if (!m || !box)
        return (dMsgData_c*)0;
    switch (m->mActiveBox) {
    case WWHD_MSGBOX_KIND_TACT:
    case WWHD_MSGBOX_KIND_TALK:  ofs = 0x118u; break;
    case WWHD_MSGBOX_KIND_ITEM:
    case WWHD_MSGBOX_KIND_ITEM2:
    case WWHD_MSGBOX_KIND_ITEM3: ofs = 0x138u; break;
    case WWHD_MSGBOX_KIND_SIGN:  ofs = 0x08Cu; break;
    default:                     return (dMsgData_c*)0;
    }
    p = *(const wwhd_gptr_t*)((const u8*)box + ofs);
    return p ? WWHD_AT(dMsgData_c, p) : (dMsgData_c*)0;
}

/** [V] The talk box specifically, or NULL when another kind is up. */
static __inline dMsgBox_c* dMsg_getTalkBox(void) {
    dMsgObject_c* m = dMsg_getObject();
    if (!m || m->mActiveBox != WWHD_MSGBOX_KIND_TALK || !m->mpBoxTalk)
        return (dMsgBox_c*)0;
    return WWHD_AT(dMsgBox_c, m->mpBoxTalk);
}

/** [V] Which kind of box is up; only meaningful while one is. */
static __inline u32 dMsg_getBoxKind(void) {
    dMsgObject_c* m = dMsg_getObject();
    return m ? m->mActiveBox : 0xFFFFFFFFu;
}

/**
 * [V] The status of the message box on screen, or fopMsgStts_NONE_e when there
 * is none and when no region has been selected.
 */
static __inline u8 dComIfGp_getMesgStatus(void) {
    const u8* p = (const u8*)dComIfGp_playAt(WWHD_PLAY_MESG_STATUS);
    return p ? *p : (u8)fopMsgStts_NONE_e;
}

/**
 * [V] The status of the box that is actually on screen, taken from its own data
 * object rather than from play.
 *
 * PREFER THIS TO dComIfGp_getMesgStatus() WHEN A BOX IS IN HAND. The play byte
 * is a copy, published by whichever state last called setMesgStatus, and the
 * states themselves do not branch on it - the typing state reads `*data`, its
 * own mStatus, and so does everything else that decides where to go next. The
 * two agree for the talk box, whose states publish diligently; they need not
 * agree for a box of another kind, and a stale copy is indistinguishable from a
 * live one at the play byte.
 *
 * Falls back to the published byte when no box this header understands is up,
 * which is what a caller wants for "is anything on screen".
 */
static __inline u8 dMsg_getBoxStatus(void) {
    const dMsgData_c* d = dMsg_getActiveBoxData();
    return d ? (u8)d->mStatus : dComIfGp_getMesgStatus();
}

/** [V] A box is on screen in some state. */
static __inline int dMsg_isBoxUp(void) {
    return dComIfGp_getMesgStatus() != (u8)fopMsgStts_NONE_e;
}

/**
 * [V] The box has finished a page and is waiting for the player to turn it.
 *
 * True for ::StateID_Stop and ::StateID_HandStop, whose only answer to a press
 * is the next page.
 */
static __inline int dMsg_isPageWait(void) {
    return dComIfGp_getMesgStatus() == (u8)fopMsgStts_STOP_e;
}

/**
 * [V] The box is printing a page. The typing state consults no button of its
 * own; the print step at wwhd_map->dMsg_printStep is the only thing that
 * decides how fast, or whether to finish the page at once.
 */
static __inline int dMsg_isTyping(void) {
    return dComIfGp_getMesgStatus() == (u8)fopMsgStts_MSG_TYPING_e;
}

/**
 * [V] The box has said its last line and is waiting for the player before it
 * goes away. Reached from the page wait when the line table runs out, which is
 * why a conversation stalls on its final box for anything that advances pages
 * but not this.
 *
 * Two states run here, ::StateID_NpcAnswerWait and
 * ::StateID_SelectNpcAnswerWait, and neither decides anything: the first
 * continues, the second is what a choice box moves to once the choice is
 * already committed.
 */
static __inline int dMsg_isAnswerWait(void) {
    return dComIfGp_getMesgStatus() == (u8)fopMsgStts_MSG_DISPLAYED_e;
}

/**
 * [V] The box is finished and waiting for the player to close it.
 *
 * True for the three ::StateID_*CloseWait states.
 */
static __inline int dMsg_isCloseWait(void) {
    return dComIfGp_getMesgStatus() == (u8)fopMsgStts_CLOSE_WAIT_e;
}

/**
 * [V] THE BOX IS ASKING A QUESTION. A press here commits the highlighted
 * option, so anything that supplies presses on the player's behalf must test
 * this and stop.
 *
 * The choice states run at 8 and 9 - not inferred, but read out of the
 * choice-in state (USA 0x026FD8AC), which branches on the data status: 8 goes
 * to ::StateID_Select2 and 9 to ::StateID_Select3. The typing state tests
 * mSelectKind before it tests the status, so a page carrying a choice never
 * publishes fopMsgStts_STOP_e at any point on the way in. 0x14 is the
 * horizontal box, carried over from the decomp and unconfirmed here, and is
 * included because a false negative costs a purchase nobody asked for.
 */
static __inline int dMsg_isChoiceWait(void) {
    const u8 s = dComIfGp_getMesgStatus();
    return s == (u8)fopMsgStts_SELECT_2_e ||
           s == (u8)fopMsgStts_SELECT_3_e ||
           s == (u8)fopMsgStts_SELECT_YOKO_e;
}

/** [V] The box is taking a number. A press commits the entry; see above. */
static __inline int dMsg_isInputWait(void) {
    return dComIfGp_getMesgStatus() == (u8)fopMsgStts_INPUT_e;
}

/**
 * [V] The message is finished and the box is waiting to be told to fetch the
 * next one. THIS WAITS ON THE BUTTON like the page wait does.
 *
 * Its state (USA 0x026FEAF4) is the decomp's dMsg_continueProc, and the match is
 * exact rather than by shape - all three of its conditions line up:
 *
 *     CPad_CHECK_TRIG_A/B                -> isDecidePressed(this)
 *     mMsgNo == 0x5ac && checkMesgCancel -> mMsgNo == 0x5AC && play + 0x4933
 *     && !dComIfGp_checkMesgBgm()        -> && play + 0x4980 == 0
 *
 * the literal 0x5AC being what settles it. A held button that skips this status
 * leaves a conversation sitting between two of its messages for as long as the
 * player holds on, which is exactly what it did before this was identified.
 *
 * Answering it is not a decision: the decomp's proc clears the select flag and
 * loads the next message. Note the two extra facts that fall out of the match -
 * play + 0x4933 is the mesg cancel button and play + 0x4980 the mesg BGM flag,
 * and while that BGM flag is set the box ignores the button on purpose.
 */
static __inline int dMsg_isContinuing(void) {
    return dComIfGp_getMesgStatus() == (u8)fopMsgStts_MSG_CONTINUES_e;
}

/**
 * [V] The box has said everything and is on its way out.
 *
 * Observed on hardware: a box whose message ends rather than pausing publishes
 * 0x10 and waits there for the advance button, so anything that advances pages
 * but not this stalls on the last box of a conversation. The waiting state
 * consults the same button check as the rest.
 */
static __inline int dMsg_isEnding(void) {
    return dComIfGp_getMesgStatus() == (u8)fopMsgStts_MSG_ENDS_e;
}

/** [V] The message data behind a box, or NULL. */
static __inline dMsgData_c* dMsgBox_getData(const dMsgBox_c* box) {
    if (!box || !box->mpData)
        return (dMsgData_c*)0;
    return WWHD_AT(dMsgData_c, box->mpData);
}

/**
 * [V] A DEMO IS DRIVING THIS BOX. NOTHING MAY ANSWER "PRESSED" FOR IT.
 *
 * The waiting states are written so the demo's path is taken only when the
 * player has NOT pressed. The page wait reads:
 *
 *     if (!isDecidePressed(this) && !(mInputFlags & CANCEL)) {
 *         if (data->mExternalDrive) {          // the demo's own route
 *             setMesgStatus(MSG_PREPARING);
 *             changeState(KeyIconOutAnimEndWaitToOutNow);
 *             return;
 *         }
 *         ...idle, bounce the arrow...
 *     }
 *     ...the player's route: KeyIconDecideAnimEndWaitToOutNow...
 *
 * and the answer wait tests the press first and mExternalDrive second, the same
 * way round, so a synthetic press takes such a box down the player's branch
 * rather than the demo's.
 *
 * WHETHER THAT MATTERS IS UNPROVEN, and this predicate is currently unused. Two
 * crashes were once blamed on it; both turned out to be the calling convention
 * described on dMsg_decideCheck, and a real player pressing during a cutscene
 * takes the same branch a synthetic press does. It is kept because the asymmetry
 * in the states is real and worth knowing about, not because anything needs it.
 */
static __inline int dMsgBox_isExternallyDriven(const dMsgBox_c* box) {
    const dMsgData_c* d = dMsgBox_getData(box);
    return d && d->mExternalDrive != 0;
}

/**
 * [V] Hand the box to the game's OWN auto-advance, this page and the next.
 *
 * This is not a synthetic button press; it is the flag a message's own
 * auto-send tag sets, and it takes three shortcuts the button cannot:
 *
 *   the typing state, seeing it at status 0x07, changes to ::StateID_AutoStop
 *   instead of ::StateID_KeyIconInAnimEndWaitToStop, so the button-prompt
 *   animation in front of the page wait never plays;
 *
 *   ::StateID_AutoStop advances the moment mWaitTimer reaches zero, which is
 *   why this zeroes it, rather than waiting on a press at all;
 *
 *   the continue state, seeing it at status 0x10, changes straight to
 *   ::StateID_Close instead of going through the close-wait pair.
 *
 * ::StateID_AutoStop clears the flag itself when it fires, so a caller holding
 * a button must set this every frame; it is never cleared here, because a
 * message that asked for auto-send on its own must keep it.
 *
 * Choices are unaffected: the typing state tests mSelectKind before it looks at
 * the status, so a page carrying a choice takes the select path either way.
 *
 * Both fields are allocated and initialised by the constructor that sizes this
 * object at 0x924, so this writes real members. It was once suspected of a
 * crash; the constructor is what cleared it, and the real cause was the calling
 * convention described on dMsg_decideCheck.
 */
static __inline void dMsg_armAutoSend(dMsgData_c* data) {
    if (!data)
        return;
    data->mAutoSendFlag = 1;
    data->mWaitTimer = 0;
}

/**
 * [V] Did the player press the advance button, as the message states ask it.
 *
 * NOTE THIS IS A TRIGGER, NOT A LEVEL. The bit is set on the frame the button
 * goes down, so a caller supplying presses on the player's behalf must supply
 * one every frame it wants the box to move; holding a real button produces a
 * single edge and the box waits after it.
 *
 * The body of the function at wwhd_map->dMsg_decideCheck, which is worth having
 * in C because a hook on that address may find itself with no original to call.
 */
static __inline int dMsgBox_isDecidePressed(const dMsgBox_c* box) {
    return box && (box->mInputFlags & 1u) != 0u;
}

/**
 * [V] The shared UI input record every message box copies its buttons from, or
 * NULL before it exists.
 *
 * Two loads from wwhd_map->uiDisplayMgr: that word holds a holder, and word 0
 * of the holder is the record. Word 0x00 is the button mask that becomes
 * dMsgBox_c::mInputFlags, word 0x08 an auxiliary mask OR'd into the box's
 * +0x58, and word 0x43 the print flags that become dMsgBox_c::mPrintFlags.
 *
 * Every box reads the same record, and so does every other screen driven by the
 * same UI input. A write here therefore reaches all of them - which is the point:
 * the five box updaters do not share one latch, and four of them inline the copy
 * where no hook can reach it (see wwhd_map dMsgBox_setInput). Writing the record
 * is the only lever that drives ordinary dialogue and cutscene text alike.
 *
 * The cost is that it is genuinely global: any other screen reading the same
 * record that frame sees the button too. Gate it on a live message box, and set
 * it only for the frames a press is actually wanted.
 */
static __inline u32* dMsg_getInputRecord(void) {
    u32 holder;
    if (!wwhd_regionResolved)
        return (u32*)0;
    holder = *WWHD_AT_DATA(u32, wwhd_map->uiDisplayMgr);
    if (!holder)
        return (u32*)0;
    return *WWHD_AT(u32*, holder);
}

/**
 * [V] Supply one frame of the advance button to a single box.
 *
 * dMsgBox_c::mInputFlags is NOT accumulated. The box's input latch overwrites the
 * whole word from the shared record immediately before the box's state manager
 * runs, with nothing in between, so a press written anywhere else in the frame is
 * gone before any state can read it. The only window that works is between that
 * copy and the state execute, which is why the intended caller is a replacement
 * of wwhd_map->dMsgBox_setInput that calls the original first and then sets this.
 *
 * That reaches the mActiveBox 0 and 1 boxes only. The other updaters inline the
 * copy, so this cannot be made to cover them - for a cutscene box, or for all
 * classes at once, write dMsg_getInputRecord()[0] instead.
 *
 * The bit is a trigger, not a level, so a caller holding a button must call
 * this every frame; see dMsgBox_isDecidePressed.
 */
static __inline void dMsgBox_pressDecide(dMsgBox_c* box) {
    if (!box)
        return;
    box->mInputFlags |= WWHD_MSGBOX_DECIDE;
}

/**
 * [V] Ask the typing state to finish the page this frame.
 *
 * dMsgBox_c::mPrintFlags is overwritten by the same copy as mInputFlags and
 * carries the same timing rule, so this belongs in the same window. Bit 1 sends
 * mPrintPos straight to WWHD_MSG_PRINT_POS_MAX; bit 0 only multiplies the rate.
 *
 * Only the talk box lays its data out where the print step expects it, so guard
 * with dMsg_activeBoxHasPrintFlags(): on the item and sign boxes the word at
 * +0x50 belongs to something else.
 */
static __inline void dMsgBox_skipTyping(dMsgBox_c* box) {
    if (!box)
        return;
    box->mPrintFlags |= WWHD_MSGBOX_PRINT_SKIP;
}

#ifdef WWHD_ENABLE_GAME_CALLS
/**
 * [V] The name of the state a box is currently in - "::StateID_Stop" and the
 * rest - or NULL.
 *
 * Every state descriptor carries its name at +0x04, and wwhd_map->fStateMgr_getState
 * hands back the descriptor of whichever state is live, so this turns a box into
 * the one piece of information the status byte cannot give: WHICH state is
 * waiting. The status is shared between classes and says only what kind of wait
 * it is; the name says which function is doing the waiting, which is what a
 * caller needs before it tries to hurry one along.
 *
 * It makes an indirect call through the live state, so hand it a box that came
 * from dMsg_getActiveBox() and call it on the game's own thread.
 */
static __inline const char* dMsgBox_getStateName(const dMsgBox_c* box) {
    typedef const void* (*getState_t)(const void* mgr);
    getState_t getState;
    const void* desc;
    if (!box || !wwhd_textResolved || !wwhd_regionResolved)
        return (const char*)0;
    getState = WWHD_FN(getState_t, wwhd_map->fStateMgr_getState);
    desc = getState((const u8*)box + 0x18);
    if (!desc)
        return (const char*)0;
    return *(const char* const*)((const u8*)desc + 4);
}
#endif /* WWHD_ENABLE_GAME_CALLS */

#endif /* LIBWWHD_D_MSG_H */
