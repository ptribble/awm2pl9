
#ifndef lint
     static char sccs_id[] = "@(#)track_menu.c	2.1 12/16/87  Siemens Corporate Research and Support, Inc.";
#endif



/* 
  RTL Menu Package Version 1.0
  by Joe Camaratta and Mike Berman, Siemens RTL, Princeton NJ, 1987
  
  track_menu.c: bring up menus and track the mouse
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
 * The menu package will break if you don't define this, but
 * it's there in case you want to see just how and where the
 * "eventstack" stuff is used and, if necessary, replace it.
 */
#define SAVE_EVENTS


#include <stdio.h>
#include "X11/Xlib.h"
#include "X11/cursorfont.h"
#include "arrow_icon.h"
#include "null_icon.h"
#include "menu.h"
#include "menu.def.h"
#include "menu.ext.h"
#include "dbug.h"
#ifdef SAVE_EVENTS
#include "eventstack.h"
#endif

#define MIN(x,y) (((x) <= (y))? x:y)
#define MAX(x,y) (((x) >= (y))? x:y)

#define CLICK_TIME 290 /* in milliseconds */

#define CursorLockMask (ButtonReleaseMask | ExposureMask)

/* Event macros */

#define EventGetXCoord(rep) ((rep).xmotion.x)
#define EventGetYCoord(rep) ((rep).xmotion.y)
#define EventType(rep) ((rep).type)
#define EventXWindow(rep) ((rep).xcrossing.window)
#define EventXTime(rep) ((rep).xcrossing.time)
#define EventXMode(rep) ((rep).xcrossing.mode)
#define EventXRootX(rep) ((rep).xcrossing.x_root)
#define EventXRootY(rep) ((rep).xcrossing.y_root)
#define EventXDetail(rep) ((rep).xcrossing.detail)
#define EventMWindow(rep) ((rep).xmotion.window)
#define EventMTime(rep) ((rep).xmotion.time)
#define EventButton(rep) ((rep).xbutton.button)
#define EventBWindow(rep) ((rep).xbutton.window)
#define EventBTime(rep) ((rep).xbutton.time)
#define EventEX(rep) ((rep).xexpose.x)
#define EventEY(rep) ((rep).xexpose.y)
#define EventEWidth(rep) ((rep).xexpose.width)
#define EventEHeight(rep) ((rep).xexpose.height)
#define PointerEvent(rep) \
     ((EventType(rep) == ButtonPress) || \
      (EventType(rep) == ButtonRelease) || \
      (EventType(rep) == MotionNotify) || \
      (EventType(rep) == EnterNotify) || \
      (EventType(rep) == LeaveNotify) || \
      (EventType(rep) == FocusIn) || \
      (EventType(rep) == FocusOut))
#define KeyEvent(rep) \
     ((EventType(rep) == KeyPress) || (EventType(rep) == KeyRelease))
/* Possible states for the state machine */
typedef enum
{
     Initial,      /* Inside a submenu, but not any item */
     CheckTrigger, /* Inside an item that has submenu, checking for pullright */
     Leaf,         /* Inside an item with no submenu */
     Exit,         /* Preparing to exit */
     LevelControl  /* Not in any submenu, waiting to enter something */
     } State;

State InitialState(), CheckTriggerState(), LeafState(), LevelControlState(),
     GetItemState();
Boolean EventNotSignificant(), PushSubmenu();

void OutputEvent(), GetNextSignificantEvent(), PopSubmenu(), 
     Highlight(), Unhighlight(), DisplayInitialMenus(), LockCursor(),
     TossExtraMoves(), UnlockCursor();

void ProcessExposeEvents();

void SaveTest();

MenuItem *MenuGetItem();
Menu *MenuGetMenu();

/* global state variables */

static MenuItem *current_item;
static Menu *current_menu;
static Window root_window;
extern Display *dpy;
extern int scr;
static int level;            /* submenu level */
static Time button_time;     /* time button press invoked */
static Cursor wait_cursor = None;  /* empty cursor for lock state */
static Boolean click_allowed;
static Boolean lock_event_mask, unlock_event_mask;

extern Boolean Autoselect;
extern int MDelta;

