
#ifndef lint
     static char sccs_id[] = "@(#)rtlmenu.c	2.1 12/16/87  Siemens Corporate Research and Support, Inc.";
#endif


/* 
  RTL Menu Package Version 1.0
  by Joe Camaratta and Mike Berman, Siemens RTL, Princeton NJ, 1987
  
  rtlmenu.c: interface level for menu package
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

#include <stdio.h>
#include "X11/Xlib.h"

#include "menu.h"
#include "menu.def.h"
#include "menu.ext.h"
#include "rtlmenu.h"
#include "dbug.h"

extern Display *dpy;
extern int scr;

static RTLPoint menu_pos;
static int initialized = 0;
    
void RTLMenu_Initialize (options_mask)
MenuOptionsMask options_mask;
{
     Entry("RTLMenu_Initialize")

     if (initialized)
	  return;
     initialized = 1;
     InitMenu ("awm", options_mask);
     Leave_void
}

/* ------------------------------------------------------------ */
RTLMenu RTLMenu_Create()
{
     Entry("RTLMenu_Create")
     /* Create a menu with no name, not in inverse video. */
     
     Leave((RTLMenu)NewMenu())
}

/* ------------------------------------------------------------ */
void RTLMenu_Destroy( menu )
RTLMenu menu;
{
     Entry("RTLMenu_Destroy")
     DisposeMenu ((Menu*) menu);
     Leave_void
}

/* $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ */

pointer RTLMenu_Data( menu, item )
/*ARGSUSED*/
RTLMenu menu;
RTLMenuItem item;
{
     Entry("RTLMenu_Data")
     Leave(ItemData((MenuItem*)item))
}

/* ------------------------------------------------------------ */

RTLMenu RTLMenu_Get_Submenu( menu, str )
RTLMenu menu;
char *str;
{
     MenuItem *item = MenuItemByName ((Menu*) menu, str);
 
     Entry("RTLMenu_Get_Submenu")

     if (!ItemIsNull(item))
	  Leave((RTLMenu)ItemSubmenu(item))
     Leave(0)
}

/* $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ */

/*ARGSUSED*/
static void RTLMenu_Client_Send( menu, item )
RTLMenu menu;
RTLMenuItem item;
{
     Entry("RTLMenu_Client_Send")

     printf ("RTLMenu_Client_Send not yet implemented for X, string = \"%s\"\n",
	     (char*) item);
     Leave_void
}

/* ------------------------------------------------------------ */

RTLMenuItem RTLMenu_Append_String( menu, str, strval )
RTLMenu menu;
char *str;
char *strval;
{
     MenuItem *item = AddMenuItem((Menu*)menu, str, (char *)NULL);
 
     Entry("RTLMenu_Append_String")

     ItemCallback(item) = (Callback)RTLMenu_Client_Send;
     ItemData(item) = (pointer) strval;
     Leave((RTLMenuItem)item)
}

/* ------------------------------------------------------------ */

RTLMenuItem RTLMenu_Append_Callback( menu, backproc )
RTLMenu menu;
VoidFunc backproc;
{
     MenuItem *item = AddMenuItem((Menu*)menu, "", (char *)NULL);
 
     Entry("RTLMenu_Append_Callback")

     ItemGenerator(item) = (Callback)backproc;
     
     Leave((RTLMenuItem)item)
}

/* ------------------------------------------------------------ */

RTLMenuItem RTLMenu_Append_Call( menu, str, pix, proc, ptrval )
RTLMenu menu;
char *str;
char *pix;
VoidFunc proc;
pointer ptrval;
{
     MenuItem *item = AddMenuItem((Menu*)menu, str, pix);
     
     Entry("RTLMenu_Append_Call")

     ItemCallback(item) = proc;
     ItemData(item) = ptrval;
     
     Leave((RTLMenuItem)item)
}

/* ------------------------------------------------------------ */

RTLMenuItem RTLMenu_Append_Checkback( menu, str, pix, checkproc, proc, ptrval )
RTLMenu menu;
char *str;   /* Menu string */
char *pix;
Boolean (*checkproc) ();  /* Boolean function called when menu entry
			  is about to be displayed.  If true, then
			  the item appears checked; if false, then
			  it does not */
VoidFunc proc;  /* function to be called when the item 
		   is selected */
pointer ptrval; /* data associated with this item */
{
     MenuItem *item = AddMenuItem((Menu*)menu, str, pix);
 
     Entry("RTLMenu_Append_Checkback")

     ItemCallback(item) = proc;
     ItemData(item) = ptrval;
     ItemCheckproc(item) =  checkproc;
     
     Leave((RTLMenuItem)item)
}
/* ------------------------------------------------------------ */

