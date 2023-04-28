
#ifndef lint
     static char sccs_id[] = "@(#)menu.c	2.1 12/16/87  Siemens Corporate Research and Support, Inc.";
#endif


/*
  RTL Menu Package Version 1.0
  by Joe Camaratta and Mike Berman, Siemens RTL, Princeton NJ, 1987
  
  menu.c: menu utility and support functions.
  
  Originally hacked by Adam J. Richter, based on the menu package for xterm.
  ( misc.c  X10/6.6 )

  */

/*
 *
 * Changed in various and subtle ways by Jordan Hubbard, Ardent Computer.
 * February 1st, 1988: Removed dependence on oldX by nuking AssocTable
 * stuff in favor of contexts. Added "label" panes, useful for titling
 * purposes. Variable height items, variable fonts, pictoral panes
 * (pixmaps) for cute graphic menus.
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

/* Got that?  Good! Now, there is actually some code in here.. */


#include <stdio.h>
#include "X11/Xlib.h"
#include "X11/cursorfont.h"
#include "X11/Xutil.h"
#include "X11/Intrinsic.h"

#include "menu.h"
#include "menu.def.h"
#include "gray1.h"
#include "arrow_icon.h"
#include "dbug.h"

#define MakeEven(x) ((x%2 == 0)? x : x-1)


#define InvertPlane		1

static char Check_MarkBits[] = {
     0x00, 0x01, 0x80, 0x01, 0xc0, 0x00, 0x60, 0x00,
     0x31, 0x00, 0x1b, 0x00, 0x0e, 0x00, 0x04, 0x00
     };

Menu MenuDefault;

/*
 * the following have already been set my GetDefaults()
 * by the time we get here.
 */
XFontStruct *MFontInfo;
XFontStruct *MBoldFontInfo;
int MBorderWidth, MPad, MDelta, MItemBorder;
Pixel MForeground, MBackground, MBorder;

extern Boolean SaveUnder;
extern int Reverse;
extern Display *dpy;
extern int scr;

static XContext Menu_context;
static XContext Item_context;
static contexts_created = FALSE;

MenuItem *AddMenuItem(), *Display_Menu(), *MenuGetItem(),
     *MenuItemByName(), *MenuItemByData(), *GetInitialItem();
static MenuItem *MoveMenu();
Boolean	 DisposeItem(), SetItemCheck(), SetItemDisable(), Recalc_Menu(),
     SetItemText();
static Boolean	SetupItems(), MapMenu(), SetupMenuWindow();
void	 DisposeMenu(), InitMenu(), Undisplay_Menu(), MenuInvert(),
     PlacePointer(), Draw_Menu(), Draw_Item(),	SetInitialItem(),
     ClearInitialItem();
static void	 Generate_Menu_Entries(), UnmapMenu(),
     SetInputMask();
Menu	 *NewMenu(), *MenuGetMenu();
int	 ItemGetMiddleY();

/*
 * AddMenuItem() adds a menu item to an existing menu, at the end of the
 * list, which are number sequentially from zero.  The menuitem index is
 * return, or -1 if failed.
 */

MenuItem *AddMenuItem(menu, text, bitmap_file)
Menu *menu;
char *text;
char *bitmap_file;
{
     MenuItem *menuitem, **next;
     int junk;
     char *data;

     Entry("AddMenuItem")

     if (!menu || (!text && !bitmap_file) || (menuitem = allocate(MenuItem, 1))
	== (MenuItem *)0)
	  Leave(NULLITEM)
     bzero((char *)menuitem, sizeof(MenuItem));
     ItemText(menuitem) = allocate(char, (strlen(text) + 1));
     strcpy(ItemText(menuitem), text);
     ItemTextLength(menuitem) = strlen(text);
     if (bitmap_file) {
	  bitmap_file = (char *)expand_from_path(bitmap_file);
	  if (XmuReadBitmapDataFromFile(bitmap_file,
			      &(menuitem->itemTextWidth),
			      &(menuitem->itemHeight),
			      &data,
			      &junk, &junk) != BitmapSuccess) {
	       fprintf(stderr, "awm: Can't read bitmap file '%s'\n",
		       bitmap_file);
	       exit(1);
	  }
	  menuitem->itemBackground =
	       XCreatePixmapFromBitmapData(dpy, RootWindow(dpy, scr),
					   data,
					   menuitem->itemTextWidth,
					   menuitem->itemHeight,
					   MForeground,
					   MBackground,
					   DefaultDepth(dpy, scr));
	  if (!menuitem->itemBackground) {
	       fprintf(stderr, "awm: Can't create pixmap for file '%s'\n",
		       bitmap_file);
	       exit(1);
	  }
	  free(data);
	  free(bitmap_file);
     }
     for(next = &menu->menuItems; *next; next = &(*next)->nextItem);

     *next = menuitem;

     SetMenuFlag(menu, menuChanged);
     Leave(menuitem)
}