#ifdef SAVE_EVENTS
static struct Ev_q *ev_save = 0;
#endif


MenuItem *TrackMenu(root_menu, root_x, root_y, 
		    init_button, root, buttime)
Menu *root_menu;    /* Pointer to root menu requested to pop up   */
int root_x, root_y; /* Position to start menu                     */
int init_button;    /* The # of button used to pop up menu        */
Window root;        /* Window label for parent of menu            */
Time buttime;       /* timestamp for button (or 0, if CLICK == 0) */
{
     State CurrentState = LevelControl;
     XEvent Event_Reply;
     int open_x;
     Boolean selected = FALSE;
     MenuItem *selected_item;
     
     Entry("TrackMenu")
     
     /* Initialize globals */
     
     button_time = buttime;
     root_window = root;
     level = 0;
     current_menu = root_menu;
     click_allowed = (TestOptionFlag(current_menu, clickokay))? TRUE : FALSE;
     unlock_event_mask = (TestOptionFlag(current_menu, savebits))?
	  MenuEventMask : (MenuEventMask | ExposureMask);
     lock_event_mask = (TestOptionFlag(current_menu, savebits))?
	  CursorLockMask : (CursorLockMask | ExposureMask);
     
     /* If not already done, set up the null cursor for lock state */
     if (wait_cursor == None)
     {
	  Pixmap wc_pixmap;
	  XColor fg, bg;
	  
	  wc_pixmap = XCreateBitmapFromData (dpy, root_window,
					     null_icon_bits,
					     null_icon_width, null_icon_height);
	  wait_cursor = XCreatePixmapCursor (dpy, wc_pixmap, wc_pixmap,
					     &fg, &bg, 1, 1);
     }
     
     
     /* Block all other action by grabbing the server */
     /*    XGrabServer (dpy); */
     /* Don't think we need to grab the server... so for now, we won't */
     
#ifdef SAVE_EVENTS     
     /* Get the present state, so it can be restored later */
     /* Any events on the queue when we start get saved now, restored later */
     SaveEvents (dpy, &ev_save, ~(unsigned long) ButtonReleaseMask);
#endif
     
     LockCursor(root_window);
     if (!(current_item =
	   Display_Menu(current_menu, NULLMENU, root_x, root_y)))
     {
	  CurrentState = Exit;
     }
     /*
      * First item is a label and autoselect is on, so we want
      * to push on to the first "real" item.
      */
     if (ItemIsDeaf(current_item) && Autoselect)
	  current_item = current_item->nextItem;
     LockCursor(ItemWindow(current_item));
     open_x = root_x;
     
     /* Push to appropriate previous item, if any */
     while (MenuHasInitialItem(current_menu) && (CurrentState != Exit))
     {
	  current_item = GetInitialItem(current_menu);
	  ClearInitialItem(current_menu);
	  
	  /* if the initial item can't be selected, take first in list */
	  if (ItemIsNull(current_item) || ItemIsDisabled(current_item))
	  {
	       current_item = MenuItems(current_menu);
	       break;
	  }
	  else if (ItemIsLeaf(current_item)) /* then we're done */
	       break;
	  else
	  {
	       open_x += ItemGetArrowPosition(current_item);
	       if (!ItemIsDeaf(current_item))
		    Highlight(current_item);
	       TossExtraMoves(ItemWindow(current_item));
	       (void)PushSubmenu(open_x);
	  }
     }
     ProcessExposeEvents();
     if (CurrentState != Exit)
	  CurrentState = (ItemIsLeaf(current_item)) ? Leaf : CheckTrigger;
     if (!ItemIsDeaf(current_item))
	  Highlight(current_item);
     XSync (dpy, 0);  /* get release click, if it's in queue */
#ifdef SAVE_EVENTS
     DisposeEvents(dpy, (PointerMotionMask | EnterWindowMask |
			 LeaveWindowMask | ExposureMask));
#endif
     LockCursor(ItemWindow(current_item));
     PlacePointer(current_menu,current_item); 
     UnlockCursor();
     	
     /* State Machine */
     
     while (CurrentState != Exit)
     {
	  GetNextSignificantEvent(&Event_Reply, init_button);
	  switch (CurrentState)
	  {
	  case LevelControl:
	       CurrentState = LevelControlState(Event_Reply);
	       break;
	  case Initial:
	       CurrentState = InitialState(Event_Reply);
	       break;
	  case CheckTrigger:
	       CurrentState = CheckTriggerState(Event_Reply);
	       break;
	  case Leaf:
	       CurrentState = LeafState(Event_Reply, &selected);
	       break;
	  default:
	       Retch("(RTLmenu) YOW! Unknown State! (%d)\n",
		     CurrentState);
	       CurrentState = Exit;
	       break;
	  }
     }
     /* Clean up and exit */
     
     selected_item = (selected)? current_item : NULLITEM;
     while (level)
     {
	  if (selected)
	       SetInitialItem(current_menu, current_item);
	  PopSubmenu();
     }
     if (selected)
     {
	  SetInitialItem(current_menu, current_item);
     }
     
     Undisplay_Menu(current_menu);
     UnlockCursor();
     XUngrabPointer(dpy, CurrentTime);
     
     /* Throw out any left over events from menu world */
     /*    if (TestOptionFlag(current_menu, savebits)) {
	   XSync(dpy,1); 
	   XUngrabServer(dpy, CurrentTime);  add this if grab added! 
	   }
	   else
	   XSync(dpy,0);*/
     
     /* Push back any events that were lying around when menus started */
     
     XFlush(dpy);
#ifdef SAVE_EVENTS
     DisposeEvents(dpy, (PointerMotionMask | EnterWindowMask |
			 LeaveWindowMask | ExposureMask));
     RestoreEvents(dpy, &ev_save);
#endif
     Leave(selected_item)
}

