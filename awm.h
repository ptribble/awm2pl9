#ifndef lint
static char *rcsid_awm_h = "$Header: /usr/graph2/X11.3/contrib/windowmgrs/awm/RCS/awm.h,v 1.2 89/02/07 21:24:27 jkh Exp $";
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
 *                         All Rights Reserved
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
 * 002 -- Loretta Guarino Reid, DEC Ultrix Engineering Group,
 *  Western Software Lab, Port to X11
 * 003 -- Jordan Hubbard, Ardent Computer
 *  Many additional declarations for awm.
 * 1.3 -- Support for WM_STATE (Mike Wexler)
 */
 
#ifndef AWM_INCLUDE
#define AWM_INCLUDE
#include <errno.h>
#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/Xatom.h>

#include "menus/rtlmenu.h"
#include "support.h"
 
#define MIN(x, y)	((x) <= (y) ? (x) : (y))
#define MAX(x, y)	((x) >= (y) ? (x) : (y))
#define VOLUME_PERCENTAGE(x)	((x)*14) 
#define ModMask 0xFF
#define ButtonMask(b)	(((b)==Button1) ? Button1Mask : \
			  (((b)==Button2) ? Button2Mask : Button3Mask))

#define DEF_DELTA		1
#define DEF_FUNC		GXcopy
#define DEF_ICON_BORDER_WIDTH 	2
#define DEF_ICON_PAD		4
#define DEF_POP_BORDER_WIDTH 	2
#define DEF_POP_PAD		4
#define DEF_MENU_BORDER_WIDTH 	2
#define DEF_MENU_PAD		4
#define DEF_GADGET_PAD		3
#define DEF_GADGET_BORDER	1
#define DEF_TITLE_PAD		2
#define DEF_VOLUME		4
#define DEF_PUSH		5
#define DEF_BCONTEXT_WIDTH	0
#define DEF_RAISE_DELAY		100	/* milliseconds */
#define DEF_MAX_COLORS		0	/* 0 means take as many as we can */
#ifndef DEF_BCONTEXT_CURSOR
#define DEF_BCONTEXT_CURSOR	XC_plus
#endif	DEF_BCONTEXT_CURSOR
#ifndef	DEF_TITLE_CURSOR
#define DEF_TITLE_CURSOR	XC_left_ptr
#endif	DEF_TITLE_CURSOR
#ifndef	NAME
#define NAME			"awm"
#endif	NAME
#ifndef CLASS
#define	CLASS			"Wm"
#endif	CLASS
#ifndef	DEF_FONT
#define	DEF_FONT		"fixed"
#endif	DEF_FONT
#ifndef	DEF_TITLE_FONT
#define	DEF_TITLE_FONT		"8x13"
#endif	DEF_TITLE_FONT
#ifndef	DEF_ICON_FONT
#define	DEF_ICON_FONT		"8x13"
#endif	DEF_ICON_FONT
#ifndef	DEF_POPUP_FONT
#define	DEF_POPUP_FONT		"9x15"
#endif	DEF_POPUP_FONT
#ifndef	DEF_GADGET_FONT
#define DEF_GADGET_FONT		"fixed"
#endif	DEF_GADGET_FONT
#ifndef	DEF_MENU_FONT
#define	DEF_MENU_FONT		"8x13"
#endif	DEF_MENU_FONT
#ifndef	DEF_BOLD_FONT
#define	DEF_BOLD_FONT		"8x13bold"
#endif	DEF_BOLD_FONT
#define DEF_MENU_DELTA		20
#ifndef DEF_NAME
#define DEF_NAME		"NoName" /* for clients w/no name */
#endif

#define INIT_PTEXT		{'0', '0', '0', 'x', '0', '0', '0'}

#ifndef TEMPFILE
#define TEMPFILE		"/tmp/awm.XXXXXX"
#endif	TEMPFILE

#define CURSOR_WIDTH		16
#define CURSOR_HEIGHT		16
 
#define MAX_ZAP_VECTORS		8
#define MAX_BOX_VECTORS		20
 