/*
 * DisposeItem() releases the memory allocated for the given indexed
 * menuitem.  Nonzero is returned if an item was actual disposed of.
 * It also checks to see whether the item we're disposing is the
 * initial item for the menu -- if so, null out the initial item.
 */
Boolean DisposeItem(menu, item)
Menu *menu;
MenuItem *item;
{
     MenuItem **next, **last, *menuitem;
 
     Entry("DisposeItem")

     if (MenuIsNull(menu) || ItemIsNull(item))
	  Leave(FALSE)
     next = &MenuItems(menu);
     do {
	  if(!*next)
	       Leave(FALSE)
	  last = next;
	  next = &(*next)->nextItem;
     } while((*last != item) && !ItemIsNull(*next));
     menuitem = *last;
     *last = *next;
     if (ItemWindow(menuitem)) {
	  XDeleteContext(dpy, ItemWindow(menuitem), Item_context);
	  XDestroyWindow(dpy, ItemWindow(menuitem));
     }
     if (ItemIsInitialItem(menu, menuitem))
	  ClearInitialItem(menu);

     if (ItemText(menuitem))
	  free((char*)ItemText(menuitem));
     free((char*)menuitem);

     SetMenuFlag(menu, menuChanged);
     Leave(TRUE)
}

/*
 * DisposeMenu() releases the memory allocated for the given menu.
 */
void DisposeMenu(menu)
Menu *menu;
{
     Entry("DisposeMenu")

     if(!menu)
	  Leave_void
     if (TestMenuFlag(menu, menuMapped))
	  UnmapMenu(menu);
     while(DisposeItem(menu, MenuItems(menu)));
     if(MenuWindow(menu)) {
	  XDeleteContext(dpy, MenuWindow(menu), Menu_context);
	  XDestroyWindow(dpy, MenuWindow(menu));
     }
     XFreePixmap (dpy, MenuGreyPixmap(menu));
     XFreePixmap (dpy, MenuArrowPixmap(menu));
     XFreePixmap (dpy, MenuCheckmarkPixmap(menu));
     XFreeGC (dpy, MenuNormalGC(menu));
     XFreeGC (dpy, MenuInvertGC(menu));
     XFreeGC (dpy, menu->boldGC);
     
     if (MenuHasInitialItem(menu))
	  ClearInitialItem(menu);
     free((char*) menu);
     Leave_void
}

static char *Name;

void InitMenu(name, options)
char *name;
unsigned int options;
{
     char *cp;
     
     Entry("InitMenu")

     Name = name;
     MenuDefault.menuFlags = menuChanged;
     
     MenuDefault.menuInitialItemText = (char *) NULL;
     MenuDefault.display = dpy;
     MenuDefault.screen = scr;
     MenuDefault.menuOptions = options;
     Leave_void
}

/*
 * ItemText changes the text of item of the menu.
 */
Boolean SetItemText(menu, item, text)
Menu *menu;
MenuItem *item;
char *text;
{
     Entry("SetItemText")

     if (strcmp (ItemText(item), text) == 0)
	  Leave (True)
     
     if(ItemText(item))
	  free((char *) ItemText(item));
     
     ItemText(item) = allocate(char, (strlen(text) + 1));
     strcpy(ItemText(item), text);
     
     ItemTextLength(item) = strlen (text);
     SetMenuFlag(menu, menuChanged);
     
     (void) Recalc_Menu (menu);
     Leave(TRUE)
}

/*
 * NewMenu() returns a pointer to an initialized new Menu structure, or NULL
 * if failed.
 *
 * The Menu structure _menuDefault contains the default menu settings.
 */
