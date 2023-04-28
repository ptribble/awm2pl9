


#ifndef lint
static char *rcsid_NewIconify_c = "$Header: /usr/graph2/X11.3/contrib/windowmgrs/awm/RCS/NewIconify.c,v 1.2 89/02/07 21:23:10 jkh Exp $";
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
 * 001 -- Loretta Guarino Reid, DEC Ultrix Engineering Group,
 *  Western Software Lab. Port to X11.
 */

#include "awm.h"

/*ARGSUSED*/
Boolean NewIconify(window, mask, button, x, y)
Window window;                          /* Event window. */
int mask;                               /* Button/key mask. */
int button;                             /* Button event detail. */
int x, y;                               /* Event mouse position. */
{
     XWindowAttributes fromInfo;	/* from window info. */
     XWindowAttributes toInfo;		/* to window info. */
     Window from, to;
     int mse_x, mse_y;			/* Mouse X and Y coordinates. */
     int new_x, new_y;			/* To U. L. X and Y coordinates. */
     int new_w, new_h;			/* To width and height. */
     int bdw;				/* border width */
     int prev_x;			/* Previous event window X location. */
     int prev_y;			/* Previous event window Y location. */
     int cur_x;				/* Current event window X location. */
     int cur_y;				/* Current event window Y location. */
     int root_x;			/* Root window X location. */
     int root_y;			/* Root window Y location. */
     int ulx, uly;			/* Event window upper left X and Y. */
     int lrx, lry;			/* Event window lower right X and Y. */
     int num_vectors;			/* Number of vectors in box. */
     int status;				/* Routine call return status. */
     int ptrmask;			/* pointer query state. */
     Window root;			/* Mouse root window. */
     Window sub_win;			/* Mouse position sub-window. */
     XEvent button_event;		/* Button event packet. */
     XSegment box[MAX_BOX_VECTORS];	/* Box vertex buffer. */
     XSegment zap[MAX_ZAP_VECTORS];	/* Zap effect vertex buffer. */
     int (*storegrid_func)();		/* Function to use for grid */
     int (*storebox_func)();		/* Function to use for box */
     AwmInfoPtr awi;
     extern Window MakeIcon();
     extern void ungrab_pointer();
     extern void grab_pointer();
     
     Entry("NewIconify")
	  
     /*
      * Do not lower or iconify the root window.
      */
     if (window == RootWindow(dpy, scr))
	  Leave(FALSE)
     
     /*
      * Change the cursor to the icon cursor.
      */
     grab_pointer();
     
     /*
      * Clear the vector buffers.
      */
     bzero(box, sizeof(box));
     if (Zap)
	  bzero(zap, sizeof(zap));
     
     /*
      * Get info on the event window.
      */
     awi = GetAwmInfo(window);
     if (!awi)
	  Leave(FALSE)
     /*
      * Are we iconifying or de-iconifying?
      */
     if (awi->state & ST_ICON) {
          from = awi->icon;
          to = (awi->frame) ? awi->frame : awi->client;
     }
     else if (awi->state & ST_WINDOW) {
          from = (awi->frame) ? awi->frame : awi->client;
          to = (awi->icon) ? awi->icon : MakeIcon(window, x, y, FALSE);
     }
     else {
          fprintf(stderr, "NewIconify: Window %x has unknown state '%x'\n",
		 awi->client, awi->state);
	  Leave(FALSE)
     }
     status = XGetWindowAttributes(dpy, from, &fromInfo);
     if (status == FAILURE)
          Leave(FALSE)

     status = XGetWindowAttributes(dpy, to, &toInfo);
     if (status == FAILURE)
          Leave(FALSE)

     /*
      * Initialize the movement variables.
      */
     ulx = x - (toInfo.width >> 1) - toInfo.border_width;
     uly = y - (toInfo.height >> 1) - toInfo.border_width;
     lrx = x + (toInfo.width >> 1) + toInfo.border_width - 1;
     lry = y + (toInfo.height >> 1) + toInfo.border_width - 1;
     prev_x = x;
     prev_y = y;

     if (awi->frame && awi->state & ST_ICON) {
	  storegrid_func = StoreTitleGridBox;
	  storebox_func = StoreTitleBox;
     }
     else {
	  storegrid_func = StoreGridBox;
	  storebox_func = StoreBox;
     }
     
     /*
      * Store the box.
      */
     if (Grid)
	  num_vectors = storegrid_func(box, ulx, uly, lrx, lry);
     else
	  num_vectors = storebox_func(box, ulx, uly, lrx, lry);
     
     /*
      * Freeze the server, if requested by the user.
      * This results in a solid box instead of a flickering one.
      */
     if (Freeze)
	  XGrabServer(dpy);
     
     /*
      * Process any outstanding events before drawing the box.
      */
     while (QLength(dpy) > 0) {
	  XPeekEvent(dpy, &button_event);
	  if (((XAnyEvent *)&button_event)->window == RootWindow(dpy, scr))
	       break;
	  GetButton(&button_event);
     }
     
     /*
      * Draw the box.
      */
     DrawBox();
     if (Freeze)
	  Frozen = window;
     
     /*
      * We spin our wheels here looking for mouse movement or a change
      * in the status of the buttons.
      */
     Snatched = TRUE;
     while (TRUE) {
	  
	  /*
	   * Check to see if we have a change in mouse button status.
	   * This is how we get out of this "while" loop.
	   */
	  if (XPending(dpy) && !ProcessRequests(box, num_vectors) &&
	      GetButton(&button_event)) {
	       /*
		* Process the pending events, this sequence is the only
		* way out of the loop and the routine.
		*/
	       
	       if ((button_event.type != ButtonPress) && 
		   (button_event.type != ButtonRelease)) {
		    continue; /* spurious menu event... */
	       }
	       
	       /*
		* If we froze the server, then erase the last lines drawn.
		*/
	       if (Freeze) {
		    DrawBox();
		    Frozen = (Window)0;
		    XUngrabServer(dpy);
	       }
	       
	       /*
		* Save the mouse cursor location.
		*/
	       if (button_event.type == ButtonPress ||
		   button_event.type == ButtonRelease) {
		    mse_x = ((XButtonEvent *)&button_event)->x;
		    mse_y = ((XButtonEvent *)&button_event)->y;
		    break;
	       }
	  }
	  else {
	       /*
		* Continue to track the mouse until we get a change
		* in button status.
		*/
	       XQueryPointer(dpy, RootWindow(dpy, scr), 
			     &root, &sub_win, &root_x, &root_y, &cur_x,
			     &cur_y, &ptrmask);
	       
	       /*
		* If the mouse has moved, then make sure the box follows it.
		*/
	       if ((cur_x != prev_x) || (cur_y != prev_y)) {
		    
		    /*
		     * If we've frozen the server, then erase the old box first!
		     */
		    if (Freeze)
			 DrawBox();
		    
		    /*
		     * Set the new box position.
		     */
		    ulx += cur_x - prev_x;
		    uly += cur_y - prev_y;
		    lrx += cur_x - prev_x;
		    lry += cur_y - prev_y;
		    
		    /*
		     * Calculate the vectors for the new box.
		     */
		    
		    if (Grid)
			 num_vectors = storegrid_func(box, ulx, uly, lrx,
							 lry);
		    else
			 num_vectors = storebox_func(box, ulx, uly, lrx, lry);
		    
		    /*
		     * Draw the new box.
		     */
		    if (Freeze)
			 DrawBox();
	       }
	       
	       /*
		* Save the old box position.
		*/
	       prev_x = cur_x;
	       prev_y = cur_y;
	       
	       /*
		* If server is not frozen, then draw the "flicker" box.
		*/
	       if (!Freeze) {
		    DrawBox();
		    DrawBox();
	       }
	  }
     }
     Snatched = FALSE;
     /*
      * If the button is not a button release of the same button pressed,
      * then abort the operation.
      */
     if ((button_event.type != ButtonRelease) ||
	 (((XButtonReleasedEvent *)&button_event)->button != button)) {
	  ResetCursor(button);
	  ungrab_pointer();
	  Leave(TRUE)
     }
     
     /*
      * If we are here we have committed to iconifying/deiconifying.
      */
     
     /*
      * Determine the coordinates of the icon or window;
      * normalize the window or icon coordinates if the user so desires.
      */
     new_w = toInfo.width;
     new_h = toInfo.height;
     bdw = toInfo.border_width;
     new_x = mse_x - (new_w >> 1) - bdw;
     new_y = mse_y - (new_h >> 1) - bdw;
     if ((NIcon && awi->state & ST_WINDOW) || (NWindow && awi->state & ST_ICON)) {
	  if (new_x < 0) new_x = 0;
	  if (new_y < 0) new_y = 0;
	  if ((new_x - 1 + new_w + (bdw << 1)) > ScreenWidth) {
	       new_x = ScreenWidth - new_w - (bdw << 1) + 1;
	  }
	  if ((new_y - 1 + new_h + (bdw << 1)) > ScreenHeight) {
	       new_y = ScreenHeight - new_h - (bdw << 1) + 1;
	  }
     }
     if (awi->state & ST_ICON) {
	  awi->state ^= ST_ICON;
	  awi->state |= (ST_WINDOW | ST_PLACED);
	  if (!awi->frame)
	       XRemoveFromSaveSet(dpy, awi->client);
     }
     else if (awi->state & ST_WINDOW) {
	  awi->state ^= ST_WINDOW;
	  awi->state |= ST_ICON;
	  XAddToSaveSet(dpy, awi->client);
     }
     else
	  fprintf(stderr, "NewIconify: Window state got trashed!\n");
     
     /*
      * Move the window into place.
      */
     XMoveWindow(dpy, to, new_x, new_y);
 
     /*
      * Map the icon window.
      */
     XMapRaised(dpy, to);
     if (Zap) {
	  num_vectors = StoreZap(zap,
				 fromInfo.x,
				 fromInfo.y,
				 fromInfo.x + fromInfo.width
				 + (fromInfo.border_width << 1),
				 fromInfo.y + fromInfo.height
				 + (fromInfo.border_width << 1),
				 ulx, uly, lrx, lry);
	  DrawZap();
	  DrawZap();
     }
     /*
      * Unmap the event window.
      */
     XUnmapWindow(dpy, from);
     XFlush(dpy);
     ungrab_pointer();
     Leave(TRUE)
}
