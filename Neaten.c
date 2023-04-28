


#ifndef lint
static char *rcsid_Neaten_c = "$Header: /usr/graph2/X11.3/contrib/windowmgrs/awm/RCS/Neaten.c,v 1.2 89/02/07 21:23:05 jkh Exp $";
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
 * MODIFICATION HISTORY
 *
 * 002 -- Jordan Hubbard, Ardent Computer.
 *  Changes to work with awm, specifically reparented windows.
 */

#include "awm.h"
     
#ifdef NEATEN
     
#include "X11/Xutil.h"
#include "X11/cursorfont.h"
#include "neaten.ext.h"

static void GetIconHints();
static void GetName();
extern Boolean IsIcon();
extern int neaten_debug_level;

#define TOP_LEVEL (awi->frame ? awi->frame : awi->client)

/*ARGSUSED*/
Boolean Neaten(window, mask, button, x, y)
Window window;                          /* Event window. */
int mask;                               /* Button/key mask. */
int button;   	                        /* Button event detail. */
int x, y;                               /* Event mouse position. */
{
     Window junk_window;
     Window w;
     Window icon_window;
     Window stacktop;
     Window *children;
     XWindowAttributes attrs;
     XSizeHints hints;
     XWindowChanges  wc;
     Status rstatus;
     char name[50];
     int junk;
     int root_x, root_y;
     int root_width, root_height;
     int icon_x, icon_y;
     int icon_width, icon_height;
     int icon_border;
     int window_x, window_y;
     int window_width, window_height;
     int rval;
     int ix;
     int iconic;
     int tiled;
     int adjustment;
     int priority;
     int priority_increment;
     unsigned int nchildren;
     unsigned int mapped_children = 0;
     unsigned int opened_children = 0;
     unsigned int  wcmask;
     Placement primary;
     Placement secondary;
     Boolean is_iconic;
     Cursor rtlcursor;
     XFontStruct *font;
     GC gc;
     AwmInfoPtr awi;

     Entry("Neaten")

     font = (XFontStruct *) XLoadQueryFont(dpy, "cursor");
     gc = DefaultGC(dpy, scr);
     XSetFont(dpy, gc, font->fid);
     
     rtlcursor = XCreateFontCursor(dpy, XC_rtl_logo);
     
     XGrabPointer(dpy, RootWindow(dpy, scr), FALSE, (unsigned int) 0,
		  GrabModeAsync, GrabModeAsync,
		  (Window) None, rtlcursor, CurrentTime);
     
     if (AbsMinWidth == 0)
	  AbsMinWidth = DEFAULT_ABS_MIN;
     if (AbsMinHeight == 0)
	  AbsMinHeight = DEFAULT_ABS_MIN;
     
     XGetGeometry(dpy, RootWindow(dpy, scr), &junk_window,
		  &root_x, &root_y,
		  &root_width, &root_height,
		  &junk, &junk);
     
     Neaten_Initialize(root_width+SEPARATION,
		       root_height+SEPARATION,
		       AbsMinWidth, AbsMinHeight);
     
     if (strcmp(PrimaryIconPlacement, "Closest") == 0)
	  primary = Place_Closest;
     else if (strcmp(PrimaryIconPlacement, "Top") == 0)
	  primary = Place_Top;
     else if (strcmp(PrimaryIconPlacement, "Bottom") == 0)
	  primary = Place_Bottom;
     else if (strcmp(PrimaryIconPlacement, "Left") == 0)
	  primary = Place_Left;
     else if (strcmp(PrimaryIconPlacement, "Right") == 0)
	  primary = Place_Right;
     else
	  primary = Place_Top;
     
     if (strcmp(SecondaryIconPlacement, "Closest") == 0)
	  secondary = Place_Closest;
     else if (strcmp(SecondaryIconPlacement, "Top") == 0)
	  secondary = Place_Top;
     else if (strcmp(SecondaryIconPlacement, "Bottom") == 0)
	  secondary = Place_Bottom;
     else if (strcmp(SecondaryIconPlacement, "Left") == 0)
	  secondary = Place_Left;
     else if (strcmp(SecondaryIconPlacement, "Right") == 0)
	  secondary = Place_Right;
     else if (strcmp(SecondaryIconPlacement, "Center") == 0)
	  secondary = Place_Center;
     else
	  secondary = Place_Left;
     
     Neaten_Icon_Placement(primary, secondary);
     
     if (XQueryTree(dpy, RootWindow(dpy, scr), &junk_window,
		    &junk_window, &children, &nchildren))
     {
	  for (ix = 0; ix < nchildren; ix++)
	  {
	       XGetWindowAttributes(dpy, children[ix], &attrs);	    
	       
	       if ((attrs.map_state != IsUnmapped) &&
		   (attrs.override_redirect == False) &&
		   (awi = GetAwmInfo(children[ix])))
	       {
		    w = children[ix];
		    if (w != TOP_LEVEL && w != awi->icon)
			 continue;
		    mapped_children++;
		    
		    is_iconic = IsIcon(w, (Window *) NULL);
		    
		    if (is_iconic == TRUE)
		    {
			 window_x = window_y =
			      window_width = window_height = 0;
			 icon_x = attrs.x;
			 icon_y = attrs.y;
			 icon_width = attrs.width;
			 icon_height = attrs.height;
			 
			 adjustment = SEPARATION;
			 icon_width += adjustment;
			 icon_height += adjustment;
		    }
		    else
		    {
			 window_x = attrs.x;
			 window_y = attrs.y;
			 window_width = attrs.width;
			 window_height = attrs.height;
			 
			 window_x -= attrs.border_width;
			 window_y -= attrs.border_width;
			 
			 adjustment = (2 * attrs.border_width) + SEPARATION;
			 window_width += adjustment;
			 window_height += adjustment;
			 
			 GetIconHints(w, &icon_x, &icon_y,
				      &icon_width, &icon_height, &icon_border);
			 
			 icon_x -= icon_border;
			 icon_y -= icon_border;
			 icon_width += (2 * icon_border) + SEPARATION;
			 icon_height += (2 * icon_border) + SEPARATION;
			 
			 opened_children++;
			 stacktop = TOP_LEVEL;
		    }
		    
		    GetName(w, name);
		    if(neaten_debug_level)
		    {
			 printf("%s\ticonic = %s\n",
				name, (is_iconic ? "TRUE" : "FALSE"));
			 printf("\ticon: x = %d y = %d width = %d height = %d\n",
				icon_x, icon_y, icon_width, icon_height);
			 if (is_iconic != TRUE)
			      printf("\twindow: x = %d y = %d width = %d height = %d\n",
				     window_x, window_y, window_width,
				     window_height);
		    }
		    
		    Neaten_Identify((int) w, is_iconic,
				    NEATEN_TRUE, NEATEN_FALSE,
				    window_x, window_y,
				    window_width, window_height,
				    NEATEN_TRUE, NEATEN_FALSE,
				    icon_x, icon_y,
				    icon_width, icon_height,
				    (unsigned long) adjustment);
		    
		    rstatus = XGetNormalHints(dpy, w, &hints);
		    if (rstatus != (Status) 0 && is_iconic != TRUE)
		    {
			 if (hints.flags & PMinSize)
			 {
			      rval = Neaten_Set_Min((int) w,
						    hints.min_width+adjustment,
						    hints.min_height+adjustment);
			      if(neaten_debug_level)
				   printf("\tmin: width = %d height = %d status = %s\n",
					  hints.min_width, hints.min_height,
					  (rval ? "SUCCESS" : "FAILURE"));
			 }
			 if (hints.flags & PMaxSize) 
			 {
			      rval = Neaten_Set_Max((int) w,
						    hints.max_width+adjustment,
						    hints.max_height+adjustment);
			      if(neaten_debug_level)
				   printf("\tmax: width = %d height = %d status = %s\n",
					  hints.max_width, hints.max_height,
					  (rval ? "SUCCESS" : "FAILURE"));		    
			 }
			 if (hints.flags & (USSize | PSize))
			 {
			      rval = Neaten_Set_Desired((int) w,
							hints.width+adjustment,
							hints.height+adjustment);
			      if(neaten_debug_level)
				   printf("\tdes: width = %d height = %d status = %s\n",
					  hints.width, hints.height,
					  (rval ? "SUCCESS" : "FAILURE"));		    
			 }
			 
			 /* if no max hints are given, set the max to the
			    max of the desired and current size 
			    */
			 if (!(hints.flags & PMaxSize))
			 {
			      int desired_width = hints.width+adjustment;
			      int desired_height = hints.height+adjustment;
			      
			      window_width = ((window_width > desired_width)
					      ? window_width :
					      desired_width);
			      window_height = ((window_height > desired_height) 
					       ? window_height :
					       desired_height);
			      rval = Neaten_Set_Max((int) w,
						    window_width,
						    window_height);
			      if(neaten_debug_level)
				   printf("\tmax: width = %d height = %d status = %s\n",
					  window_width-adjustment, window_height-adjustment,
					  (rval ? "SUCCESS" : "FAILURE"));
			 }
		    }
	       }
	  }
	  
	  if (UsePriorities == TRUE)
	  {
	       priority = MIN_PRIORITY;
	       if (opened_children > 1)
		    priority_increment = (MAX_PRIORITY - MIN_PRIORITY) /
			 (opened_children - 1);
	       for (ix = 0; ix < nchildren; ix++)
	       {
		    w = children[ix];
		    awi = GetAwmInfo(w);
		    if (!awi)
			 continue;
		    if (w != TOP_LEVEL && w != awi->icon)
			 continue;
		    is_iconic = IsIcon(w, (Window *) NULL);
		    
		    if (is_iconic != TRUE)
		    {
			 XGetWindowAttributes(dpy, w, &attrs);	    
			 if ((attrs.map_state != IsUnmapped) &&
			     (attrs.override_redirect == False))
			 {
			      Neaten_Set_Priorities((int) w, priority, priority);
			      if (neaten_debug_level)
			      {
				   GetName(w, name);
				   printf("%s: priority = %d\n", name, priority);
			      }
			      priority += priority_increment;
			 }
		    }
	       }
	  }
	  
	  if (FixTopOfStack == TRUE)
	  {
	       Neaten_Set_Options((int) stacktop, NEATEN_FALSE, NEATEN_TRUE);
	       if (neaten_debug_level)
	       {
		    GetName(stacktop, name);
		    printf("stacktop = %s\n", name);
	       }
	  }
	  
	  if(neaten_debug_level)
	       printf("mapped children = %d\topened_children = %d\n",
		      mapped_children, opened_children);
	  
	  Neaten_Desktop((RetainSize == FALSE), (KeepOpen == FALSE), Fill);
	  
	  for (ix = 0; ix < nchildren; ix++)
	  {
	       w = children[ix];
	       awi = GetAwmInfo(w);
	       if (!awi)
		    continue;
	       if (w != TOP_LEVEL && w != awi->icon)
		    continue;
	       if (Neaten_Get_Geometry((int) w, &iconic, &tiled,
				       &window_x, &window_y,
				       &window_width, &window_height,
				       (unsigned long) &adjustment) == 0)
		    continue;
	       GetName(w, name);
	       if(neaten_debug_level)
	       {
		    printf("%s iconic = %s  tiled = %s\n", name,
			   (iconic ? "TRUE" : "FALSE"),
			   (tiled ? "TRUE" : "FALSE"));
		    printf("   x = %d, y = %d, width = %d, height = %d\n",
			   window_x, window_y, window_width, window_height);
	       }
	       
	       wc.x = window_x;
	       wc.y = window_y;
	       wc.width = window_width;
	       wc.height = window_height;
	       wcmask = CWX | CWY | CWWidth | CWHeight;
	       
	       if (iconic == NEATEN_TRUE)
	       {
		    is_iconic = IsIcon(w, &icon_window);
		    if (is_iconic == TRUE)
		    {
			 wc.width -= adjustment;
			 wc.height -= adjustment;
			 
			 XConfigureWindow(dpy, w, wcmask, &wc);
			 if(tiled != NEATEN_TRUE)
			      XRaiseWindow(dpy, w);
		    }
		    else
		    {
			 /* have to go to the server to get the info, this
			    could have been avoided if the the variable
			    adjustment was a pointer to a structure that
			    contained the information for the icon and the
			    information for the window.  However, I chose
			    not to mess with all that memory management
			    */
			 GetIconHints(w, &icon_x, &icon_y,
				      &icon_width, &icon_height, &icon_border);
			 
			 wc.x -= icon_border;
			 wc.y -= icon_border;
			 wc.width -= (2 * icon_border) + SEPARATION;
			 wc.height -= (2 * icon_border) + SEPARATION;
			 
			 XAddToSaveSet(dpy, w);
			 XUnmapWindow(dpy, TOP_LEVEL);
			 XConfigureWindow(icon_window, wcmask, &wc);
			 XMapWindow(dpy, icon_window);
			 if (tiled != NEATEN_TRUE)
			      XRaiseWindow(dpy, icon_window);
		    }
	       }
	       else
	       {
		    wc.width -= adjustment;
		    wc.height -= adjustment;
		    adjustment -= SEPARATION;
		    wc.x += adjustment/2;
		    wc.y += adjustment/2;
		    /* I think this will be the outer win */
		    ConfigureWindow(w, wcmask, &wc);
		    if(tiled != NEATEN_TRUE)
			 XRaiseWindow(dpy, TOP_LEVEL);
	       }
	  }
	  
	  if (FixTopOfStack == TRUE)
	       XRaiseWindow(dpy, stacktop);
	  
	  XFree(children);
     }
     
     XBell(dpy, VOLUME_PERCENTAGE(Volume));
     XUngrabPointer(dpy, CurrentTime);
     Leave(FALSE)
}