Menu *NewMenu()
{
     extern Pixmap MakeGreyStipple ();
     Menu *menu = 0;
     
     XGCValues gcValues;
     
     static unsigned long gcMask =
	  (GCFunction | GCForeground | GCBackground | GCFont | GCStipple |
	   GCSubwindowMode | GCGraphicsExposures);
     
     char *cp;

     Entry("NewMenu")

     /*
      * Allocate the memory for the menu structure.
      */
     if(MenuIsNull((menu = allocate(Menu, 1))))
	  Leave(NULLMENU)
     
     /*
      * Initialize to default values.
      */
     *menu = MenuDefault;
     gcValues.font = MFontInfo->fid;
     
     /*
      * If the menu cursor hasn't been given, make a default one.
      */

     MenuCursor(menu) = XCreateFontCursor (dpy, XC_right_ptr);
     MenuArrowPixmap(menu) = XCreatePixmapFromBitmapData(dpy,
							 RootWindow(dpy, scr),
							 arrow_bits,
							 arrow_width,
							 arrow_height,
							 MForeground,
							 MBackground,
							 DefaultDepth(dpy, scr));

     MenuGreyPixmap(menu) = XCreateBitmapFromData(dpy,
						  RootWindow(dpy, scr),
						  gray1_bits,
						  gray1_width, gray1_height);
     
     MenuCheckmarkPixmap(menu) = XCreatePixmapFromBitmapData(dpy,
							     RootWindow(dpy, scr),
							     Check_MarkBits,
							     checkMarkWidth,
							     checkMarkHeight,
							     MForeground,
							     MBackground,
							     DefaultDepth(dpy, scr));

     gcValues.foreground = MForeground;
     gcValues.background = MBackground;

     gcValues.stipple = MenuGreyPixmap(menu);
     gcValues.function = GXcopy;
     gcValues.subwindow_mode = IncludeInferiors;
     gcValues.graphics_exposures = False;
     MenuNormalGC(menu) =
	  XCreateGC (dpy, RootWindow(dpy, scr),
		     gcMask,
		     &gcValues);
     /* reverse */
     gcValues.foreground = MBackground;
     gcValues.background = MForeground;

     MenuInvertGC(menu) = XCreateGC(dpy, RootWindow (dpy, scr),
				    gcMask,
				    &gcValues);
     gcValues.foreground = MForeground;
     gcValues.background = MBackground;
     gcValues.font = MBoldFontInfo->fid;
     menu->boldGC = XCreateGC(dpy, RootWindow(dpy, scr),
			      gcMask,
			      &gcValues);
     Leave(menu)
}


/*
 * SetItemCheck sets the check state of item of the menu to "state".
 */
Boolean SetItemCheck(menu, item, state)
Menu *menu;
MenuItem *item;
int state;
{
     Entry("SetItemCheck")

     if (TestItemFlag(item,itemChecked) == state) /* Exit if unchanged */
	  Leave (True)
     
     if (state)
	  SetItemFlag(item, itemChecked);
     else
	  ResetItemFlag(item, itemChecked);
     
     SetItemFlag(item, itemChanged);
     SetMenuFlag(menu, menuItemChanged);
     
     Leave(TRUE)
}
/*
 * SetItemDeaf sets the "deaf" state of item of the menu to "state".
 * "deaf" means that the item is display only and not sensitive to input.
 */
Boolean SetItemDeaf(menu, item, state)
Menu *menu;
MenuItem *item;
int state;
{
     Entry("SetItemDeaf")

     if (TestItemFlag(item,itemDeaf) == state) /* Exit if unchanged */
	  Leave (True)
     
     if (state)
	  SetItemFlag(item, itemDeaf);
     else
	  ResetItemFlag(item, itemDeaf);
     
     SetItemFlag(item, itemChanged);
     SetMenuFlag(menu, menuItemChanged);
     if (ItemIsInitialItem(menu, item))
	  ClearInitialItem(menu);
     Leave(TRUE)
}

/*
 * SetItemDisable sets the disable state of item "n" of the menu to "state".
 */
Boolean SetItemDisable(menu, item, state)
Menu *menu;
MenuItem *item;
int state;
{
     Entry("SetItemDisable")

     if (TestItemFlag(item,itemDisabled) == state) /* Exit if unchanged */
	  Leave (True)
     
     if(state)
     {
	  SetItemFlag(item, itemDisabled);
	  /* if disabled item is currently initial item, null initial item */
	  if (ItemIsInitialItem(menu, item))
	       ClearInitialItem(menu);
     }
     else
	  ResetItemFlag(item, itemDisabled);
     
     SetItemFlag(item, itemChanged);
     SetMenuFlag(menu, menuItemChanged);
     
     Leave(TRUE)
}

MenuItem *Display_Menu(menu, parent, x, y)
Menu *menu;
Menu *parent;
int x;
int y;
{
     MenuItem *item;
     
     Entry("Display_Menu")

     if (MenuIsNull(menu))
	  Leave(FALSE)
     
     MenuParent(menu) = parent;
     
     if (MenuIsNull(parent))
	  MenuNested(menu) = 0;
     else
	  MenuNested(menu) = MenuNested(parent) + 1;
     if (!MenuWindow(menu) || TestMenuFlag(menu,
        (menuChanged | menuItemChanged))) {
	  if (!SetupMenuWindow(menu))
	       Leave(NULL)
	  Generate_Menu_Entries(menu);
     }
     if (TestOptionFlag(menu,savebits) &&
	 (MenuSavedPixmap(menu) != (Pixmap) 0))
	  SetInputMask(menu, MenuIgnoreMask);
     else
	  SetInputMask(menu, ExposureMask);
     if (!(item = MoveMenu(menu, x, y)) || !MapMenu(menu))
	  Leave(FALSE)
     Draw_Menu(menu);
     
     Leave(item)
}

