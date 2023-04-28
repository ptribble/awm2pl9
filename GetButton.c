


#ifndef lint
static char *rcsid_GetButton_c = "$Header: /usr/graph2/X11.3/contrib/windowmgrs/awm/RCS/GetButton.c,v 1.3 89/02/07 22:39:19 jkh Exp $";
#endif	lint

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
 * 001 -- L. Guarino Reid, DEC Ultrix Engineering Group, Western Software Lab
 *	  February 16, 1987
 *	  Add EnterWindow, LeaveWindow, and MouseMotion as recognized
 *	  awm buttons for awm menus. Add bug fixes to prevent mem faults
 *	  if icon_str is NULL.
 * 002 -- L. Guarino Reid, DEC Ultrix Engineering Group
 *	  April 16, 1987
 *	  Convert to X11
 * 003 -- J. Hubbard, U.C. Berkeley. Title/gadget box event handling.
 *    December 3, 1987.
 * 004 -- J.Hubbard, Ardent Computer. More conformance with ICCCM. Merge of
 * changes from R2 uwm.
 */

#ifndef lint
static char *sccsid = "@(#)GetButton.c	3.8	1/24/86";
#endif
/*
 *	GetButton - This subroutine is used by the Ardent Window Manager (awm)
 *	to acquire button events.  It waits for a button event to occur
 *	and handles all event traffic in the interim.
 *
 *	File:		GetButton.c
 */

#include "awm.h"
#include "X11/Xutil.h"

extern XContext AwmContext;
extern Window FocusWindow;

#define ICONSTR	(icon_str ? icon_str : "")
/* Amount of padding between text in a title bar and the edge of the bar */
#define PAD	1

static Icon_modified = FALSE;