#define DRAW_WIDTH		0 	/* use fastest hardware draw */
#define DRAW_VALUE		0xfd
#define DRAW_FUNC		GXxor
#define DRAW_PLANES		1

#define NOCOLOR			-1

/*
 * The first BITS_USED bits of the mask are used to define the most generic
 * types. All other bits are free for storing peripheral information.
 * For now, we use the extra bits to specify which gadget(s) are bound
 * to an action. MAX_GADGETS depends on BITS_USED to determine the maximum
 * number of gadgets possible. If you add a new context type, be sure to
 * increment BITS_USED.
 */
#define ROOT			0x1
#define WINDOW			0x2
#define ICON			0x4
#define TITLE			0x8
#define BORDER			0x10
#define GADGET			0x20

#define BITS_USED		6

/* Window states */
#define ST_WINDOW	0x1
#define ST_ICON		0x2
#define ST_PLACED	0x4
#define ST_DECORATED	0x8

#define DECORATED(a) (a && (a->state & ST_DECORATED))

/* Window attributes */
#define AT_NONE		0x0
#define AT_TITLE	0x1
#define AT_GADGETS	0x2
#define AT_RAISE	0x4
#define AT_BORDER	0x8
#define AT_INPUT	0x10
#define AT_ICONLABEL	0x20

/*
 * Gadgets aren't the sort of embellishments that one uses in quantitity
 * (unless one is designing a truly odd interface), so we keep the information
 * in an array instead of using the usual linked list. MAX_GADGETS is a derived
 * macro (see BITS_USED) that is used to determine the size of the
 * Array.
 *
 */
#define MAX_GADGETS		((sizeof(int) * 8) - BITS_USED)

#ifndef FAILURE
#define FAILURE	0
#endif	FAILURE

#define NAME_LEN		256L	/* Maximum length string names */
#define EVENTMASK		(ButtonPressMask | ButtonReleaseMask)


#define DrawBox() XDrawSegments(dpy, RootWindow(dpy, scr),DrawGC,box,num_vectors)
#define DrawZap() XDrawSegments(dpy, RootWindow(dpy, scr),DrawGC,zap,num_vectors)

#define gray_width 16
#define gray_height 16
extern char gray_bits[];

#define solid_width 16
#define solid_height 16
extern char solid_bits[];
 
#define xlogo32_width 32
#define xlogo32_height 32
extern char xlogo32_bits[];

/*
 * All one needs to know about an awm managed window.. (so far...)
 */
#ifdef WMSTATE
#define WithdrawState	0
#define NormalState	1
#define IconicState	3

typedef struct {
  int state;
  Window icon;
} WM_STATE;

extern Atom wm_state_atom;
#endif /* WMSTATE */


typedef struct _awminfo {
     Window title, client, frame, icon;	/* Associated windows */
     Window *gadgets;			/* associated gadgets */
     char *name;			/* The formatted window name */
     Boolean own;			/* Do we own the icon window? */
     Pixmap back, bold, iconPixmap;	/* background, bold and icon pix */
     Pixmap BC_back, BC_bold;		/* BC back and bold pixmaps */
#ifdef RAINBOW
     /* Per window versions of the globals colours */
     Pixmap grayPixmap, solidPixmap;
     Pixmap iBackPixmap;
     Pixel foreColor, backColor;
     Pixel iBorder;
     Pixel iBackground, iForeground;
     Pixel iTextForeground, iTextBackground;
#endif
     unsigned int border_width;		/* original border width */
     int state;				/* The state of the window */
     int attrs;				/* Window "attributes" */
     GC winGC;				/* GC at proper depth for window */
#ifdef WMSTATE
     WM_STATE wm_state;
#endif /* WMSTATE */
} AwmInfo, *AwmInfoPtr;

/*
 * This whole section has changed substantially. Basically, since all the
 * variables have vanished into the resource manager, the keyword table
 * only needs to keep track of function pointers, boolean pointers and
 * "special" keywords like menu and gagdet. Since some things are still
 * modifiable from menus (only booleans, currently, though this will change),
 * we keep these in the keyword table even though
 * they're no longer directly modifable from the .awmrc
 */