void Undisplay_Menu(menu)
Menu *menu;
{
     Entry("Undisplay_Menu")

     if (MenuIsNull(menu))
	  Leave_void
     
     MenuParent(menu) = NULLMENU;
     MenuNested(menu) = 0;
     
     UnmapMenu(menu);
     Leave_void
}

void MenuInvert(menu, item)
Menu *menu;
MenuItem *item;
{
     Entry("MenuInvert")

     XFillRectangle(dpy,
		    ItemWindow(item),
		    MenuInvertGC(menu),
		    0, 0, 
		    MenuWidth(menu),
		    item->itemHeight);
     Leave_void
}

/*
 * Recalculate all of the various menu and item variables.
 */
Boolean Recalc_Menu(menu)
Menu *menu;
{
     MenuItem *item;
     int max, height, fontheight, boldfontheight, nitems;
     /* Gets set to false first time we see an item with text */
     unsigned int menuIsPictures = TRUE;
     
     Entry("Recalc_Menu")

     /*
      * We must have already gotten the menu font.
      */
     if(!MFontInfo)
	  Leave(FALSE)
     /*
      * Initialize the various max width variables.
      */
     fontheight = MFontInfo->ascent + MFontInfo->descent + 2;
     boldfontheight = MBoldFontInfo->ascent + MBoldFontInfo->descent + 2;
     height = nitems = 0;
     MenuMaxTextWidth(menu) = 0;
     /*
      * The item height is the maximum of the font height and the
      * checkbox height, unless we find out that it's a pixmap (in which
      * case, most of these values are overridden).
      */
     max = fontheight;
     if(checkMarkHeight > max)
	  max = checkMarkHeight;
     
     max += 2*MItemBorder;
     max = MakeEven(max);
     	   
     /*
      * Go through the menu item list.
      */
     for(item = MenuItems(menu) ; item ; item = ItemNext(item)) {
	  if (item->itemBackground)	/* pixmap is static size */
	       max = item->itemHeight;
	  else {
	       if (ItemIsDeaf(item))	/* It's a label, use bold info */
		    max = boldfontheight;
	       else
		    max = fontheight;
	  }
	  if (checkMarkHeight > max)
	       max = checkMarkHeight;
	  if (!item->itemBackground) {
	       max += 2 * MItemBorder;
	       max = MakeEven(max);
	       item->itemHeight = max;
	  }
	  height += max;
	  nitems++;
	  /*
	   * Check the text width with the max value stored in
	   * menu.
	   */
	  if (!item->itemBackground) {
	       if ((ItemTextWidth(item) = XTextWidth(ItemIsDeaf(item) ?
						     MBoldFontInfo
						     : MFontInfo,
						     ItemText(item),
						     strlen (ItemText(item))))
                  > MenuMaxTextWidth(menu))
                    MenuMaxTextWidth(menu) = ItemTextWidth(item);
	       menuIsPictures = FALSE;
          }
          /* ItemTextWidth is really pixmap size. Yick. */
	  else {
               if (ItemTextWidth(item) > MenuMaxTextWidth(menu))
                    MenuMaxTextWidth(menu) = ItemTextWidth(item);
          }

     }
     /*
      * Set the menu height and then set the menu width.
      */
     MenuHeight(menu) = height;
     menu->avgHeight = height / nitems;

     if (menuIsPictures == TRUE)
	  MenuWidth(menu) = MenuMaxTextWidth(menu) + (2 * MItemBorder);
     else {
	  MenuWidth(menu) = 4 * MenuItemPad(menu) + MenuMaxTextWidth(menu) +
	       checkMarkWidth + arrow_width + (2 * MItemBorder);
     }
     MenuItemWidth(menu) = MenuWidth(menu) - (2 * MItemBorder);
     Leave(TRUE)
}

/*
 * Figure out where to popup the menu, relative to the where the button was
 * pressed.
 * Returns pointer to initial item to warp to.
 */