/* Used for debugging */

void OutputEvent(Event_Reply)
XEvent Event_Reply;
{
     Entry("OutputEvent")

     switch (EventType(Event_Reply))
     {
     case ButtonPress:
     case ButtonRelease:
	  DBUG_5("RTLmenu","Button Press/Release, button %d on window %d at time %d\n",
		 EventButton(Event_Reply), EventBWindow(Event_Reply), 
		 EventBTime(Event_Reply));
	  break;
     case MotionNotify:
	  DBUG_5("RTLmenu","Motion Notify on window %d at time %d, x=%d\n", 
		 EventMWindow(Event_Reply), EventMTime(Event_Reply),
		 EventGetXCoord(Event_Reply));
	  break;
     case EnterNotify:
	  DBUG_4("RTLmenu","Enter Notify on window %d at time %d\n",
		 EventXWindow(Event_Reply), EventXTime(Event_Reply));
	  break;
     case LeaveNotify:
	  DBUG_4("RTLmenu","Leave Notify on window %d at time %d\n",
		 EventXWindow(Event_Reply), EventXTime(Event_Reply));
	  break;
     default:
	  DBUG_3("RTLmenu","Unexpected event type %d\n", EventType(Event_Reply));
	  break;
     }
     Leave_void
}

static Boolean locked = FALSE;

/* Lock the cursor: make it disappear, and ignore events it generates.  */
/* Optionally, confine it to a single window.                           */
/* (Using "None" for confine_window doesn't confine it.    )            */
void LockCursor(confine_window)
Window confine_window;
{
     int result;
     
     Entry("LockCursor")
     
     locked = TRUE;
     result = XGrabPointer(dpy,
			   RootWindow(dpy, MenuScreen(current_menu)),
			   True, lock_event_mask, GrabModeSync, 
			   GrabModeAsync, confine_window,
			   wait_cursor, CurrentTime);
     DBUG_3("RTLmenu","Lock Cursor grab = %d\n",result);
     Leave_void
}

/* Unlock (and unconfine) the cursor.  If cursor lock is not set,    */
/* this does nothing.                                                */

void UnlockCursor()
{
     int result;
     
     Entry("UnlockCursor")
     
     if (locked)
     {
	  locked = FALSE;
	  result = XGrabPointer(dpy, 
				RootWindow(dpy, MenuScreen(current_menu)),
				True,  unlock_event_mask,
				GrabModeAsync, GrabModeAsync, None,
				MenuCursor(current_menu), CurrentTime);
	  DBUG_3("RTLmenu","Unlock Cursor grab = %d\n",result);	    
     }
     Leave_void
}