/*
 * Keyword table entry.
 */
typedef struct _keyword {
     char *name;
     int type;
     Boolean *bptr;
     Boolean (*fptr)();
} Keyword;

/*
 * Keyword table type entry.
 */
#define IsFunction	1
#define IsMenuMap	2
#define IsMenu		3
#define IsDownFunction	4
#define IsParser	5
#define IsQuitFunction	6
#define IsGadget	7
#define IsBoolean	8
#define IsAction	9

/*
 * Button/key binding type.
 */
typedef struct _binding {
    struct _binding *next;
    int context;
    unsigned int mask;
    int button;
    Boolean (*func)();
    char *menuname;
    RTLMenu menu;
} Binding;
 
/*
 * Key expression type.
 */
typedef struct _keyexpr {
    char *name;
    int mask;
} KeyExpr;
 
/*
 * Context expression type.
 */
typedef struct _contexpr {
    char *name;
    int mask;
} ContExpr;

/*
 * Button modifier type.
 */
typedef struct _buttonmodifier {
    char *name;
    int mask;
} ButtonModifier;

/*
 * Gravity expression type.
 */
typedef struct _gravityexpr {
     char *name;
     int mask;
} GravityExpr;

/*
 * Button modifier mask definitions.
 * bits 13 and 14 unused in key masks, according to X.h
 * steal bit 15, since we don't use AnyModifier
 */

#define DeltaMotion	(1<<13)
#define ButtonUp	(1<<14)
#define ButtonDown	AnyModifier
#define ButtonMods	DeltaMotion+ButtonUp+ButtonDown

/* 
 * Button and mask redefinitions, for X11
 */
#define LeftMask 	Button1Mask
#define MiddleMask 	Button2Mask
#define RightMask 	Button3Mask
#define LeftButton	Button1
#define MiddleButton	Button2
#define RightButton	Button3

/*
 * Declaration specific information for gadgets. This defines only gadget
 * types, not the actual gadgets. The pixmap member is only used if a pixmap
 * is being displayed. Gravity and offset are purely optional.
 */

#define NoGadgetGravity		0
#define LeftGadgetGravity	1
#define RightGadgetGravity	2
#define CenterGadgetGravity	3

typedef struct _gadgetdecl {	/*   Declaration (type) information */
     unsigned char *name;	/* Either text label or pixmap file name */
     unsigned char *data;	/* If pixmap file, this is the data from it */
     char *forecolor;		/* foreground color for pixmap */
     char *backcolor;		/* background color for pixmap */
     XFontStruct *fontInfo;	/* font for text */
     int high, wide;		/* width and height of pixmap or text */
     int gravity;		/* stick to the left or right? */
     int offset;		/* offset from previous item */
} GadgetDecl;

/*
 * MenuInfo data type.
 */
typedef struct _menuinfo {
    char *name;			/* Name of this menu. */
    char *pixmapname;		/* Name of label pixmap (opt) */
    RTLMenu menu;		/* RTL menu handle for destroy */
    struct _actionline *line;	/* Linked list of menu items. */
} MenuInfo;
 
/*
 * Action Line data type.
 */
typedef struct _actionline {
    struct _actionline *next;	/* Pointer to next line. */
    char *name;			/* Name of this line. */
    char *pixmapname;		/* Name of the backing pixmap (opt) */
    int type;			/* IsShellCommand, IsText, IsTextNL... */
    RTLMenuItem item;		/* RTL item handle */
    char *text;			/* Text string to be acted upon. */
    Boolean (*func)();		/* Window manager function to be invoked. */
} ActionLine;
 
/*
 * ActionLine->type definitions.
 */
#define IsShellCommand		1
#define IsText			2
#define IsTextNL		3
#define IsUwmFunction		4
#define IsMenuFunction		5
#define IsImmFunction		6	/* Immediate (context-less) function. */
#define IsVar			7	/* we're setting a boolean variable */
 