static MenuItem *MoveMenu(menu, ev_x, ev_y)
Menu *menu;
int ev_x, ev_y;
{
     int x, y;
     int total_width, total_height;
     int offset;
     MenuItem *item;
     Window junk;
     int r_x, foo, state;
     
     Entry("MoveMenu")

     /*
      * Get the coordinates of the mouse when the button was pressed.
      */
     
     total_width = MenuWidth(menu) + 2 * MenuBorderWidth(menu);
     total_height = MenuHeight(menu) + 2 * MenuBorderWidth(menu);

     XQueryPointer(dpy, RootWindow(dpy, scr), &junk, &junk, &r_x, &y,
		   &foo, &foo, &state); 
     x = ev_x - MenuItemPad(menu);
     if (x < 0)
	  x = 0;
     else if (TestOptionFlag(menu, rightoffset) &&
	      !MenuIsNull(MenuParent(menu)))
     {
	  /* check whether parent is close to right edge... */
	  /* "too close" means that child would leave < delta of its parent */
	  /* visible to its left.                                           */
	  if (TestOptionFlag(menu, bigoffset))
	  {
	       if (MenuX(MenuParent(menu)) + MenuWidth(MenuParent(menu)) > 
		   DisplayWidth(dpy, scr) - total_width)
		    x = MenuX(MenuParent(menu))
			 - total_width + 2*MenuBorderWidth(menu);
	  }
	  else
	  {
	       if (MenuX(MenuParent(menu)) + MenuDelta(MenuParent(menu)) > 
		   DisplayWidth(dpy, scr) - total_width)
	       {
		    x = (MenuX(MenuParent(menu)) + MenuWidth(MenuParent(menu)) +
			 2 * MenuBorderWidth(MenuParent(menu))
			 - total_width - MenuDelta(menu));
	       }
	  }
     }
     if (x + total_width >
	 DisplayWidth(dpy, scr))
	  x = DisplayWidth(dpy, scr)
	       - total_width;
     
     /*
      * If we have an initial item, try to popup the menu centered
      * vertically within this item.
      */
     if(MenuHasInitialItem(menu)) {
	  int tmp_y;

	  /*
	   * Look through the item list. "y" is the vertical position
	   * of the top of the current item and "n" is the item number.
	   */
	  offset = MenuBorderWidth(menu);
	  for(item = MenuItems(menu) ; ;) {
	       /*
		* On finding the initial item, center within this item.
		*/
	       if (ItemIsInitialItem(menu, item)) {
		    y -= offset;
		    break;
	       }
	       else
		    offset += item->itemHeight;
	       /*
		* If we run out of items, turn off the initial item
		* and treat this as if no initial item.
		*/
	       if(!(item = ItemNext(item))) {
		    ClearInitialItem(menu);
		    break;
	       }
	  }
     }
     
     if (y < 0)
	  y = 0;
     else if (y + total_height >
	      DisplayHeight(dpy, scr))
     {
	  y = DisplayHeight(dpy, scr)
	       - (total_height + 1);
     }
     y = MakeEven(y) + 1;
     XMoveWindow(dpy, MenuWindow(menu), x, y);
     MenuX(menu) = x;
     MenuY(menu) = y;
     
     if (!ItemIsInitialItem(menu, item))
	  item = MenuItems(menu);
     Leave(item)
}


void PlacePointer(menu, item)
Menu *menu;
MenuItem *item;
{
     int y;
     
     Entry("PlacePointer")

     y = ItemGetMiddleY(item);
     
     XWarpPointer(dpy, None,
		  RootWindow(dpy, scr), 
		  0, 0, 0, 0, 
		  MenuX(menu) + MPad + (MenuWidth(menu) / 2), y);
     Leave_void
}

/*
 * Map the menu window.
 */
static Boolean MapMenu(menu)
Menu *menu;
{
     Entry("MapMenu")

     if (!TestMenuFlag(menu, menuMapped))
	  MenuItemHighlighted(menu) = NULLITEM;
     
     /*
      * Actually map the window.
      */
     
     if (TestOptionFlag(menu,savebits))
     {
	  if ((MenuSavedPixmap(menu) == (Pixmap) 0) ||
	      (MenuOldWidth(menu) != MenuWidth(menu)) ||
	      (MenuOldHeight(menu) != MenuHeight(menu)) ||
	      (MenuOldBorderWidth(menu) != MenuBorderWidth(menu)))
	  {
	       if (MenuSavedPixmap(menu) != (Pixmap) 0)
		    XFreePixmap(dpy, MenuSavedPixmap(menu));
	       
	       MenuSavedPixmap(menu) =
		    XCreatePixmap(dpy, 
				  RootWindow(dpy, scr), 
				  MenuWidth(menu) + 2*MenuBorderWidth(menu),
				  MenuHeight(menu) + 2*MenuBorderWidth(menu),
				  DefaultDepth(dpy, scr));
	       DBUG_5("Pixmaps","Created pixmap: width %d, height %d, depth %d\n",
		      MenuWidth(menu) + 2*MenuBorderWidth(menu),
		      MenuHeight(menu) + 2*MenuBorderWidth(menu),
		      DefaultDepth(dpy, scr)); 
	       MenuOldWidth(menu) = MenuWidth(menu);
	       MenuOldHeight(menu) = MenuHeight(menu);
	       MenuOldBorderWidth(menu) = MenuBorderWidth(menu);
	  }
	  else
	       SetInputMask(menu, ExposureMask);
	  
	  if (MenuSavedPixmap(menu) != (Pixmap) 0) { 
	       /*	XFillRectangle(dpy,
			MenuSavedPixmap(menu),
			SaveGC,
			0,0,
			MenuWidth(menu) + 2*MenuBorderWidth(menu),
			MenuHeight(menu) + 2*MenuBorderWidth(menu)); */
	       SetInputMask(menu, MenuIgnoreMask);
	       XCopyArea(dpy, 
			 RootWindow(dpy, scr),
			 MenuSavedPixmap(menu),
			 MenuNormalGC(menu),
			 MenuX(menu), 
			 MenuY(menu), 
			 (unsigned int) (MenuWidth(menu) + 2*MenuBorderWidth(menu)), 
			 (unsigned int) (MenuHeight(menu) + 2*MenuBorderWidth(menu)), 
			 0, 0);
	  }
	  else
	       SetInputMask(menu, ExposureMask);
	  
     }
     
     XRaiseWindow(dpy, MenuWindow(menu));
     XMapWindow(dpy, MenuWindow(menu));
     SetMenuFlag(menu, menuMapped);
     Leave(TRUE)
}