Boolean GetButton(b_ev)
XEvent *b_ev;	/* Button event packet. */
{
#define STRLEN 50
     char *icon_str;		/* Icon's name string. */
     register int icon_str_len;	/* Icon name string lenght.  */
     register int key_char;	/* Key press character code. */
     register int icon_x;	/* Icon window X coordinate. */
     register int icon_y;	/* Icon window Y coordinate. */
     register unsigned int icon_w;	/* Icon window width. */
     register unsigned int icon_h;	/* Icon window height. */    
     int status;			/* Routine call return status. */
     Window icon;		/* Icon window. */
     XWindowAttributes win_info;	/* Icon window info structure. */
     char kbd_str[STRLEN];      /* Keyboard string. */
     int nbytes;                /* Keyboard string length. */
     int i;                     /* Iteration counter. */
     Window win;		/* scratch */
     AwmInfoPtr awi;
     extern char *GetTitleName();
     
     Entry("GetButton")
  
     /*
      * Get next event from input queue and store it in the event packet
      * passed to GetButton.
      */
  
     XNextEvent(dpy, b_ev);
     /* print_event_info("mainloop", b_ev); /* debugging for event handling */

     /*
      * The event occured on the root window, check for substructure
      * changes. Otherwise, it must be a mouse button event. 
      */

     if (b_ev->xany.window == RootWindow(dpy, scr)) {
	  switch (b_ev->xany.type) {
	  case CreateNotify:
	  case UnmapNotify:
	  case ReparentNotify:
	  case ConfigureNotify:
	  case GravityNotify:
	  case MapNotify:
	  case CirculateNotify:
	       Leave(FALSE)
		    
	  case MappingNotify:
	       XRefreshKeyboardMapping((XMappingEvent *) b_ev);
	       Leave(FALSE)

	  case MapRequest: 
	       CheckMap(b_ev->xmap.window);
	       Leave(FALSE)

	  case ConfigureRequest: 
	       Configure((XConfigureRequestEvent *)b_ev);
	       Leave(FALSE)
	       
	  case CirculateRequest: 
	       Circulate((XCirculateEvent *)b_ev);
	       Leave(FALSE)
	       
	  case DestroyNotify:
	       RemoveIcon(b_ev->xdestroywindow.window);
	       Leave(FALSE)

	  case FocusIn:
	       if (b_ev->xfocus.detail == NotifyPointerRoot) {
		    if (FocusSetByUser) {
			 XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot,
					CurrentTime);
			 FocusSetByUser = FALSE;
			 FocusWindow = RootWindow(dpy, scr);
		    }
	       }
	       Leave(FALSE)
	       
	  case FocusOut:
	       if (b_ev->xfocus.detail == NotifyPointerRoot) {
		    if (FocusSetByUser == FALSE && !FocusSetByWM) {
			 XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot,
					CurrentTime);
			 FocusWindow = RootWindow(dpy, scr);
		    }
	       }
	       Leave(FALSE)
	       
	  case ButtonPress:
	  case ButtonRelease:
	       Leave(TRUE)

	  default: 
	       Leave(FALSE) 
	  }
     }
     
     /*
      * If the event type is EnterWindow, LeaveWindow, or MouseMoved,
      * we are processing a menu. 
      * If the event type is ButtonPress or ButtonRelease,
      * we have a button event.
      * If it's an expose, then we may have exposed a title bar.
      * If it's a Notify, we've probably frobbed a titled window.
      */
     
     switch (b_ev->type) {
	  XEvent e_fake;

     case MotionNotify: 
     case ButtonPress: 
     case ButtonRelease: 
	  Leave(TRUE)
	  break;

     case FocusIn:
	  if (!FocusSetByWM) {
	       e_fake.xcrossing.type = FocusIn;
	       e_fake.xcrossing.focus = TRUE;
	       e_fake.xcrossing.window = b_ev->xcrossing.window;
	       HandleFocusIn(&e_fake);
	  }
	  Leave(FALSE)
	  break;

     case FocusOut:
	  if (!FocusSetByWM) {
	       e_fake.xcrossing.type = FocusOut;
	       e_fake.xcrossing.focus = TRUE;
	       e_fake.xcrossing.window = b_ev->xcrossing.window;
	       HandleFocusOut(&e_fake);
	  }
	  Leave(FALSE)
	  break;
	  
     case EnterNotify:
	  HandleFocusIn(b_ev);
	  Leave(FALSE)

     case LeaveNotify: 
	  HandleFocusOut(b_ev);
	  Leave(FALSE)

     case ConfigureRequest:
	  Configure((XConfigureRequestEvent *)b_ev);
	  Leave(FALSE)

     case MapRequest:
	  CheckMap(b_ev->xmap.window);
	  Leave(FALSE);

     case MapNotify:
          Leave(FALSE)
	  break;
	  
     case UnmapNotify:
	  win = b_ev->xunmap.window;
          if (!(awi = GetAwmInfo(win)))
              Leave(FALSE)

	  if (IsIcon(win, None))
	       Leave(FALSE)
	   XUnmapWindow(dpy, awi->frame);
	  Leave(FALSE)
	  break;

     case DestroyNotify:
	  win = b_ev->xdestroywindow.window;
          if (!(awi = GetAwmInfo(win)))
              Leave(FALSE)
	  if (win != awi->title) {
	       /* remove any icon associated with this window */
	       RemoveIcon(win);
	       /* remove the frame/titlebar (if present) */
	       NoDecorate(win, TRUE);
	       /* punt the rest of the per-window info */
	       XDeleteContext(dpy, awi->client, AwmContext);
	       free(awi);
	  }
          Leave(FALSE)

     case PropertyNotify:
	  win = b_ev->xproperty.window;
          if (!(awi = GetAwmInfo(win)))
               Leave(FALSE);
#ifdef WMSTATE
	  if (!(b_ev->xproperty.atom==wm_state_atom)) /* ignore state change */
#endif /* WMSTATE */
	  switch(b_ev->xproperty.atom) {

	  case XA_WM_COMMAND:
	  case XA_WM_HINTS:
	  case XA_WM_CLIENT_MACHINE:
	       break;

	  case XA_WM_ICON_NAME:
	       if (Icon_modified == TRUE) {
		    /*
		     * Icon was modified by type-in (I still think that's a
		     * gross feature, but some people like it... sigh),
		     * ignore this event.
		     */
		    Icon_modified = FALSE;	/* reset */
		    Leave(FALSE)
	       }
	       /*
		* Icon was modifed in a more civilized fashion.
		*/
	       if (awi->icon && awi->own && awi->iconPixmap == IBackPixmap) {
		    win = awi->icon;
		    status = XGetWindowAttributes(dpy, win, &win_info);
		    icon_str = GetIconName(awi->client);
		    icon_str_len = icon_str ? strlen(icon_str) : 0;
		    if (win_info.width != XTextWidth(IFontInfo, ICONSTR,
						     strlen(ICONSTR)) +
			(HIconPad << 1)) {
			 XResizeWindow(dpy, win, 
				       XTextWidth(IFontInfo, ICONSTR,
						  strlen(ICONSTR)) +
				       (HIconPad << 1), IFontInfo->ascent +
				       IFontInfo->descent + (VIconPad << 1));
		    }
		    XClearWindow(dpy, win);
		    if (icon_str_len != 0) {
			 XDrawImageString(dpy, win, IconGC, HIconPad,
					  VIconPad+IFontInfo->ascent, icon_str,
					  icon_str_len);
			 free(icon_str);
		    }
	       }
	       break;

	  case XA_WM_ICON_SIZE:
	       break;

	  case XA_WM_NAME:
	       if (awi->title) {
		    if (awi->name)
			 free(awi->name);
		    awi->name = GetTitleName(win);
		    PaintTitle(win, (FocusWindow == awi->client));
	       }
	       break;

	  case XA_WM_NORMAL_HINTS:
	  case XA_WM_SIZE_HINTS:
	  case XA_WM_ZOOM_HINTS:
	       break;

	  default:
	       fprintf(stderr, "awm: Got unknown property %d\n",
		       b_ev->xproperty.atom);
	  }
	  Leave(FALSE)
	  break;
	  
	  
     case Expose:
	  win = b_ev->xexpose.window;
          if (!(awi = GetAwmInfo(win))) /* probably a menu */
               Leave(FALSE)
	  if (awi->title == win) {
	       if (b_ev->xexpose.count == 0) {
		    XEvent evt;

		    /* Eat up any previous exposes */
		    while (XCheckTypedWindowEvent(dpy, awi->title, Expose,
						  &evt));
		    PaintTitle(win, (FocusWindow == awi->client));
		    if (awi->gadgets)
			 PaintGadgets(win);
	       }
	       Leave(FALSE)
	  }
	  break;
	  
     default:
	  break;
     }
     
     /*
      * If we got this far, the event must be for an icon.
      */
     win = b_ev->xany.window;
     if (!(awi = GetAwmInfo(win)))
          Leave(FALSE)
     status = XGetWindowAttributes(dpy, win, &win_info);
     if (status == FAILURE)
	  Leave(FALSE)
     
     if (b_ev->type == MapNotify || 
	 b_ev->type == UnmapNotify ||
	 b_ev->type == CreateNotify ||
	 b_ev->type == ReparentNotify ||
	 b_ev->type == GravityNotify ||
	 b_ev->type == CirculateNotify ||
	 b_ev->type == ConfigureNotify)
	  Leave(FALSE)
     
     /*
      * Initialize the icon position variables.
      */
     icon_x = win_info.x;
     icon_y = win_info.y;
     
     /*
      * Get the name of the window associated with the icon and
      * determine its length.
      */
     if (!awi->icon)
	  Leave(FALSE)
     /*
      * If we don't own it, or it's got a background pixmap, we don't have
      * to repaint it.
      */
     if (!awi->own || (awi->iconPixmap != IBackPixmap))
	  Leave(FALSE)
     icon = awi->icon;
     icon_str = GetIconName(awi->client);
     icon_str_len = icon_str ? strlen(icon_str) : 0;
     
     /*
      * If the event is a window exposure event and the icon's name string
      * is not of zero length, simply repaint the text in the icon window
      * and return FALSE.
      */
     if (b_ev->type == Expose && (!Freeze || Frozen == 0)) {
	  if (win_info.width != XTextWidth(IFontInfo, ICONSTR,
					   strlen(ICONSTR))+(HIconPad << 1)) {
	       XResizeWindow(dpy, icon, 
			     XTextWidth(IFontInfo, ICONSTR,
					strlen(ICONSTR)) + (HIconPad << 1),
			     IFontInfo->ascent + IFontInfo->descent + 
			     (VIconPad << 1));
	  }
	  XClearWindow(dpy, icon);
	  if (icon_str_len != 0) {
	       XDrawImageString(dpy, icon,
				IconGC, HIconPad, VIconPad+IFontInfo->ascent,
				icon_str, icon_str_len);
	       /*
		* Remember to free the icon name string.
		*/
	       free(icon_str);
	  }
	  Leave(FALSE)
     }