/*
 * Menu Link data type.  Used by the parser when creating a linked list
 * of menus. 
 */
typedef struct _menulink {
    struct _menulink *next;	/* Pointer to next MenuLink. */
    struct _menuinfo *menu;	/* Pointer to the menu in this link. */
} MenuLink;

/*
 * External variable definitions.
 */
extern int errno;
extern Window Pop;		/* Pop-up dimension display window. */
extern Window Frozen;		/* Contains window id of "gridded" window. */
extern XFontStruct *IFontInfo;	/* Icon text font information. */
extern XFontStruct *PFontInfo;	/* Pop-up text font information. */
extern XFontStruct *TFontInfo;	/* Title text font information. */
extern XFontStruct *TFontBoldInfo;/* Title text (bold) font information. */
extern XFontStruct *GFontInfo; /* Gadget box text font */
extern XFontStruct *MFontInfo;	/* Menu font */
extern XFontStruct *MBoldFontInfo;/* Menu bold font */
extern Pixmap GrayPixmap;	/* Gray pixmap. */
extern Pixmap SolidPixmap;
extern Pixmap IBackPixmap;	/* Icon window background pixmap. */
extern Pixmap IDefPixmap;	/* Icon pixmap for twm style icons */
extern char *BForeground;	/* Border Context (pixmap) foreground pixel */
extern char *BBackground;	/* Border Context (pixmap) background pixel */
extern char *WBorder;		/* Window border pixel */
extern char *TTextForeground;	/* Title text foreground pixel */
extern char *TTextBackground;	/* Title text background pixel */
extern char *TForeground;	/* Title (pixmap) foreground pixel */
extern char *TBackground;	/* Title (pixmap) background pixel */
extern char *Foreground;	/* default forground color (text) */
extern char *Background;	/* default background color (text) */
extern Pixel IBorder;		/* Icon window border pixel. */
extern Pixel ITextForeground;	/* Icon window text forground color. */
extern Pixel ITextBackground;	/* Icon window text background color. */
extern Pixel IForeground;	/* Icon pixmap foreground color */
extern Pixel IBackground;	/* Icon pixmap background color */
extern Pixel PForeground;	/* Pop-up window forground color. */
extern Pixel PBackground;	/* Pop-up window background color. */
extern Pixel PBorder;		/* Pop-Up Window border pixel. */
extern Pixel ForeColor;		/* default foreground color */
extern Pixel BackColor;		/* default background color */
extern Pixel MBorder;		/* Menu border color */
extern Pixel MForeground;	/* Menu foreground color */
extern Pixel MBackground;	/* Menu background color */
extern Cursor ArrowCrossCursor; /* Arrow cross cursor. */
extern Cursor TextCursor;	/* Text cursor used in icon windows. */
extern Cursor IconCursor;	/* Icon Cursor. */
extern Cursor LeftButtonCursor;	/* Left button main cursor. */
extern Cursor MiddleButtonCursor;/* Middle button main cursor. */
extern Cursor RightButtonCursor;/* Right button main cursor. */
extern Cursor TargetCursor;	/* Target (select-a-window) cursor. */
extern Cursor TitleCursor;	/* Title bar cursor */
extern Cursor FrameCursor;	/* Frame cursor */
extern Cursor GumbyCursor;	/* Used in icons if not type-in   */
extern unsigned int GadgetBorder;	/* Width of gadget borders */
extern int ScreenWidth;		/* Display screen width. */
extern int ScreenHeight;	/* Display screen height. */
extern int TitleHeight;		/* Height in pixels of title bar(s) */
extern int titleHeight;		/* Derived height of title bar(s) */
extern int gadgetHeight;	/* Height of highest gadget */
extern int NameOffset;		/* Offset for window name */
extern int IBorderWidth;	/* Icon window border width. */
extern int PWidth;		/* Pop-up window width (including borders). */
extern int PHeight;		/* Pop-up window height (including borders). */
extern unsigned int PBorderWidth;	/* Pop-up window border width. */
extern int PPadding;		/* Pop-up window padding. */
extern int Delta;		/* Mouse movement slop. */
extern int HIconPad;		/* Icon horizontal padding. */
extern int VIconPad;		/* Icon vertical padding. */
extern int Pushval;		/* Number of pixels to push window by. */
extern int BContext;		/* Width of border context area in pixels */
extern int RaiseDelay;		/* Delay in milliseconds before autoraising windows */
extern int Volume;		/* Audible alarm volume. */
extern int NumGadgets;		/* Number of gadgets used */
extern int GadgetPad;		/* Padding between gadgets */
extern int TitlePad;		/* Title text padding */
extern int status;		/* Routine return status. */
extern int MPad;		/* menu padding */
extern int MDelta;		/* Menu subitem delta */
extern int MBorderWidth;	/* Menu border width */
extern int MItemBorder;		/* Menu item border width */
extern unsigned int BCursor;	/* Border context cursor */
extern unsigned int TCursor;	/* Title context cursor */
extern MenuLink *Menus;		/* Linked list of menus. */
extern GC  IconGC;		/* graphics context for icon */
extern GC  PopGC;		/* graphics context for pop */
extern GC  DrawGC;		/* graphics context for zap */