/* Keep getting the X events, until finding one that may be interesting */
/* to the operation of the state machine. */

void GetNextSignificantEvent(Event_Reply,init_button)
XEvent *Event_Reply;
int init_button;		/* the button that initiated the menu */
{
     XEvent Next_Event_Reply;
     Boolean InsignificantEvent = True;
     
     Entry("GetNextSignificantEvent")
     
     /* Loop as long as any of a number of "insignificant" events */
     /* are found; when the event no longer matches one of the tests, */
     /* it is assumed to be "significant" and returned.*/
     do
     {
	  XNextEvent(dpy, Event_Reply);
	  DBUG_EXECUTE("RTLmenu", OutputEvent(*Event_Reply));
	  
	  /* If this event is an "enter", check whether there is a   */
	  /* "leave" for the same window already in the queue,       */
	  /* immediately following it; if so, throw them both out    */
	  /* and get the next event                                  */
	  /* NOTE: might try to look further ahead, but this is      */
	  /* tricky because other events might intervene.            */
	  
	  if ((EventType(*Event_Reply) == EnterNotify) &&
	      (EventXMode(*Event_Reply) == NotifyNormal) &&
	      (QLength(dpy) > 0) &&
	      (MenuGetMenu(current_menu, EventXWindow(*Event_Reply))
	       != current_menu))
	  {
	       XPeekEvent(dpy, &Next_Event_Reply);
	       if ((EventType(Next_Event_Reply) == LeaveNotify) &&
		   (EventXMode(Next_Event_Reply) == NotifyNormal) &&
		   (EventXWindow(Next_Event_Reply) == EventXWindow(*Event_Reply)))
	       {
		    DBUG_2("RTLmenu","TOSS: Enter/leave pair.\n");
		    XNextEvent(dpy, Event_Reply);
		    XNextEvent(dpy, Event_Reply);
	       }
	  }
#ifdef SAVE_EVENTS
	  if (EventNotSignificant(*Event_Reply, init_button))
	  {
	       if (!(PointerEvent(*Event_Reply) || KeyEvent(*Event_Reply)
		     || EventType(*Event_Reply) == Expose))
	       {
		    /* might be significant elsewhere -- save it for later */
		    AddEventToStore(&ev_save, *Event_Reply);
	       }
	  }
	  else
#else
	       if (!EventNotSignificant(*Event_Reply, init_button))
#endif
		    InsignificantEvent = FALSE;
     }
     while (InsignificantEvent);
     
     DBUG_2("RTLmenu","--->");
     Leave_void
}

/* Check whether the event matches one of the events considered */
/* "not significant".*/
Boolean EventNotSignificant(Event_Reply, init_button)
XEvent Event_Reply;
int init_button;
{
     Entry("EventNotSignificant")

     /* Insignificant if not in following list */
     Leave(!((EventType(Event_Reply) == ButtonRelease) ||
	       (EventType(Event_Reply) == ButtonPress) ||
	       (EventType(Event_Reply) == MotionNotify) ||
	       (EventType(Event_Reply) == EnterNotify) ||
	       (EventType(Event_Reply) == Expose) ||
	       (EventType(Event_Reply) == LeaveNotify))
	     ||
	     /* Insignificant if leave or enter is not "Normal"  */
	     (((EventType(Event_Reply) == LeaveNotify) ||
	       (EventType(Event_Reply) == EnterNotify)) &&
	      (EventXMode(Event_Reply) != NotifyNormal))
	     ||
	     /* Insignificant if hit button other than initial one */
	     ((EventType(Event_Reply) == ButtonRelease) &&
	      (EventButton(Event_Reply) != init_button))
	     ||
	     /* Insignificant if tail end of a click -- and clicks allowed */
	     (click_allowed &&
	      (EventType(Event_Reply) == ButtonRelease) &&
	      (EventBTime(Event_Reply) - button_time < CLICK_TIME))
	     )
}