static void Generate_Menu_Entries (menu)
Menu *menu;
{
     MenuItem *item;
     
     Entry("Generate_Menu_Entries")

     for (item = MenuItems(menu); item; (item = ItemNext(item))) {
	  
	  if (ItemGenerator(item)) {
	       char *newText;
	       
	       (ItemGenerator(item)) (&newText, &ItemCallback(item));
	       SetItemText (menu, item, newText);
	  }
	  
	  if (ItemCheckproc(item))
	       SetItemCheck (menu, item, (ItemCheckproc(item))(menu,item));
     }
     Leave_void
}

/*
 * Draw the entire menu in the blank window.
 */
void Draw_Menu(menu)
Menu *menu;
{
     MenuItem *item;
     
     Entry("Draw_Menu")

     ResetMenuFlag(menu, menuChanged);
     /*
      * For each item in the list, first draw any check mark and then
      * draw the rest of it.
      */
     for(item = MenuItems(menu) ; item ; item = ItemNext(item)) {
	  if (TestOptionFlag(menu, savebits))
	  {
	       /* go ahead and draw it, don't wait for exposes */
	       Draw_Item(menu, item);
	  }
     }
     Leave_void
}

/*
 * Draw the item  at vertical position y.
 */
void Draw_Item(menu, item)
Menu *menu;
MenuItem *item;
{
     int y;  /* baseline */
     int x = MenuItemPad(menu);
     int x1 = 2 * MenuItemPad(menu) + checkMarkWidth;
     int pad;
     int high;
     XGCValues gcValues;
     GC theGC;

     Entry("Draw_Item")

     high = (MenuItemHighlighted(menu) == item);
     theGC = high ? MenuInvertGC(menu) : MenuNormalGC(menu);

     if (TestItemFlag(item, itemDisabled))
     {
	  gcValues.fill_style = FillOpaqueStippled;
	  XChangeGC(dpy, theGC, (GCFillStyle), &gcValues);
     }
     
     if (!item->itemBackground) {
	  XFillRectangle(dpy, ItemWindow(item),
			 (high) ? MenuNormalGC(menu) : MenuInvertGC(menu),
			 0, 0, MenuWidth(menu), item->itemHeight);
	  
	  /*
	   * Draw the check mark, possibly dimmed, wherever is necessary.
	   */
     }
     if(TestItemFlag(item, itemChecked)){
	  XCopyArea (dpy, MenuCheckmarkPixmap(menu),
		     ItemWindow(item),
		     theGC,
		     0, 0, checkMarkWidth, checkMarkHeight,
		     (int) x, (item->itemHeight - checkMarkHeight) / 2);
     }
     /* Draw submenu indicator arrow */
     if(ItemSubmenu(item)) {
	  XCopyArea (dpy, MenuArrowPixmap(menu),
		     ItemWindow(item),
		     theGC,
		     0, 0,
		     arrow_width, arrow_height,
		     (int) (x + MenuItemWidth(menu) -
			    arrow_width - MenuItemPad(menu)),
		     (item->itemHeight - arrow_height) / 2 - 1);
     }
     /*
      * Draw the text, centered vertically.
      */
     if (!item->itemBackground) {
	  if (!TestItemFlag(item, itemDeaf)) {
	       pad = (item->itemHeight - 
		      (MFontInfo->ascent + MFontInfo->descent)) / 2;
	       y = item->itemHeight - pad - MFontInfo->descent;
	       
	       XDrawImageString(dpy, ItemWindow(item),
				theGC,
				x1, y, ItemText(item), ItemTextLength(item));
	       if (TestItemFlag(item, itemDisabled))
	       { 
		    gcValues.fill_style = FillSolid;
		    XChangeGC(dpy, theGC,
			      (GCFillStyle), &gcValues);
	       }
	       
	  }
	  else {
	       pad = (item->itemHeight - (MBoldFontInfo->ascent +
					  MBoldFontInfo->descent)) / 2;
	       y = item->itemHeight - pad - MBoldFontInfo->descent;
	       XDrawImageString(dpy, ItemWindow(item), menu->boldGC, x1, y,
				ItemText(item), ItemTextLength(item));
	  }
     }
     Leave_void
}