#ifndef NO_ICON_TYPIN     
     /*
      * If we have gotten this far event can only be a key pressed event.
      */
     
     /* 
      * We convert the key pressed event to ascii.
      */
     nbytes = XLookupString((XKeyEvent *)b_ev, kbd_str, STRLEN,
			    (KeySym *) NULL, (XComposeStatus *) NULL);
     
     /*
      * If kbd_str is a "non-string", then don't do anything.
      */
     if (nbytes == 0 || !kbd_str || !*kbd_str) {
	  if (icon_str)
	       free(icon_str);
	  Leave(FALSE)
     }
     for (i = 0; i < nbytes; i++) {
	  key_char = kbd_str[i];
	  /*
	   * If the key was <DELETE>, then delete a character from the end of
	   * the name, return FALSE.
	   *
	   * If the key was <CTRL-U>, then wipe out the entire window name
	   * and return FALSE.
	   *
	   * All other ctrl keys are squashed and we return FALSE.
	   *
	   * All printable characters are appended to the window's name, which
	   * may have to be grown to allow for the extra length.
	   */
	  if (key_char == '\177') {
	       /*
		* <DELETE>
		*/
	       if (icon_str_len > 0) {
		    icon_str_len--;
		    icon_str[icon_str_len] = '\0';
	       }
	  }
	  else if (key_char == '\025') {
	       /*
		* <CTRL-U>
		*/
	       if (icon_str_len > 0) {
		    icon_str_len = 0;
		    icon_str[0] = '\0';
	       }
	  }
	  else if (key_char < IFontInfo->min_char_or_byte2 ||
		   key_char > IFontInfo->max_char_or_byte2) {
	       /*
		* Any other random (non-printable) key; ignore it.
		*/
	       /* do nothing */ ;
			   }
	  else {
	       /*
		* ASCII Alphanumerics.
		*/
	       if (icon_str == NULL)
		    icon_str = (char *) malloc (icon_str_len + 2);
	       else
		    icon_str = (char *)realloc(icon_str, (icon_str_len + 2));
	       if (icon_str == NULL) {
		    errno = ENOMEM;
		    Error("GetButton -> Realloc of window name string memory failed.");
	       }
	       icon_str[icon_str_len] = key_char;
	       icon_str[icon_str_len + 1] = '\0';
	       icon_str_len += 1;
	  }
     }
     
     /*
      * Now that we have changed the size of the icon we have to reconfigure
      * it so that everything looks good.  Oh yes, don't forget to move the
      * mouse so that it stays in the window!
      */
     
     /*
      * Set the window name to the new string. Icon_modified is a kludge
      * that tells us to avoid the next PropertyNotify, as it's a result of
      * this call.
      */
     XSetIconName(dpy, awi->client, ICONSTR);
     Icon_modified = TRUE;
     /*
      * Determine the new icon window configuration.
      */
     icon_h = IFontInfo->ascent + IFontInfo->descent + (VIconPad << 1);
     icon_w = XTextWidth(IFontInfo, ICONSTR, strlen(ICONSTR));
     if (icon_w == 0) {
	  icon_w = icon_h;
     }
     else {
	  icon_w += (HIconPad << 1);
     }
     
     if (icon_x < 0) icon_x = 0;
     if (icon_y < 0) icon_y = 0;
     if (icon_x - 1 + icon_w + (IBorderWidth << 1) > ScreenWidth) {
	  icon_x = ScreenWidth - icon_w - (IBorderWidth << 1) + 1;
     }
     if (icon_y - 1 + icon_h + (IBorderWidth << 1) > ScreenHeight) {
	  icon_y = ScreenHeight - icon_h - (IBorderWidth << 1) + 1;
     }
     
     XMoveResizeWindow(dpy, icon, icon_x, icon_y, icon_w, icon_h);
     XWarpPointer(dpy, None, icon, 
		  0, 0, 0, 0, (icon_w >> 1), (icon_h >> 1));
     
     /* 
      * Free the local storage and return FALSE.
      */
     if (icon_str)
	  free(icon_str);