State LevelControlState(rep)
XEvent rep;
{
     State next_state;
     Menu *entered_menu;
     MenuItem *entered_item;
     
     Entry("LevelControlState")
     switch (EventType(rep))
     {
     case MotionNotify:
     case LeaveNotify: 
	  next_state = LevelControl; /* loop back to this state */
	  break;
     case EnterNotify:
	  /* Decide whether we've entered a menu window or item window */
	  entered_menu = MenuGetMenu(current_menu, EventXWindow(rep));
	  entered_item = MenuGetItem(current_menu,EventXWindow(rep));
	  
	  if ((MenuIsNull(entered_menu)) && (ItemIsNull(entered_item)))
	       /* Must be some other window; carry on */
	       next_state = LevelControl;
	  else if (!ItemIsNull(entered_item) &&
		   MenuIsDisplayed(ItemMenu(entered_item)))
	  {
	       /* we entered an item, but not a window. This should only happen */
	       /* when we stayed in the parent of the current submenu.  So,     */
	       /* Pop that submenu and get to the item.                         */
	       if (level)
	       {
		    LockCursor(ItemWindow(entered_item));
		    PopSubmenu();
		    ProcessExposeEvents();
		    UnlockCursor();
		    current_item = entered_item;
		    Highlight(current_item);
		    next_state = GetItemState(rep);
	       }
	       else
	       { 
		    Retch("(RTLmenu) Tried to pop the root menu...\n");
		    next_state = Exit;
	       }
	  }
	  
	  else if (!MenuIsNull(entered_menu)&&
		   MenuIsDisplayed(entered_menu))
	  {
	       /* entered a menu that is displayed */
	       while ((current_menu != entered_menu) && level)
		    /* drop down the menu that was entered */
		    PopSubmenu();
	       ProcessExposeEvents();
	       UnlockCursor();
	       if (current_menu == entered_menu)
		    next_state = Initial;
	       else
	       {
		    next_state = Exit;
		    Retch("(RTLmenu) Couldn't find the menu I entered!!\n");
	       }
	  }
	  else 
	       next_state = LevelControl;
	  break;
     case ButtonRelease:
	  next_state = Exit;
	  break;
     default:
	  Retch("RTLmenu","YOW! Unexpected event! (%d)\n", rep.type);
	  next_state = Exit;
	  break;
     }
     Leave(next_state)
}

/* Figure out the status of the item we've just entered */
State GetItemState(rep)
XEvent rep;
{
     int open_x;
     State next_state;
     
     Entry("GetItemState")
     if (ItemIsNull(current_item))
     {
	  Retch("(RTLmenu) null current item!");
	  next_state = Exit;
     }
     else if (MenuIsNull(current_menu))
     {
	  Retch("(RTLmenu) null current menu!");
	  next_state = Exit;
     }
     else if (ItemIsLeaf(current_item))
     {
	  if (MenuHasInitialItem(current_menu))
	       ClearInitialItem(current_menu);
	  next_state = Leaf;
     }
     else if (EventGetXCoord(rep) >= (int)(ItemGetArrowPosition(current_item) - 4))
     {
	  /* entered item in "auto pop-up zone", i.e., over pull-right arrow. */
	  LockCursor(ItemWindow(current_item));
	  TossExtraMoves(ItemWindow(current_item));
	  if (PushSubmenu(EventXRootX(rep)))
	  {
	       LockCursor(ItemWindow(current_item));
	       PlacePointer(current_menu, current_item); 
	       next_state = Initial;
	       ProcessExposeEvents();
	  }
	  else
	       next_state = CheckTrigger;
	  UnlockCursor();
     }
     else if (MenuHasInitialItem(current_menu))
     {
	  /* Entered menu has initial item -- move to it */
	  DBUG_2("RTLmenu","Pushing for initial item.");
	  current_item = GetInitialItem(current_menu);
	  open_x = ItemGetArrowPosition(current_item) +
	       EventXRootX(rep);	    
	  ClearInitialItem(current_menu);
	  LockCursor(ItemWindow(current_item));
	  if (PushSubmenu(open_x))
	  {
	       ProcessExposeEvents();
	       LockCursor(ItemWindow(current_item));
	       PlacePointer(current_menu, current_item); 
	       next_state = Initial;
	  }
	  UnlockCursor();
     }
     else /* parent pull */
	  next_state = CheckTrigger;
     Leave(next_state)
}

