


#ifndef lint
static char *rcsid_Menu_c = "$Header: /usr/graph2/X11.3/contrib/windowmgrs/awm/RCS/Menu.c,v 1.3 89/02/07 22:39:41 jkh Exp $";
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

#include <signal.h>
#include "X11/cursorfont.h"

/*
 * MODIFICATION HISTORY
 *
 * 000 -- J.Hubbard, Ardent Computer.
 * 	This file bears little resemblance to its former namesake.
 *	Because of massive changes to support RTL menus, the Menu()
 *      function is now little more than a bootstrap for RTL.
 * 1.2 -- Select_Window code fixed for icons.
 */

#ifndef lint
static char *sccsid = "@(#)Menu.c	3.8	1/24/86";
#endif

#include "awm.h"

/*ARGSUSED*/
Boolean DoMenu(window, mask, button, x, y, menu)
Window window;				/* Event window. */
int mask;				/* Button/key mask. */
int button;				/* Button event detail. */
int x, y;				/* Event mouse position. */
RTLMenu menu;
{
     RTLPoint pos;
     
     Entry("DoMenu")
	  
     pos.x = x;
     pos.y = y;
     if (!menu)
	Leave(FALSE)
     RTLMenu_Enter(menu, button, 0, window, pos);
     Leave(TRUE)
}

/*ARGSUSED*/
Boolean DoAction(window, mask, button, x, y, menu, action)
Window window;				/* Event window. */
int mask;				/* Button/key mask. */
int button;				/* Button event detail. */
int x, y;				/* Event mouse position. */
RTLMenu menu;
ActionLine *action;
{
     char *buff, *cmd;
     int status, pid, w;
     void (*istat)(), (*qstat)();

     Entry("DoAction")

     switch(action->type) {
     case IsText:
	  XStoreBytes(dpy, action->text, strlen(action->text));
	  break;

     case IsTextNL:
	  buff = (char *)malloc(strlen(action->text) + 2);
	  strcpy(buff, action->text);
	  strcat(buff, "\n");
	  XStoreBytes(dpy, buff, strlen(buff));
	  free(buff);
	  break;

     case IsShellCommand:
	  cmd = action->text;
	  if ((pid = fork()) == 0) {
	       setpgrp(0, getpid());
	       signal(SIGHUP, SIG_DFL);
	       signal(SIGQUIT, SIG_DFL);
	       signal(SIGINT, SIG_DFL);
	       execl("/bin/sh", "sh", "-c", cmd, 0);
	       _exit(127);
	  }
	  istat = (void (*)())signal(SIGINT, SIG_IGN);
	  qstat = (void (*)())signal(SIGQUIT, SIG_IGN);
	  while ((w = wait(&status)) != pid && w != -1);
	  if (w == -1)
	       status = -1;
	  signal(SIGINT, istat);
	  signal(SIGQUIT, qstat);
	  break;

     default:
	  fprintf(stderr, "awm: Warning: Unknown action type %d (%s) invoked\n",
		  action->type, action->text);
	  break;
     }
}

/*
 * Routine to let user select a window using the mouse
 */

Window Select_Window(x, y, button)
int *x, *y, *button;
{
     XEvent event;
     Window target_win, root;
     int root_x, root_y, x2, y2;
     unsigned int ptrmask;
     AwmInfoPtr awi;
     
     Entry("Select_Window")
	  
     /* Make the target cursor */
     if (XGrabPointer( dpy, RootWindow(dpy, scr),
		      TRUE, (unsigned int) EVENTMASK, GrabModeAsync,
		      GrabModeAsync, None,
		      TargetCursor, CurrentTime )
	 != GrabSuccess ) {
	  fprintf(stderr, "awm (Select_Window): Can't grab the mouse.");
	  Leave(RootWindow(dpy, scr))
     }
     Snatched = TRUE;
     /* Select a window */
     while (TRUE) {
	  XPeekEvent(dpy, &event);
	  if (event.type != ButtonPress) {
	       GetButton(&event);
	       continue;
	  }
          XNextEvent(dpy, &event);
	  XQueryPointer(dpy, RootWindow(dpy, scr), &root,
			&target_win, &root_x, &root_y, &x2, &y2, &ptrmask);

	  *x = root_x;
	  *y = root_y;
	  *button = event.xbutton.button;
	  if (target_win == 0)
	       target_win = event.xbutton.window;
	  XUngrabPointer(dpy, CurrentTime);      /* Done with pointer */
	  if (awi = GetAwmInfo(target_win))
	       if (target_win != awi->icon)
	       	     target_win = (awi->frame) ? awi->frame : awi->client;
	  Snatched = FALSE;
	  Leave(target_win)
     }
}