extern Boolean Autoraise;	/* Raise window on input focus? */
extern Boolean Autoselect;	/* Warp mouse to default menu selection? */
extern Boolean Borders;		/* Display border context areas? */
extern Boolean ConstrainResize;	/* Don't resize until pointer leaves window */
extern Boolean Freeze;		/* Freeze server during move/resize? */
extern Boolean Grid;		/* Should the m/r box contain a 9 seg. grid. */
extern Boolean Hilite;		/* Should we highlight titles on focus? */
extern Boolean BorderHilite;	/* Should we highlight borders on focus? */
extern Boolean FrameFocus;	/* Should frame be considered part of window */
extern Boolean ShowName;	/* Display names in title bars */
extern Boolean NWindow;		/* Normalize windows? */
extern Boolean NIcon;		/* Normalize icons? */
extern Boolean RootResizeBox;	/* Should resize box obscure window? */
extern Boolean InstallColormap;	/* Install colormap for clients? */
extern Boolean Push;		/* Relative=TRUE, Absolute=FALSE. */
extern Boolean ResizeRelative;	/* Relative=TRUE, Absolute=FALSE. */
extern Boolean Reverse;		/* Reverse video? */
extern Boolean SaveUnder;	/* Save unders? */
extern Boolean Snatched;	/* We're in the middle of an no-highlight/raise op */
extern Boolean Titles;		/* Title bars on windows? */
extern Boolean IconLabels;	/* Labels on pixmap icons? (twm style) */
extern Boolean ILabelTop;	/* label top of icon? */
extern Boolean PushDown;	/* Down=TRUE, Up=FALSE */
extern Boolean UseGadgets;	/* Gadget boxes in title bars? */
extern Boolean Wall;		/* Don't allow windows past edges of screen */
extern Boolean WarpOnRaise;	/* Warp to upper right corner on raise. */
extern Boolean WarpOnIconify;   /* Warp to icon center on iconify. */
extern Boolean WarpOnDeIconify; /* Warp to upper right corner on de-iconify. */
extern Boolean Zap;		/* Should the the zap effect be used. */
extern Boolean FocusSetByUser;  /* True if f.focus called */
extern Boolean FocusSetByWM;	/* True if awm set the focus */
 
extern char PText[];		/* Pop-up window dummy text. */
extern int PTextSize;		/* Pop-up window dummy text size. */

extern int Lineno;		/* Line count for parser. */
extern Boolean Startup_File_Error; /* Startup file error flag. */
extern char Startup_File[];	/* Startup file name. */
extern char *IFontName;		/* Icon font name. */
extern char *PFontName;		/* Pop-up font name. */
extern char *TFontName;		/* Title font name. */
extern char *GFontName;		/* Gadget font name */
extern char *TFontBoldName;	/* Bold Title font name. */
extern char *TBoldPixmapName;	/* Title (highlighted) pixmap file */
extern char *TBackPixmapData;	/* Bitmap data file title background */
extern char *TBoldPixmapData;	/* ditto, except highlighted */
extern char *BBackPixmapData;	/* Border Context area background pixmap data */
extern char *BBoldPixmapData;	/* Border Context bold pixmap data */
extern char *awmPath;		/* Pathlist for pixmap files */
extern char **Argv;		/* Pointer to command line parameters. */
extern char **Environ;		/* Pointer to environment. */
 