/*
 * UnmapMenu() unmaps a menu, if it is currently mapped.
 */
static void UnmapMenu(menu)
Menu *menu;
{
     Entry("UnmapMenu")

     if(!menu || !(TestMenuFlag(menu, menuMapped)))
	  Leave_void
     XUnmapWindow(dpy, MenuWindow(menu));
     
     if (TestOptionFlag(menu, savebits))
     {
	  if (MenuSavedPixmap(menu))
	       XCopyArea (dpy,
			  MenuSavedPixmap(menu),
			  RootWindow (dpy, scr),
			  MenuNormalGC(menu),
			  0, 0, 
			  MenuWidth(menu) + 2*MenuBorderWidth(menu),
			  MenuHeight(menu) + 2*MenuBorderWidth(menu),
			  MenuX(menu), MenuY(menu));
	  
     }
     ResetMenuFlag(menu, menuMapped);
     Leave_void
}


static Boolean SetupMenuWindow (menu)
Menu *menu;
{
     int changed = TestMenuFlag(menu, (menuChanged | menuItemChanged));
 
     Entry("SetupMenuWindow")

     if (contexts_created == FALSE) {
	  contexts_created = TRUE;
	  Menu_context = XUniqueContext(); 
	  Item_context = XUniqueContext();
     }
     
     /*
      * If the entire menu has changed, throw away any saved pixmap and
      * then call RecalcMenu().
      */
     
     if(changed & menuChanged) {
	  if(!Recalc_Menu(menu))
	       Leave(FALSE)
	  changed &= ~menuItemChanged;
     }
     
     if(!MenuWindow(menu)) {
	  static unsigned long valuemask =
	       CWOverrideRedirect | CWBorderPixel | CWBackPixel;
	  XSetWindowAttributes attributes;
	  
	  attributes.override_redirect = True;
	  attributes.border_pixel = MBorder;
	  attributes.background_pixel = MBackground;
	  if (SaveUnder) {
	       attributes.save_under = True;
	       valuemask |= CWSaveUnder;
	  }

	  if((MenuWindow(menu) =
	      XCreateWindow(dpy, 
			    RootWindow(dpy, scr),
			    0, 0,
			    MenuWidth(menu), MenuHeight(menu),
			    MenuBorderWidth(menu),
			    DefaultDepth(dpy, scr),
			    InputOutput, 
			    DefaultVisual(dpy, scr),
			    valuemask, &attributes)
	      ) == (Window)0)
	       Leave(FALSE)
	  else if (SetupItems(menu) == FALSE)
	       Leave(FALSE)

	  XSaveContext(dpy, MenuWindow(menu), Menu_context, (char *) menu);
	  XMapSubwindows(dpy, MenuWindow(menu));
	  
	  XDefineCursor(dpy, MenuWindow(menu), MenuCursor(menu));
     }
     else if(changed & menuChanged) {
	  XResizeWindow(dpy, MenuWindow(menu),
			MenuWidth(menu), MenuHeight(menu));
	  if (SetupItems(menu) == FALSE)
	       Leave(FALSE)
	  XMapSubwindows(dpy, MenuWindow(menu));
     }
     Leave(TRUE)
}

static Boolean SetupItems(menu)
Menu *menu;
{
     int y;
     MenuItem *item;
     int changed = TestMenuFlag(menu, (menuChanged | menuItemChanged));

     Entry("SetupItems")
     
     for (item = MenuItems(menu), y = 0; item;
	  y += item->itemHeight, item = ItemNext(item)) {
	  if (!ItemWindow(item)) {
	       static unsigned long valuemask =
		    (CWOverrideRedirect | CWBorderPixel | CWBackPixel);
	       XSetWindowAttributes attributes;

	       attributes.override_redirect = True;
	       attributes.border_pixel = MBorder;
	       attributes.background_pixel = MBackground;

	       if((ItemWindow(item) =
		   XCreateWindow(dpy, 
				 MenuWindow(menu),
				 0, y, 
				 MenuItemWidth(menu), 
				 item->itemHeight - (2 * MItemBorder), 
				 MItemBorder,
				 DefaultDepth(dpy, scr),
				 InputOutput, 
				 DefaultVisual(dpy, scr),
				 valuemask, &attributes)
		   ) == (Window) 0)
		    Leave(FALSE)
	       if (item->itemBackground) {
		    XSetWindowBackgroundPixmap(dpy, ItemWindow(item),
					       item->itemBackground);
		    XFreePixmap(dpy, item->itemBackground);
	       }
	       ItemMenu(item) = menu;
	       XSaveContext(dpy, ItemWindow(item), Item_context,
			    (char *) item);
	       XDefineCursor(dpy, ItemWindow(item),
			     MenuCursor(menu));
	  }
	  
	  else if (changed & menuChanged) {
	       XResizeWindow(dpy, ItemWindow(item),
			     MenuItemWidth(menu), 
			     item->itemHeight - (2 * MItemBorder));
	       XMoveWindow(dpy, ItemWindow(item), 0, y);
	  }
     }
     Leave(TRUE)
}