State InitialState( rep)
XEvent rep;
{
     State next_state;
     
     Entry("Initial")
     switch (EventType(rep))
     {
     case EnterNotify:
	  if (MenuIsNull(current_menu))
	  {
	       Retch("(RTLmenu) null current menu!?!?");
	       next_state = Exit;
	  }
	  else if (EventXDetail(rep) == NotifyInferior)
	       next_state = Initial;
	  else
	  {
	       current_item = MenuGetItem(current_menu, EventXWindow(rep));
	       if (ItemIsNull(current_item))
	       {
		    /* Retch("(RTLmenu) Window entered not an item!\n"); */
		    next_state = Initial;
	       }
	       else
	       {
		    Highlight(current_item);
		    next_state = GetItemState(rep);
	       }
	  }
	  break;
     case LeaveNotify:
	  /* Decide whether we're actually leaving      */
	  /* this menu for another submenu or the root, */
	  /* or going into an item.                     */
	  next_state = (EventXDetail(rep) == NotifyInferior)?
	       Initial : LevelControl;
	  break;
     case ButtonRelease:
	  next_state = Exit;
	  break;
     case MotionNotify:
	  next_state = Initial;
	  break;
     default:
	  Retch("(RTLmenu) YOW! Unexpected event! (%d)\n", rep.type);
	  next_state = Exit;
	  break;
     }
     Leave(next_state)
}

#define NotSet -1
/* Look to see if pull-right is requested */
State CheckTriggerState(rep)
XEvent rep;
{
     State next_state = CheckTrigger;
     static int Trigger = NotSet;
     static int OldX, NewX, childX, button;
     
     Entry("CheckTrigger")
     if (MenuIsNull(current_menu) || ItemIsNull(current_item))
     {
	  Retch("(RTLmenu) Null menu or item...");
	  next_state = Exit;
	  goto exit;
     }
     if (Trigger == NotSet) /* set it */
     {
	  Trigger = MIN(EventGetXCoord(rep) + MenuDelta(current_menu),
			ItemGetArrowPosition(current_item));
	  NewX = NotSet;
     }
     switch (EventType(rep))
     {
     case LeaveNotify:
	  next_state = Initial;
	  Unhighlight(MenuGetItem(current_menu, EventXWindow(rep)));
	  Trigger = NotSet;
	  break;
     case ButtonRelease:
	  next_state = Exit;
	  Trigger = NotSet;
	  break;
	  
     case ButtonPress:
	  button = rep.xbutton.button;
	  while (TRUE) {
	       XNextEvent(dpy, &rep);
	       if (rep.type == ButtonRelease &&
		   rep.xbutton.button == button)
		    break;
	  }
	  next_state = CheckTrigger;
	  childX = TestOptionFlag(current_menu, fixedchild) ?
	       (MenuX(current_menu) + ItemGetArrowPosition(current_item)) :
		    EventXRootX(rep);
	  Trigger = NotSet;
	  if (PushSubmenu(childX))
	  {
	       next_state = LevelControl;
	       ProcessExposeEvents();
	       LockCursor(ItemWindow(current_item));
	       PlacePointer(current_menu, current_item);
	  }
	  UnlockCursor();
	  break;

     case MotionNotify:
	  next_state = CheckTrigger;
	  OldX = NewX;
	  NewX = EventGetXCoord(rep);
	  if (NewX >= Trigger)
	  {
	       LockCursor(ItemWindow(current_item));
	       childX = TestOptionFlag(current_menu, fixedchild)?
		    (MenuX(current_menu) + ItemGetArrowPosition(current_item)):
			 EventXRootX(rep);
	       Trigger = NotSet;
	       if (PushSubmenu(childX))
	       {
		    next_state = LevelControl;
		    ProcessExposeEvents();
		    LockCursor(ItemWindow(current_item));
		    PlacePointer(current_menu, current_item); 
	       }
	       UnlockCursor();
	  }
	  else if (NewX < OldX) /* reverse motion */
	       Trigger = MIN(Trigger, NewX + MenuDelta(current_menu));
	  break;

     default:
	  Retch("(RTLmenu) YOW! Unexpected event!\n");
	  next_state = Exit;
	  break;
     }
 exit:
     Leave(next_state)
}