#endif
     Leave(FALSE)
}

CheckMap(window)
Window window;
{
     XWMHints *XGetWMHints();
     XWMHints *wmhints;
     Window transient_for = None;
     Binding *bptr;
     AwmInfoPtr awi;
     long event_mask;

     Entry("CheckMap")

     /* if it's a transient window, we won't rubber-band
      * note that this call always sets transient_for.
      */
     if (!XGetTransientForHint( dpy, window, &transient_for )) {
	  /*
	   * Handle any registered grabs for this window. We do this here
	   * because we may be about to make an icon out of this window
	   * and we want to register any potential grabs on it before this
	   * happens.
	   */
	  awi = GetAwmInfo(window);
	  if (!awi) {
	       for (bptr = Blist; bptr; bptr = bptr->next)
		    if (bptr->context & WINDOW)
			 Grab(bptr->mask, window);
	       awi = RegisterWindow(window);
	       Decorate(awi->client);
	       event_mask = PropertyChangeMask | FocusChangeMask;
	       if (!FrameFocus)
		    event_mask |= (EnterWindowMask | LeaveWindowMask);
	       XSelectInput(dpy, window, event_mask);
#ifndef RAINBOW
	       SetBorderPixmaps(awi, GrayPixmap);
#else
	       SetBorderPixmaps(awi, awi->grayPixmap);
#endif
	  }
	  if ((wmhints = XGetWMHints(dpy, window))) {
	       if ((wmhints->flags & StateHint) && (awi->state & ST_WINDOW) &&
		   (wmhints->initial_state == IconicState)) {
		    /* window wants to be created as an icon. Leave size
		       /* and position alone, create as icon. */
                         XFree(wmhints);
                         Iconify(window, 0, 0, 0, 0);
                         Leave_void
	       }
	       XFree(wmhints);
	  }
     }
     else { /* It's a transient */
	  if (!(awi = GetAwmInfo(window)))
	       awi = RegisterWindow(window);
	  awi->attrs = AT_NONE;
	  if (transient_for == None)
	       transient_for = RootWindow(dpy, scr);
     }
     if (awi->state & ST_WINDOW) {
	  PlaceWindow(window, transient_for);
	  if (awi->frame) {
	       XMapRaised(dpy, awi->frame);
	       XMapWindow(dpy, awi->client);
	  }
	  else
	       XMapRaised(dpy, awi->client);
	  XSync(dpy, 0);
     }
     Leave_void
}

