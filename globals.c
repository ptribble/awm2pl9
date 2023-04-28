


#ifndef lint
static char *rcsid_globals_c = "$Header: /usr/graph2/X11.3/contrib/windowmgrs/awm/RCS/globals.c,v 1.2 89/02/07 21:24:58 jkh Exp $";
#endif  lint
 
/*
 *
 * Copyright 1987, 1988 by Ardent Computer Corporation, Sunnyvale, Ca.
 *
 * Copyright 1987 by Jordan Hubbard.
 *
 *
 *                         All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Ardent Computer
 * Corporation or Jordan Hubbard not be used in advertising or publicity
 * pertaining to distribution of the software without specific, written
 * prior permission.
 *
 */

/*
 * Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.
 *
 *			   All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Digital Equipment
 * Corporation not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 *
 *
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

 
/*
 * MODIFICATION HISTORY
 *
 * 000 -- M. Gancarz, DEC Ultrix Engineering Group
 * 001 -- R. Kittell, DEC Storage A/D May 19, 1986
 *  Added global vars for warp options.
 * 002 -- Loretta Guarino Reid, DEC Ultrix Engineering Group 
 *  Western Software Lab. April 17, 1987
 *  Convert to X11
 * 003 -- Jordan Hubbard, U.C. Berkeley. Misc new vars.
 * 1.4 -- Vars added for new icon stuff.
 */
 
#include "awm.h"
#include "X11/Xutil.h"
 
Window Pop;			/* Pop up dimension display window. */
Window Frozen;			/* Contains window id of "gridded" window. */
XFontStruct *IFontInfo;		/* Icon text font information. */
XFontStruct *PFontInfo;		/* Pop-up text font information. */
XFontStruct *TFontInfo;		/* Title text font information. */
XFontStruct *TFontBoldInfo;	/* Title text (bold) font information. */
XFontStruct *GFontInfo;		/* Gadget box text font */
Pixmap GrayPixmap;		/* Gray pixmap. */
Pixmap SolidPixmap;		/* Solid pixmap */
Pixmap IBackPixmap;		/* Icon window background pixmap. */
Pixmap IDefPixmap;		/* default icon pixmap for IconLabels */
Pixel ForeColor;		/* Generic foreground color */
Pixel BackColor;		/* Generic background color */
Pixel IBorder;			/* Icon window border color. */
Pixel PBackground;		/* Pop-up Window background color. */
Pixel PForeground;		/* Pop-up Window foregrould color */
Pixel PBorder;			/* Pop-Up Window border color. */
Pixel IBackground;		/* Icon background (for pixmap) color */
Pixel IForeground;		/* Icon foreground (for pixmap) color */
Pixel ITextForeground;		/* Icon text foreground color */
Pixel ITextBackground;		/* Icon text background color */
char *Foreground;		/* foreground color (text) */
char *Background;		/* background color (text) */
char *WBorder;			/* Window border color */
char *BBackground;		/* Border context background color */
char *BForeground;		/* Border context foreground color */
char *TBackground;		/* Titlebar background color */
char *TForeground;		/* Titlebar foreground color */
char *TTextBackground;		/* Titlebar text background color */
char *TTextForeground;		/* Titlebar text foreground color */
Cursor ArrowCrossCursor;	/* Arrow cross cursor. */
Cursor TextCursor;		/* Text cursor used in icon windows. */
Cursor IconCursor;		/* Icon Cursor. */
Cursor LeftButtonCursor;	/* Left button main cursor. */
Cursor MiddleButtonCursor;	/* Middle button main cursor. */
Cursor RightButtonCursor;	/* Right button main cursor. */
Cursor TargetCursor;		/* Target (select-a-window) cursor. */
Cursor TitleCursor;		/* Titlebar cursor */
Cursor FrameCursor;		/* Border context cursor */
Cursor GumbyCursor;		/* Used in icons if not type-in */
unsigned int GadgetBorder;	/* Width of gadget borders */
int ScreenWidth;		/* Display screen width. */
int ScreenHeight;		/* Display screen height. */
int IBorderWidth;		/* Icon window border width. */
int TitleHeight;		/* Height of title bar(s) (in pixels) */
int titleHeight;		/* Derived height of title bar(s) in pixels */
int gadgetHeight;		/* Height of highest gadget */
int NameOffset;			/* Offset for window name */
int TitlePad;			/* Padding for titles */
int PWidth;			/* Pop-up window width. */
int PHeight;			/* Pop-up window height. */
unsigned int PBorderWidth;	/* Pop-up window border width. */
int PPadding;			/* Pop-up window padding. */
int Delta;			/* Mouse movement slop. */
int HIconPad;			/* Icon horizontal padding. */
int VIconPad;			/* Icon vertical padding. */
int Pushval;			/* Number of pixels to push window by. */
int BContext;			/* Border context area width (in pixels) */
int RaiseDelay;			/* Number of milliseconds delay before autoraise */
int NumGadgets;			/* Number of gadgets we're using */
int GadgetPad;			/* Amount of padding for gadgets */
int Volume;			/* Audible alarm volume. */
int status;			/* Routine return status. */
unsigned int BCursor;		/* Border context cursor */
unsigned int TCursor;		/* Title context cursor */
MenuLink *Menus;		/* Linked list of menus. */
GC  IconGC;			/* graphics context for icon */
GC  PopGC;			/* graphics context for pop */
GC  DrawGC;			/* graphics context for drawing */
Binding *Blist;			/* Button/key binding list. */

