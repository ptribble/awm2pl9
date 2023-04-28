
/*
#ifndef lint
static char sccs_id[] = "@(#)menu.extern.h	2.1 12/16/87  Siemens Corporate Research and Support, Inc.";
#endif
*/


/* 
  RTL Menu Package Version 1.0
  by Joe Camaratta and Mike Berman, Siemens RTL, Princeton NJ, 1987

  menu.extern.h: external function declarations for menu package
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

extern MenuItem *AddMenuItem();
extern Boolean     DisposeItem();
extern void     DisposeMenu();
extern void     InitMenu();
extern Boolean     SetItemText();
extern Menu     *NewMenu();
extern Boolean     SetItemCheck();
extern Boolean     SetItemDisable();
extern MenuItem *Display_Menu();
extern void     Undisplay_Menu();
extern void     MenuInvert();
extern void     PlacePointer();
extern void     Draw_Menu();
extern void     Draw_Item();

extern MenuItem *MenuGetItem();
extern MenuItem *MenuItemByName();
extern MenuItem *MenuItemByData();
extern Menu     *MenuGetMenu();
extern int      ItemGetMiddleY();
extern void     SetInitialItem();
extern void     ClearInitialItem();
extern MenuItem *GetInitialItem();

extern MenuItem *TrackMenu();










