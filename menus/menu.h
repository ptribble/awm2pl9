
/*
#ifndef lint
static char sccs_id[] = "@(#)menu.h	2.1 12/16/87  Siemens Corporate Research and Support, Inc.";
#endif
*/


/* 
  RTL Menu Package Version 1.0
  by Joe Camaratta and Mike Berman, Siemens RTL, Princeton NJ, 1987

  menu.h: menus header file
  based on: menu.h	X10/6.6	11/3/86
*/

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

Copyright 1985, 1986, 1987 by the Massachusetts Institute of Technology

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in
advertising or publicity pertaining to distribution of the
software without specific, written prior permission.
M.I.T. makes no representations about the suitability of
this software for any purpose.  It is provided "as is"
without express or implied warranty.

*/

/*

Copyright 1987 by
	Siemens Corporate Research and Support, Inc., Princeton, New Jersey

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting
documentation, and that the name of Siemens not be used in
advertising or publicity pertaining to distribution of the
software without specific, written prior permission.
Siemens makes no representations about the suitability of
this software for any purpose.  It is provided "as is"
without express or implied warranty.

*/

#ifndef MENU_INCLUDE
#define MENU_INCLUDE
/*
 * Menu items are constructed as follows, starting from the left side:
 *
 *	menuItemPad
 *	space for check mark
 *	menuItemPad
 *	text + padding
 *	menuItemPad
 *
 * The padding for the text is that amount that this text is narrower than the
 * widest text.
 */
extern void Retch();

#include "std_defs.h"
#include "rtlmnu.opt.h"

typedef void (*Callback) ();
typedef struct _menuItem MenuItem;
typedef struct _menu Menu;

/*
 * Special note for those familiar with the original RTL menus:
 * Where we used to keep height and width information for items
 * in the _menu structure, we now just keep width. Each item
 * can be of a different height, but is still constrained
 * to a fixed width (which is the width of the widest item in the
 * menu). We could allow variable widths, but the menu would look
 * like hell. We also allow arbitrary pixmap backgrounds for each
 * item now.
 */
struct _menuItem {
	int itemFlags;			/* flags of item */

#define	itemDisabled		0x0001	/* item is disabled */
#define	itemChecked		0x0002	/* item has check mark */
#define itemDeaf		0x0004	/* item is a label */
#define	itemChanged		0x0010	/* item desires change */
	Pixmap itemBackground;		/* either zero or a pixmap */
	char *itemText;			/* text of item (if no pixmap) */
	unsigned int itemTextWidth;	/* width of text (or pixmap) */
	unsigned int itemTextLength;	/* length of text */
	unsigned int itemHeight;	/* height of this item */
	struct _menuItem *nextItem;	/* next item in chain */
	struct _menu *itemSubmenu;	/* For pull-right menus  */
	Callback generator;		/* Change string and data */
	caddr_t genParamPointer;	/* Address for generated parameter */
	Boolean (*checkproc) ();	/* Checkmark callback generator */
	Callback itemCallback;          /* user callback for item */
	pointer itemData;               /* user data associated with menu */
        Window  itemWindow;             /* window of item */
        Menu *itemMenu;                 /* menu this item belongs to */
};

struct _menu {
	unsigned int menuWidth;		/* full width of menu */
	unsigned int menuHeight;	/* full height of menu */
	unsigned int avgHeight;		/* Hack.. */
        unsigned int menuOldWidth;
        unsigned int menuOldHeight;
        unsigned int menuItemWidth;     /* width of a menu item */
	int menuFlags;			/* flags of this menu */

# define	menuChanged	0x0001		/* menu changed, must redraw */
# define	menuItemChanged	0x0002		/* item changed, must redraw */
# define	menuMapped	0x0004		/* menu is now mapped */

	unsigned int menuMaxTextWidth;	/* width of widest text */
        unsigned int menuOldBorderWidth;
	Display *display;		/* display of menu */
	int screen;			/* screen on which to display menu */
	Window menuWindow;		/* window of menu */
	Cursor menuCursor;		/* cursor used in menu */
	Pixmap savedPixmap;             /* for saving image under menu */
	MenuItem *menuItems;		/* head of menu item chain */
	XFontStruct *menuFontInfo; 	/* Font info */
	XFontStruct *boldFont;
	GC normalGC;			/* Graphics contexts */
	GC invertGC;			/* Used for inverted text */
	GC boldGC;			/* For labels */
	Pixmap greyPixmap;		/* Need to know what to free */
	Pixmap checkmarkPixmap;		/* Pixmap for drawing check mark. */
	Pixmap arrowPixmap;		/* Pixmap for drawing arrow. */
	char *menuInitialItemText;  	/* != NULL, initial item */
	MenuItem *highlightedItem;	/* highlighted menu item */
	int menuNested;			/* depth you are nested */
	int menuX, menuY;		/* Menu position */
        struct _menu *parentMenu;       /* menu to which you are a submenu
                                         * (NULL if you are the root)
					 */
	MenuOptionsMask menuOptions;    /* option values for this menu */
    };
#endif MENU_INCLUDE