XContext AwmContext;		/* Main context for awm */

Boolean Autoselect;		/* Warp mouse to default menu selection? */
Boolean Autoraise;		/* Raise window on input focus? */
Boolean Borders;		/* Display border context areas? */
Boolean ConstrainResize;	/* Don't resize until pointer leaves window */
Boolean Freeze;			/* Freeze server during move/resize? */
Boolean Grid;			/* Should the m/r box contain a 9 seg. grid. */
Boolean ShowName;		/* If True, print window names in titlebars */
Boolean Hilite;			/* Should we highlight titles on focus? */
Boolean BorderHilite;		/* Should we highlight borders on focus? */
Boolean InstallColormap;	/* Should we install colormaps for clients? */
Boolean NWindow;		/* Normalize windows? */
Boolean NIcon;			/* Normalize icons? */
Boolean Push;			/* Relative=TRUE, Absolute=FALSE. */
Boolean RootResizeBox;		/* Resize window is placed over sized window? */
Boolean ResizeRelative;		/* resizes should be relative to window edge */
Boolean Titles;			/* Title bar frob on windows? */
Boolean IconLabels;		/* icon labels? */
Boolean ILabelTop;		/* icon label on top? */
Boolean UseGadgets;		/* Gadget boxes on titles? */
Boolean FrameFocus;		/* Treat the frame as part of the window? */
Boolean Reverse;		/* Reverse video? */
Boolean SaveUnder;		/* Save unders? */
Boolean PushDown;		/* Down=TRUE, Up=FALSE */
Boolean Wall;			/* restrict to root window boundries? */
Boolean WarpOnRaise;		/* Warp to upper right corner on raise. */
Boolean WarpOnIconify;          /* Warp to icon center on iconify. */
Boolean WarpOnDeIconify;        /* Warp to upper right corner on de-iconify. */
Boolean FocusSetByUser;         /* True if f.focus called */
Boolean FocusSetByWM;		/* True if awm set the focus */
Boolean Zap;			/* Should the the zap effect be used. */
 
char PText[7] = INIT_PTEXT;	/* Pop-up window dummy text. */
int PTextSize = sizeof(PText);	/* Pop-up window dummy text size. */
 
int Lineno = 1;			/* Line count for parser. */
Boolean Startup_File_Error = FALSE;/* Startup file error flag. */
char Startup_File[NAME_LEN] = "";/* Startup file name. */
char *TBackPixmapData;		/* Bitmap data file title background */
char *TBoldPixmapData;		/* ditto, except highlighted */
char *BBackPixmapData;		/* Border context background pixmap data */
char *BBoldPixmapData;		/* Border context bold pixmap data */
char *awmPath;			/* Optional pathlist to search for pixmaps */
char **Argv;			/* Pointer to command line parameters. */
char **Environ;			/* Pointer to environment. */

char gray_bits[] = {
     0x55, 0x55, 0xaa, 0xaa, 0x55, 0x55, 0xaa, 0xaa,
     0x55, 0x55, 0xaa, 0xaa, 0x55, 0x55, 0xaa, 0xaa,
     0x55, 0x55, 0xaa, 0xaa, 0x55, 0x55, 0xaa, 0xaa,
     0x55, 0x55, 0xaa, 0xaa, 0x55, 0x55, 0xaa, 0xaa
};