State LeafState(rep,selected)
XEvent rep;
Boolean *selected;
{
     State next_state;
     
     Entry("LeafState")
     switch(EventType(rep))
     {
     case LeaveNotify:
	  Unhighlight(MenuGetItem(current_menu, EventXWindow(rep)));
	  next_state = Initial;
	  break;
	  
     case ButtonRelease:
	  *selected = TRUE;
	  next_state = Exit;
	  break;
     
     case ButtonPress:
     case EnterNotify:
     case MotionNotify: /* if events set right, this never happens */
	  next_state = Leaf;
	  break;
	  
     default:
	  Retch("(RTLMenu) YOW! Unexpected event! (%d)\n",
		rep.type);
	  next_state = Exit;
	  break;
     }
     Leave(next_state)
}

Boolean PushSubmenu(x)
int x;
{
     int y;
     Boolean pushed;
     MenuItem *new_current_item;
     
     Entry("PushSubmenu")

     if (ItemIsNull(current_item))
     {
	  Retch("(RTLMenu) Can't push from null item.\n");
	  pushed = FALSE;
     }
     else if (MenuIsNull(ItemSubmenu(current_item)))
     {
	  Retch("(RTLmenu) Null submenu.\n");
	  pushed = FALSE;
     }	
     else if (ItemIsNull(MenuItems(ItemSubmenu(current_item))))
	  /* submenu has no items -- don't push, but not an error */
	  pushed = FALSE;
     else
     {
	  y =  ItemGetMiddleY(current_item);
	  ++level;
	  
	  if (new_current_item =
	      Display_Menu(ItemSubmenu(current_item), current_menu, x, y))
	  {
	       XFlush(dpy);
	       current_menu = ItemSubmenu(current_item);
	       current_item = new_current_item;
	       if (ItemIsDeaf(current_item) && Autoselect)
		    current_item = current_item->nextItem;
	       pushed = TRUE;
	  }
	  else
	  {
	       Retch("(RTLmenu) Display_Menu failed!\n");
	       pushed = FALSE;
	  }
     }
     Leave(pushed)
}

void PopSubmenu()
{
     Menu *parent;
     MenuItem *item;
     
     Entry("PopSubmenu")
     --level;
     parent = current_menu->parentMenu;
     Undisplay_Menu(current_menu);
     current_menu = parent;
     if (!MenuIsNull(current_menu))
     {
	  item = MenuItemHighlighted(current_menu);
	  if (!ItemIsNull(item))
	  {
	       current_item = item;
	  }
     }
     
     Leave_void
}

void Highlight(item)
MenuItem *item;
{
     MenuItem *old_highlight;
     
     Entry("Highlight")
     
     old_highlight = MenuItemHighlighted(current_menu);
     if ((item != old_highlight) && /* else, already highlighted */
	 (!ItemIsNull(item)))
     {
	  if (!ItemIsNull(old_highlight) && !ItemIsDeaf(item))
	       Unhighlight(old_highlight); 
	  SetHighlightItem(ItemMenu(item), item);
	  Draw_Item(ItemMenu(item), item);
     }
     Leave_void
}

void Unhighlight(item)
MenuItem *item;
{
     Entry("Unhighlight")
     if (!ItemIsNull(item))
     {
	  if (MenuItemHighlighted(current_menu) == item)
	  {
	       ResetHighlightItem(ItemMenu(item));
	       Draw_Item(ItemMenu(item), item);
	  }
     }
     Leave_void
}

void TossExtraMoves(window)
Window window;
{
     XEvent ev;
     
     Entry("TossExtraMoves")
     while (XCheckTypedWindowEvent(dpy, window, MotionNotify, &ev))
	  DBUG_2("RTLmenu","Tossing extra motion.\n");
     Leave_void
}


void ProcessExposeEvents()
{
     MenuItem *item;
     XEvent ev;
     
     Entry("ProcessExposeEvents")

     XSync(dpy,0);
     while (XCheckTypedEvent(dpy, Expose, &ev))
     {
	  item = MenuGetItem(current_menu, EventXWindow(ev));
	  if (!ItemIsNull(item))
	       Draw_Item(ItemMenu(item), item);
     }
     Leave_void
}