extern char *DefaultBindings[];	/* Default bindings string array. */
extern Keyword KeywordTable[];	/* Keyword lookup table. */
extern Binding *Blist;		/* Button/key bindings list. */
extern KeyExpr KeyExprTbl[];	/* Key expression table. */
extern ContExpr ContExprTbl[];	/* Context expression table. */
extern ButtonModifier ButtModTbl[];/* Button modifier table. */
extern GravityExpr GravityExprTbl[]; /* Gravity expression table. */

extern GadgetDecl **Gadgets;	/* Gadgets declared. See gram.y */
extern int scr;
extern Display *dpy;		/* Display info pointer. */
 
#ifdef PROFIL
int ptrap();
#endif
 
/*
 * External routine typing.
 */
extern Boolean Beep();
extern Boolean CircleDown();
extern Boolean CircleUp();
extern Boolean Continue();
extern Boolean Focus();
extern Boolean UnFocus();
extern Boolean GetButton();
extern Boolean Iconify();
extern Boolean Lower();
extern Boolean DoMenu();
extern Boolean DoAction();
extern Boolean Lock();
extern Boolean Move();
extern Boolean MoveOpaque();
extern Boolean Neaten();
extern Boolean NewIconify();
extern Boolean Pause();
extern Boolean ShoveDown();
extern Boolean ShoveLeft();
extern Boolean ShoveRight();
extern Boolean ShoveUp();
extern Boolean Quit();
extern Boolean Raise();
extern Boolean Redraw();
extern Boolean Refresh();
extern Boolean ResetBindings();
extern Boolean ResetMenus();
extern Boolean ResetGadgets();
extern Boolean Resize();
extern Boolean Restart();
extern Boolean FDecorate();
extern Boolean FNoDecorate();
extern Boolean DestroyClient();
extern Boolean GetBoolRes();
extern Boolean ConfigureWindow();
extern int StoreCursors();
extern int StoreBox();
extern int StoreTitleBox();
extern int StoreGridBox();
extern int StoreTitleGridBox();
extern int StoreZap();
extern int Error();
extern int XError();
extern int GetIntRes();
extern Window Reparent(), Decorate();
extern unsigned char *expand_metachars();
extern char *stash();
extern char *GetIconName();
extern char *expand_from_path();
extern char *GetStringRes();
extern char *GetPixmapDataRes();
extern Pixmap GetPixmapRes();
extern Pixel GetColorRes();
extern Pixel GetPixel();
extern XFontStruct *GetFontRes();
extern Drawable GetPixmapFromCache();
extern AwmInfoPtr GetAwmInfo();
extern AwmInfoPtr RegisterWindow();
extern AwmInfoPtr IsTitled();
extern AwmInfoPtr IsGadgetWin();

extern void Init_Titles(), Init_Frames();
extern void NoDecorate();
extern void PaintTitle();
extern void SetBorderPixmaps();
extern void FreePixmapFromCache();

#ifdef	NEATEN
#define DEFAULT_ABS_MIN		64
#define SEPARATION		2
#define DEF_PRIMARY_PLACEMENT	"Top"
#define DEF_SECONDARY_PLACEMENT	"Left"

extern int AbsMinWidth;
extern int AbsMinHeight;
extern Boolean RetainSize;
extern Boolean KeepOpen;
extern Boolean Fill;
extern Boolean UsePriorities;
extern Boolean FixTopOfStack;
extern char *PrimaryIconPlacement;
extern char *SecondaryIconPlacement;
#endif	NEATEN
#endif AWM_INCLUDE
