
/*
#ifndef lint
static char sccs_id[] = "@(#)menu.define.h	2.1 12/16/87  Siemens Corporate Research and Support, Inc.";
#endif
*/


/*
  RTL Menu Package Version 1.0
  by Joe Camaratta and Mike Berman, Siemens RTL, Princeton NJ, 1987

  menu.h: macros for menu package

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

Copyright 1987 by
	Siemens Corporate Research and Support, Inc., Princeton, New Jersey

Permission to use, copy, modify, and distribute this software
and its documentation for any purpose and without fee is
hereby granted, provided that the above copyright notice
appear in all copies and that both that copyright notice and
this permission notice appear in supporting documentation, and
that the name of Siemens not be used in advertising or
publicity pertaining to distribution of the software without
specific, written prior permission.  Siemens makes no
representations about the suitability of this software for any
purpose.  It is provided "as is" without express or implied
warranty.

*/
#ifndef MENU_DEF_INCLUDE
#define MENU_DEF_INCLUDE

/* Menu data structure access macros */

#define MenuWidth(menu) (menu)->menuWidth
#define MenuHeight(menu) (menu)->menuHeight
#define MenuOldWidth(menu) (menu)->menuOldWidth
#define MenuOldHeight(menu) (menu)->menuOldHeight
#define MenuItemWidth(menu) (menu)->menuItemWidth
#define MenuFlags(menu) (menu)->menuFlags
#define MenuMaxTextWidth(menu) (menu)->menuMaxTextWidth
#define MenuBorderWidth(menu) MBorderWidth
#define MenuOldBorderWidth(menu) (menu)->menuOldBorderWidth
#define MenuItemPad(menu) MPad
#define MenuDisplay(menu) (menu)->display
#define MenuScreen(menu) (menu)->screen
#define MenuWindow(menu) (menu)->menuWindow
#define MenuCursor(menu) (menu)->menuCursor
#define MenuSavedPixmap(menu) (menu)->savedPixmap
#define MenuSavedImage(menu) ((menu)->savedImage)
#define MenuSavedImageX(menu) (menu)->menuSavedImageX
#define MenuSavedImageY(menu) (menu)->menuSavedImageY
#define MenuItems(menu) (menu)->menuItems
#define MenuFontInfo(menu) (menu)->menuFontInfo
#define MenuNormalGC(menu) (menu)->normalGC
#define MenuInvertGC(menu) (menu)->invertGC
#define MenuGreyGC(menu) (menu)->greyGC
#define MenuGreyPixmap(menu) (menu)->greyPixmap
#define MenuCheckmarkPixmap(menu) (menu)->checkmarkPixmap
#define MenuArrowPixmap(menu) (menu)->arrowPixmap
#define MenuInitialItemText(menu) (menu)->menuInitialItemText
#define MenuItemHighlighted(menu) (menu)->highlightedItem
#define MenuNested(menu) (menu)->menuNested
#define MenuX(menu) (menu)->menuX
#define MenuY(menu) (menu)->menuY
#define MenuParent(menu) (menu)->parentMenu
#define MenuDelta(menu) MDelta
#define MenuOptions(menu) (menu)->menuOptions
#define NULLMENU ((Menu *) NULL)

/* Menu predicates */

#define MenuIsNull(menu) (menu == NULLMENU)
#define MenuIsDisplayed(menu) (menu->menuFlags & menuMapped)
#define MenuHasInitialItem(menu) (menu->menuInitialItemText != (char *) NULL)

/* Menu modifiers */

#define SetHighlightItem(menu, item) (menu->highlightedItem = item)
#define ResetHighlightItem(menu) (menu->highlightedItem = NULLITEM)
#define	SetMenuEventHandler(menu,f)	menu->menuEventHandler = f
#define SetMenuFlag(menu, flag)     MenuFlags(menu) |= flag
#define ResetMenuFlag(menu,flag)    MenuFlags(menu) &= ~flag
#define TestMenuFlag(menu,flag)     (MenuFlags(menu) & flag)
#define TestOptionFlag(menu,option) (MenuOptions(menu) & option)

/* Item data structure access macros */

#define ItemFlags(item) (item)->itemFlags
#define ItemText(item) (item)->itemText
#define ItemTextLength(item) (item)->itemTextLength
#define ItemTextWidth(item) (item)->itemTextWidth
#define ItemNext(item) (item)->nextItem
#define ItemSubmenu(item) (item)->itemSubmenu
#define ItemTag(item) (item)->itemTag
#define ItemGenerator(item) (item)->generator
#define ItemGeneratorParamPointer(item) (item)->genParamPointer
#define ItemCheckproc(item) (item)->checkproc
#define ItemCallback(item) (item)->itemCallback
#define ItemData(item) (item)->itemData
#define ItemWindow(item) (item)->itemWindow
#define ItemMenu(item) (item)->itemMenu
#define NULLITEM ((MenuItem *) NULL)
#define ItemGetArrowPosition(item) (item->itemMenu->menuWidth - arrow_width)

/* Item predicates */
#define ItemIsNull(item) (item == NULLITEM)
#define ItemIsInitialItem(menu, item) \
    (MenuInitialItemText(menu) && \
     (strcmp(MenuInitialItemText(menu), ItemText(item)) == 0))
#define ItemIsLeaf(item) ((item->itemSubmenu == (Menu *) 0) ? TRUE : FALSE)
#define ItemIsDisabled(item) (ItemFlags(item) & itemDisabled)
#define ItemIsDeaf(item) (ItemFlags(item) & itemDeaf)

/* Item modifiers */

#define	CheckItem(menu,item)	SetItemCheck(menu,item,1)
#define	DisableItem(menu,item)	SetItemDisable(menu,item,1)
#define	EnableItem(menu,item)	SetItemDisable(menu,item,0)
#define	UncheckItem(menu,item)	SetItemCheck(menu,item,0)
#define SetItemFlag(item, flag) ItemFlags(item) |= flag
#define ResetItemFlag(item,flag) ItemFlags(item) &= ~flag
#define TestItemFlag(item,flag) (ItemFlags(item) & flag)
#define MoveItemFlag(item,flag1,flag2) \
    if (TestItemFlag(item,flag1))\
        SetItemFlag(item,flag2);\
    else\
        ResetItemFlag(item,flag2);\
    ResetItemFlag(item,flag1);

/* Masks */

#define MenuEventMask (LeaveWindowMask | EnterWindowMask | ButtonReleaseMask)
#define NormalItemEventMask MenuEventMask
#define SubmenuItemEventMask (MenuEventMask | PointerMotionMask)
#define MenuIgnoreMask (unsigned long) 0

/* Magic numbers */

#define	checkMarkWidth		9
#define	checkMarkHeight		8
#define	defaultCursorWidth     16
#define	defaultCursorHeight    16
#define	defaultCursorX		1
#define	defaultCursorY		1
#define	lineSeparatorHeight	9

/* external declarations */

extern Menu *NewMenu();
extern MenuItem *TrackMenu ();
extern MenuItem *MenuItemByName (), *Display_Menu(); 
extern void InitMenu ();
extern void DisposeMenu ();
extern void Draw_Menu();
#endif MENU_DEF_INCLUDE