Configure(event)
XConfigureRequestEvent *event;
{
     XWindowChanges values;
     
     Entry("Configure")

     values.x = event->x;
     values.y = event->y;
     values.width = event->width;
     values.height = event->height;
     values.border_width = event->border_width;
     values.stack_mode = event->detail;
     values.sibling = event->above;

     ConfigureWindow(event->window, event->value_mask, &values);
     Leave_void
}

Circulate(event)
XCirculateEvent *event;
{
     Entry("Circulate")

     if (event->place == PlaceOnTop)
	  XRaiseWindow(event->display, event->window);
     else
	  XLowerWindow(event->display, event->window);
     Leave_void
}

PlaceWindow(window, transient)
Window window;
Window transient;
{
     XSizeHints wsh;
     int x, y;
     unsigned int w, h;
     XWindowChanges xwc;
     AwmInfoPtr awi;
     unsigned long flags;

     wsh.flags = 0;
     flags = CWX | CWY | CWWidth | CWHeight;
     awi = GetAwmInfo(window);
     XGetSizeHints(dpy, window, &wsh, XA_WM_NORMAL_HINTS);
     CheckConsistency(&wsh);
     AskUser(dpy, scr, window, &x, &y, &w, &h, &wsh, transient);

     wsh.flags |= (USPosition | USSize);
     wsh.x = x;
     wsh.y = y;
     wsh.width = w;
     wsh.height = h;
     if (x < 0 || y < 0) {
	  Window jW;
	  int j, border_width;

	  XGetGeometry(dpy, window, &jW, &j, &j, &j, &j, &border_width, &j);

	  if (x < 0)
	       x += DisplayWidth(dpy, scr) - w - (border_width<<1);
	  if (y < 0)
	       y += DisplayHeight(dpy, scr) - h - (border_width<<1);
     }
     if (awi->frame)
	  XSetSizeHints(dpy, awi->frame, &wsh, XA_WM_NORMAL_HINTS);
     XSetSizeHints(dpy, awi->client, &wsh, XA_WM_NORMAL_HINTS);
#ifdef titan /* 5x4 alignment */
     x = ((x+3) / 5) * 5;
     y = ((y+2) / 4) * 4;
#endif
     xwc.x = x;
     xwc.y = y;
     xwc.height = h;
     xwc.width = w;
     ConfigureWindow(window, flags, &xwc);
     awi->state |= ST_PLACED;
}