char solid_bits[] = {
     0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
     0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
     0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
     0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

char xlogo32_bits[] = {
   0xff, 0x00, 0x00, 0xc0, 0xfe, 0x01, 0x00, 0xc0, 0xfc, 0x03, 0x00, 0x60,
   0xf8, 0x07, 0x00, 0x30, 0xf8, 0x07, 0x00, 0x18, 0xf0, 0x0f, 0x00, 0x0c,
   0xe0, 0x1f, 0x00, 0x06, 0xc0, 0x3f, 0x00, 0x06, 0xc0, 0x3f, 0x00, 0x03,
   0x80, 0x7f, 0x80, 0x01, 0x00, 0xff, 0xc0, 0x00, 0x00, 0xfe, 0x61, 0x00,
   0x00, 0xfe, 0x31, 0x00, 0x00, 0xfc, 0x33, 0x00, 0x00, 0xf8, 0x1b, 0x00,
   0x00, 0xf0, 0x0d, 0x00, 0x00, 0xf0, 0x0e, 0x00, 0x00, 0x60, 0x1f, 0x00,
   0x00, 0xb0, 0x3f, 0x00, 0x00, 0x98, 0x7f, 0x00, 0x00, 0x98, 0x7f, 0x00,
   0x00, 0x0c, 0xff, 0x00, 0x00, 0x06, 0xfe, 0x01, 0x00, 0x03, 0xfc, 0x03,
   0x80, 0x01, 0xfc, 0x03, 0xc0, 0x00, 0xf8, 0x07, 0xc0, 0x00, 0xf0, 0x0f,
   0x60, 0x00, 0xe0, 0x1f, 0x30, 0x00, 0xe0, 0x1f, 0x18, 0x00, 0xc0, 0x3f,
   0x0c, 0x00, 0x80, 0x7f, 0x06, 0x00, 0x00, 0xff};

int scr;
Display *dpy;

#ifdef WMSTATE
Atom wm_state_atom;
#endif

#ifdef NEATEN
int AbsMinWidth;
int AbsMinHeight;
Boolean RetainSize;
Boolean KeepOpen;
Boolean Fill;
Boolean UsePriorities;
Boolean FixTopOfStack;
char *PrimaryIconPlacement;
char *SecondaryIconPlacement;
#endif

/*
 * Keyword lookup table for parser.
 */
Keyword KeywordTable[] =
{
    { "autoselect",	IsBoolean,	&Autoselect, 0 },
    { "autoraise",	IsBoolean,	&Autoraise, 0 },
    { "freeze",		IsBoolean,	&Freeze, 0 },
    { "hilite",		IsBoolean,	&Hilite, 0 },
    { "rootResizeBox",	IsBoolean,	&RootResizeBox, 0 },
    { "titles",		IsBoolean,	&Titles, 0 },
    { "gadgets",	IsBoolean,	&UseGadgets, 0 },
    { "grid",		IsBoolean,	&Grid, 0 },
    { "showName",	IsBoolean,	&ShowName, 0,},
    { "normali",	IsBoolean,	&NIcon, 0 },
    { "normalw",	IsBoolean,	&NWindow, 0 },
    { "pushRelative",	IsBoolean,	&Push, 0 },
    { "reverse",	IsBoolean,	&Reverse, 0 },
    { "wall",		IsBoolean,	&Wall, 0 },
    { "frameFocus",	IsBoolean,	&FrameFocus, 0 },
    { "warpOnIconify",	IsBoolean,	&WarpOnIconify, 0 },
    { "warpOnDeIconify",IsBoolean,	&WarpOnDeIconify, 0 },
    { "warpOnRaise",	IsBoolean,	&WarpOnRaise, 0 },
#ifdef NEATEN
    { "retainSize",	IsBoolean,	&RetainSize, 0 },
    { "keepOpen",	IsBoolean,	&KeepOpen, 0 },
    { "fill",		IsBoolean,	&Fill, 0 },
    { "usePriorities",	IsBoolean,	&UsePriorities },
    { "fixTopOfStack",	IsBoolean,	&FixTopOfStack },
#endif
    { "zap",		IsBoolean,	&Zap, 0 },
    { "gadget",		IsGadget,	0, 0 },
    { "f.beep",		IsImmFunction,	0, Beep },
    { "f.circledown",	IsImmFunction,	0, CircleDown },
    { "f.circleup",	IsImmFunction,	0, CircleUp },
    { "f.continue",	IsImmFunction,	0, Continue },
    { "f.decorate",	IsFunction,	0, FDecorate },
    { "f.exit",		IsQuitFunction,	0, Quit },
    { "f.focus",	IsFunction,	0, Focus },
    { "f.unfocus",	IsImmFunction,	0, UnFocus },
    { "f.iconify",	IsFunction,	0, Iconify },
    { "f.lower",	IsFunction,	0, Lower },
    { "f.menu",		IsMenuMap,	0, DoMenu },
    { "f.action",	IsAction,	0, DoAction },
    { "f.move",		IsDownFunction,	0, Move },
    { "f.moveopaque",	IsDownFunction,	0, MoveOpaque },
    { "f.newiconify",	IsDownFunction,	0, NewIconify },
    { "f.neaten",	IsImmFunction,	0, Neaten },
    { "f.pause",	IsImmFunction,	0, Pause },
    { "f.pushdown",	IsFunction,	0, ShoveDown },
    { "f.pushleft",	IsFunction,	0, ShoveLeft },
    { "f.pushright",	IsFunction,	0, ShoveRight },
    { "f.pushup",	IsFunction,	0, ShoveUp },
    { "f.raise",	IsFunction,	0, Raise },
    { "f.redraw",	IsDownFunction,	0, Redraw },
    { "f.refresh",	IsImmFunction,	0, Refresh },
    { "f.resize",	IsDownFunction,	0, Resize },
    { "f.restart",	IsQuitFunction,	0, Restart },
    { "f.destroy",      IsDownFunction, 0, DestroyClient },
    { "f.nodecorate",   IsFunction,     0, FNoDecorate },
    { "f.lock",   	IsImmFunction,  0, Lock },
    { "menu",		IsMenu,		0, 0 },
    { "resetbindings",	IsParser,	0, ResetBindings },
    { "resetmenus",	IsParser,	0, ResetMenus },
    { "resetgadgets",	IsParser,	0, ResetGadgets },
    { NULL,		NULL,		NULL, NULL }
};
 
/*
 * Key expression table for parser.
 */
KeyExpr KeyExprTbl[] = {
    { "ctrl", ControlMask },
    { "c", ControlMask },
    { "lock", LockMask },
    { "l", LockMask },
    { "meta", Mod1Mask },
    { "m", Mod1Mask },
    { "shift", ShiftMask },
    { "s", ShiftMask },
    { "mod1", Mod1Mask },
    { "mod2", Mod2Mask },
    { "mod3", Mod3Mask },
    { "mod4", Mod4Mask },
    { "mod5", Mod5Mask },
    { NULL, NULL }
};
 
/*
 * Context expression table for parser.
 */
ContExpr ContExprTbl[] = {
    { "icon", ICON },
    { "i", ICON },
    { "root", ROOT },
    { "r", ROOT },
    { "window", WINDOW },
    { "w", WINDOW },
    { "t", TITLE },
    { "title", TITLE },
    { "g", GADGET },
    { "gadget", GADGET },
    { "border", BORDER },
    { "b", BORDER },
    { NULL, NULL }
};

/*
 * Gravity expressions for parser.
 */
GravityExpr GravityExprTbl[] = {
    { "noGravity", NoGadgetGravity },
    { "nogravity", NoGadgetGravity },
    { "NoGravity", NoGadgetGravity },
    { "leftGravity", LeftGadgetGravity },
    { "leftgravity", LeftGadgetGravity },
    { "LeftGravity", LeftGadgetGravity },
    { "rightGravity", RightGadgetGravity },
    { "rightgravity", RightGadgetGravity },
    { "RightGravity", RightGadgetGravity },
    { "centerGravity", CenterGadgetGravity },
    { "centerGravity", CenterGadgetGravity },
    { "CenterGravity", CenterGadgetGravity },
    { NULL, NULL}
};

/*
 * Button expression table for parser.
 */
ButtonModifier ButtModTbl[] = {
    { "left", LeftMask },
    { "leftbutton", LeftMask },
    { "l", LeftMask },
    { "middle", MiddleMask },
    { "middlebutton", MiddleMask },
    { "m", MiddleMask },
    { "right", RightMask },
    { "rightbutton", RightMask },
    { "r", RightMask },
    { "move", DeltaMotion },
    { "motion", DeltaMotion },
    { "delta", DeltaMotion },
    { "boogie", DeltaMotion },
    { "truckin", DeltaMotion }, /* it's been a silly day... */
    { "down", ButtonDown },
    { "d", ButtonDown },
    { "up", ButtonUp },
    { "u", ButtonUp },
    { NULL, NULL }
};
 