static void
     GetIconHints(window, x, y, width, height, border)
Window window;
int *x, *y, *width, *height, *border;
{
     Window root;
     XWMHints *wmhints;
     XWindowAttributes attrs;
     int junk;
     
     *border = *x = *y = 0;
     /*
      * Process window manager hints.
      */ 
     if (wmhints = XGetWMHints(dpy, window)) 
     {
	  if (wmhints->flags&IconWindowHint)
	  {
	       XGetWindowAttributes(dpy, wmhints->icon_window, &attrs);
	       *x = attrs.x;
	       *y = attrs.y;
	       *width = attrs.width;
	       *height = attrs.height;
	       *border = attrs.border_width;
	  }
	  else if (wmhints->flags&IconPixmapHint) 
	  {
	       XGetWindowAttributes(dpy, wmhints->icon_pixmap, &attrs);
	       *width = attrs.width;
	       *height = attrs.height;
	  }
	  else
	  {
	       GetDefaultSize(window, width, height);
	  }
     }
     else
     {
	  GetDefaultSize(window, width, height);
     }
     
     /*
      * Fix up sizes by padding.
      */ 
     if (!wmhints || !(wmhints->flags&(IconPixmapHint|IconWindowHint))) {
	  *width += (HIconPad << 1);
	  *height += (VIconPad << 1);
     }
     
     if (wmhints && (wmhints->flags&IconPositionHint)) {
	  *x = wmhints->icon_x;
	  *y = wmhints->icon_y;
     }
}

static void
     GetName(w, name)
Window w;
char *name;
{
     Status rstatus;
     char *s = (char *) NULL;
     
     
     rstatus = XFetchName(dpy, w, &s);
     
     if (rstatus != (Status) 0 && s != (char *) NULL)
     {
	  strcpy(name, s);
	  XFree(s);
     }
     else
     {
	  strcpy(name, "UNKNOWN");
     }
}

void NeatenDebug(window, left, top, width, height, adjustment)
int window;
int left, top;
int width, height;
unsigned long adjustment;
{
     Window w = (Window) window;
     
     left += (int) ((float)adjustment/2.0);
     top += (int) ((float)adjustment/2.0);
     
     width -= adjustment;
     height -= adjustment;
     
     XMoveResizeWindow(dpy, w, left, top, width, height);
     XFlush(dpy);
}
#else /* NEATEN not installed */
/*ARGSUSED*/
Boolean Neaten(window, mask, button, x, y)
Window window;                          /* Event window. */
int mask;                               /* Button/key mask. */
int button;   	                        /* Button event detail. */
int x, y;                               /* Event mouse position. */
{
     fprintf(stderr, "awm: Warning: Neaten package not installed in ");
     fprintf(stderr, "this version of of awm.\n");
}
#endif NEATEN