RTLMenuItem RTLMenu_Append_Submenu( menu, str, pix, submenu )
RTLMenu menu;
char *str;
char *pix;
RTLMenu submenu;
{
     MenuItem *item = AddMenuItem((Menu*)menu, str, pix);
 
     Entry("RTLMenu_Append_Submenu")

     ItemSubmenu(item) = (Menu*)submenu;
     
     Leave((RTLMenuItem)item)
}

/* $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ */

void RTLMenu_Replace_Data_Keyed_String( menu, data, str )
RTLMenu menu;
pointer data;
char *str;
{
     MenuItem *item = MenuItemByName((Menu*) menu, str);
 
     Entry("RTLMenu_Replace_Keyed_String")

     if (!ItemIsNull(item))
	  ItemData(item) = data;
     Leave_void
} 

/* $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ */

void RTLMenu_Activate_Entry( menu, item )
RTLMenu menu;
RTLMenuItem item;
{
     Entry("RTLMenu_Activate_Entry")

     (void)SetItemDisable((Menu*)menu, (MenuItem*)item, FALSE);
     Leave_void
}

/* ------------------------------------------------------------ */    

void RTLMenu_Inactivate_Entry( menu, item )
RTLMenu menu;
RTLMenuItem item;
{
     Entry("RTLMenu_Inactivate_Entry")

     (void)SetItemDisable((Menu*)menu, (MenuItem*)item, TRUE);
     Leave_void
}

void RTLMenu_Label_Entry( menu, item )
RTLMenu menu;
RTLMenuItem item;
{
     Entry("RTLMenu_Label_Entry")

     (void)SetItemDeaf((Menu*)menu, (MenuItem*)item, TRUE);
     Leave_void
}

/* $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ */

Boolean RTLMenu_Has_Data( menu, val )
RTLMenu menu;
pointer val;
{
     MenuItem *item = MenuItemByData((Menu*)menu, val);
     Entry("RTLMenu_Has_Data")

     Leave(!ItemIsNull(item))
}

/* $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ */

void RTLMenu_Delete_Entries( menu )
RTLMenu menu;
{
     Entry("RTLMenu_Delete_Entries")

     while (DisposeItem((Menu*)menu, MenuItems((Menu*)menu)));
     Leave_void
}

/* ------------------------------------------------------------ */

void RTLMenu_Delete_String( menu, str )
RTLMenu menu;
char *str;
{
     MenuItem *item = MenuItemByName((Menu*) menu, str);
     
     Entry("RTLMenu_Delete_String");

     if (!ItemIsNull(item))
	  (void) DisposeItem ((Menu*)menu, item);
     Leave_void
}

/* ------------------------------------------------------------ */

void RTLMenu_Delete_Data( menu, val )
RTLMenu menu;
pointer val;
{
     MenuItem *item = MenuItemByData((Menu*)menu, val);
     
     Entry("RTLMenu_Delete_Data");

     if (!ItemIsNull(item))
	  (void) DisposeItem((Menu*)menu, item);
     Leave_void
}

/* $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ */

/*ARGSUSED*/
void RTLMenu_Box( menu )
RTLMenu menu;
{
     /* Commented out */
}

/* ------------------------------------------------------------ */

/*ARGSUSED*/
void RTLMenu_Enter( menu, butnum, buttime, base_window, pos )
RTLMenu menu;
int butnum;
Time buttime;
Window base_window;
RTLPoint pos;
{
     MenuItem *selected;
     
     Entry("RTLMenu_Enter")

     menu_pos = pos;
     selected = TrackMenu ((Menu*) menu, menu_pos.x, menu_pos.y,
			   butnum, base_window, buttime);
     if (!ItemIsNull(selected))
	  (ItemCallback(selected)) (menu, selected, base_window);
     Leave_void
}
/* ------------------------------------------------------------ */

RTLPoint RTLMenu_Entry_Pos()
{
     Entry("RTLMenu_Entry_Pos")
     /* menu_pos is a global static that gets set by the call to */
     /* XQueryPointer that is used by the TrackMenu call.        */
     
     Leave(menu_pos)
}


void RTLMenu_Generate_Items(menu, proc)
/* apply VoidFunc proc to each item in the menu, with       */
/* arguments menu and menuItem                              */
RTLMenu menu;
VoidFunc proc;
{
     MenuItem *item;
     
     Entry("RTLMenu_Generate_Items")

     for (item = MenuItems((Menu*)menu);
	  !ItemIsNull(item); item = ItemNext(item))
     {
	  (proc)((RTLMenu)menu, (RTLMenuItem)item);
     }
     Leave_void
}