Boolean ConfigureWindow(w, mask, xwc)
Window w;
unsigned int mask;
XWindowChanges *xwc;
{
     AwmInfoPtr awi;
     int height, width, x, y;
     int bch = 0, bcv = 0;
     Entry("ConfigureWindow")

     awi = GetAwmInfo(w);
     if (!awi) {
	  /*
	   * Make sure XtRealize gets its configure event. We'll actually
	   * do another configure before we map, but this fakes XtRealize
	   * into not waiting around for the configure notify.
	   */
	  XConfigureWindow(dpy, w, mask, xwc);
	  XFlush(dpy);
          Leave(FALSE)
     }

     if (awi->icon == w) {
	  XConfigureWindow(dpy, w, mask, xwc);
	  Leave(TRUE)
     }
     if (awi->attrs & AT_BORDER && DECORATED(awi)) {
	  bcv = BContext + 1;
	  bch = bcv * 2;
     }

     height = xwc->height;
     width = xwc->width;
     if (awi->frame)
	  x = y = 0;
     else {
	  x = xwc->x;
	  y = xwc->y;
     }
     /* width */
     xwc->width += bch;
     if (awi->title)
	  XResizeWindow(dpy, awi->title, (unsigned) xwc->width, 
			(unsigned) titleHeight);
     /* height */
     if (DECORATED(awi)) {
	  if (awi->attrs & AT_TITLE) {
	       y = titleHeight + 2;
	       xwc->height += y;
	  }
	  if (awi->attrs & AT_BORDER) {
	       if (!(awi->attrs & AT_TITLE)) {
		    xwc->height += (bcv * 2);
		    y = bcv;
	       }
	       else
		    xwc->height += bcv;
	       xwc->x -= (BContext + (awi->border_width ? awi->border_width :
				      1));
	       x = BContext;
	  }
     }
     if (PushDown == FALSE) {
	  if (awi->attrs & AT_TITLE && DECORATED(awi))
	       xwc->y -= (titleHeight + 2);
	  else if (awi->attrs & AT_BORDER && DECORATED(awi))
	       xwc->y -= (BContext + (awi->border_width ?
				      awi->border_width : 1));
     }
     if (awi->frame)
          XConfigureWindow(dpy, awi->frame, mask, xwc);
     if (awi->gadgets)
	  PutGadgets(w);
     xwc->width = width;
     xwc->height = height;
     xwc->x = x;
     xwc->y = y;
     XConfigureWindow(dpy, awi->client, mask, xwc);
     Leave(TRUE)
}

int
ProcessRequests(box, num_vectors)
XSegment *box;
int num_vectors;
{
	XEvent event;

	XPeekEvent(dpy, &event);
	switch (event.type) {

	case MapRequest:
	case ConfigureRequest:
	case CirculateRequest:

	    DrawBox();
	    GetButton(&event);
	    DrawBox();
	    return TRUE;
	}
	return FALSE;
}