static void SetInputMask(menu, mask)
Menu *menu;
unsigned int mask;
{
     MenuItem *item;
 
     Entry("SetInputMask")

     XSelectInput(dpy, MenuWindow(menu),
		  (mask | MenuEventMask));
     for(item = MenuItems(menu) ; item ; item = ItemNext(item))
     {
	  if (TestItemFlag(item, itemDisabled) || TestItemFlag(item, itemDeaf))
	       XSelectInput(dpy, ItemWindow(item),
			    (mask | MenuIgnoreMask));
	  else if (ItemIsLeaf(item))
	       XSelectInput(dpy, ItemWindow(item),
			    (mask | NormalItemEventMask));
	  else
	       XSelectInput(dpy, ItemWindow(item),
			    (mask | SubmenuItemEventMask));
     }
     Leave_void
}

MenuItem *MenuItemByData(menu, data)
Menu* menu;
pointer data;
{
     MenuItem *item;
     
     Entry("MenuItemByData")

     for (item = MenuItems(menu);
	  !ItemIsNull(item) && (ItemData(item) != data);
	  item = ItemNext(item));
     
     Leave(item)
}   

MenuItem *MenuItemByName (menu, name)
Menu *menu;
char *name;
{
     MenuItem *item;

     Entry("MenuItemByName")

     for (item = MenuItems(menu); item; item = ItemNext(item))
	  if (strcmp (name, ItemText(item)) == 0)
	       Leave(item)
     Leave(NULLITEM)
}

MenuItem *MenuGetItem(menu, window) 
Menu *menu;
Window window;
{    
     MenuItem *foo = 0;
     Window tmpwindow = window;

     Entry("MenuGetItem")

     if (!menu || !window)
	  Leave(NULLITEM)
     if (window == menu->menuWindow)
	  if (menu->highlightedItem)
	       tmpwindow = menu->highlightedItem->itemWindow;
     XFindContext(dpy, tmpwindow, Item_context, &foo);
     Leave(foo)
}

Menu *MenuGetMenu(menu, window)
Menu *menu;
Window window;
{
     Menu *bar = 0;
 
     Entry("MenuGetMenu")

     XFindContext(dpy, window, Menu_context, &bar);
     Leave(bar)
}

int ItemGetMiddleY(item)
MenuItem *item;
{
     Window child;
     XWindowAttributes attributes;
     int x, y;
 
     Entry("ItemGetMiddleY")

     XGetWindowAttributes(dpy, ItemWindow(item), &attributes);
     XTranslateCoordinates(dpy,
			   MenuWindow(ItemMenu(item)), 
			   RootWindow(dpy, scr),
			   attributes.x, attributes.y, 
			   &x, &y, &child);
     Leave((item->itemHeight / 2) + y)
}

void SetInitialItem(menu, item)
Menu *menu;
MenuItem *item;
{
     Entry("SetInitialItem")

     if (MenuHasInitialItem(menu)) {
	  if (strlen(MenuInitialItemText(menu)) < strlen(ItemText(item)))
	       MenuInitialItemText(menu) =
		    realloc(MenuInitialItemText(menu), 
			    strlen(ItemText(item)) + 1);
     }
     else
	  MenuInitialItemText(menu) =
	       allocate(char,(strlen(ItemText(item)) + 1));
     strcpy(MenuInitialItemText(menu), ItemText(item));
     Leave_void
}

void ClearInitialItem(menu)
Menu *menu;
{
     Entry("ClearInitialItem")

     if (MenuHasInitialItem(menu)) {
	  free(MenuInitialItemText(menu));
	  MenuInitialItemText(menu) = (char *) NULL;
     }
     Leave_void
}

MenuItem *GetInitialItem(menu)
Menu *menu;
{
     MenuItem *item;
     
     Entry("GetInitialItem")

     if (MenuHasInitialItem(menu)) {
	  for(item = MenuItems(menu) ; item ; item = ItemNext(item)) {
	       if (ItemIsInitialItem(menu, item))
		    Leave(item)
	  }
     }
     Leave((MenuItem *) NULL)
}


/* Some utility functions */

void Retch(s, p1, p2, p3, p4)
char *s;
long p1, p2, p3, p4;
{
     char buffer[1024];
     
     sprintf(buffer, s, p1, p2, p3, p4);
     fprintf(stderr, "Error in %s %s\n", curr_rtn(), buffer);
     return;
}
