#include <stdlib.h>
#include <unistd.h>
#include <sys/fcntl.h>

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
 *  Western Software Lab. Convert to X11.
 * 002 -- Jordan Hubbard, U.C. Berkeley. Add title bar context stuff.
 * 003 -- Jordan Hubbard, Ardent Computer. Added gadgets, border contexts.
 * 1.6 -- Various irritating changes. Support for WMSTATE..
 */

#include <signal.h>
#include "awm.h"
#include <sys/ioctl.h>
#include "X11/Xutil.h"
#include "X11/cursorfont.h"

#ifdef titan
#include <sys/file.h>
#endif

#ifdef PCS
#include <unistd.h>
#include <bsd/sys/time.h>
#endif
#ifdef PROFIL
#include <signal.h>
/*
 * Dummy handler for profiling.
 */
ptrap()
{
     exit(0);
}
#endif

Boolean NeedRootInput=FALSE;
MenuOptionsMask options;
char *sfilename;
char execfile[NAME_LEN]; /* Pointer to file to exec with -e */
Boolean Snatched;
Boolean desktop_execd = TRUE;
extern FILE *yyin;
extern int errorStatus;
extern int ErrorHandler();
extern XContext AwmContext;

/*
 * Main program.
 */
main(argc, argv, environ)
int argc;
char **argv;
char **environ;
{
     int hi;			/* Button event high detail. */
     int lo;			/* Button event low detail. */
     int x, y;                  /* Mouse X and Y coordinates. */
     int root_x, root_y;        /* Mouse root X and Y coordinates. */
     int cur_x, cur_y;		/* Current mouse X and Y coordinates. */
     int down_x, down_y;	/* mouse X and Y at ButtonPress. */
     int str_width;             /* Width in pixels of output string. */
     unsigned int pop_width, pop_height; /* Pop up window width and height. */
     int context;		/* Root, window, or icon context. */
     unsigned int ptrmask;	/* for QueryPointer */
     Boolean func_stat;		/* If true, function swallowed a ButtonUp. */
     Boolean delta_done;	/* If true, then delta functions are done. */
     Boolean local;		/* If true, then do not use system defaults. */
     Boolean nolocal;		/* If true, ignore user defaults */
     register Binding *bptr;	/* Pointer to Bindings list. */
     char *root_name;		/* Root window name. */
     char *display = NULL;	/* Display name pointer. */
     char message[128];		/* Error message buffer. */
     char *rc_file;		/* Pointer to $HOME/.awmrc. */
     Window event_win;          /* Event window. */
     Window root;		/* Root window for QueryPointer. */
     AwmInfoPtr awi;
     XEvent button_event; 	/* Button input event. */
     XGCValues xgc;		/* to create font GCs */
     XSetWindowAttributes swa;
     unsigned long valuemask;
     Window bwin;		/* Button window */
     int num;
     
     /* next three variables are for XQueryWindow */
     Window junk;
     Window *kiddies = NULL;
     unsigned int nkids;
     
     Entry("main")
	  
#ifdef PROFIL
	  signal(SIGTERM, ptrap);
#endif
     
     /* 
      * Parse the command line arguments.
      */
     Argv = argv;
     Environ = environ;
     local = nolocal = FALSE;
     argc--, argv++;
     /*
      * The destruction of '-e' args below is to prevent the startup
      * command from being invoked again if we do an f.restart (see
      * Restart.c and Argv)
      */
     while (argc) {
	  if (**argv == '-') {
	       if (!strcmp(*argv, "-display") || !strcmp(*argv, "-d")) {
		    argc--; argv++;
		    if (argc <= 0)
			 Usage();
		    display = *argv;
	       }
	       else if (!(strcmp(*argv, "-f"))) {
		    argc--, argv++;
		    if ((argc == 0) || (Startup_File[0] != '\0'))
			 Usage();
		    strncpy(Startup_File, *argv, NAME_LEN);
	       }
	       else if (!(strcmp(*argv, "-e"))) {
		    strcpy(*argv, "--");	/* destroy the arg */
		    argc--; argv++;
		    if ((argc == 0) || (execfile[0] != '\0'))
			 Usage();
		    desktop_execd = FALSE; /* assume we have desktop to run */
		    strncpy(execfile, *argv, NAME_LEN);
	       }
	       /* Destroyed arg, skip over what used to be filename for -e */
	       else if (!(strcmp(*argv, "--"))) {
		    argv += 2; argc -= 2;
		    continue;
	       }
	       else if (!(strcmp(*argv, "-b")))
		    local = TRUE;
	       else if (!(strcmp(*argv, "-i")))
		    nolocal = TRUE;
	       
	       else Usage();
	  }
	  else
	       Usage();
	  argc--, argv++;
     }
 
#ifdef CONSOLE
     if (access("/dev/console", W_OK) == 0) {
          freopen("/dev/console", "w", stderr);
	  freopen("/dev/console", "w", stdout);
     }
#endif /* CONSOLE */
     /* Open the damn display */
     if ((dpy = XOpenDisplay(display)) == NULL) {
	  fprintf(stderr, "awm: Unable to open display\n");
	  exit(1);
     }

     scr = DefaultScreen(dpy);
     
     /*
      * Set XErrorFunction to be non-terminating.
      */
     XSetErrorHandler(ErrorHandler);
     
     /*
      * Force child processes to disinherit the TCP file descriptor.
      * This helps shell commands forked and exec'ed from menus
      * to work properly. God knows if this works under SysV.
      */
     if ((status = fcntl(ConnectionNumber(dpy), F_SETFD, 1)) == -1) {
	  perror("awm: child cannot disinherit TCP fd");
	  Error("TCP file descriptor problems");
     }
     
     /*
      * Initialize the menus for later use.
      */
     RTLMenu_Option_Set(options, rightoffset);
     RTLMenu_Initialize(options);
     
     /* Init the context manager stuff */
     AwmContext = XUniqueContext();
     
     /*
      * Get all the defaults we expect from the resource manager.
      */
     FocusSetByUser = FALSE;
     Get_Defaults();
#if defined(WMSTATE)
     wm_state_atom = XInternAtom(dpy, "WM_STATE", False);
#endif /* WMSTATE */

     /*
      * Initialize the default bindings.
      */
     if (!local)
	  InitBindings();
     
     /*
      * Read in and parse $HOME/.awmrc, if it exists.
      */
     if (!nolocal) {
	  extern char *getenv();
	  char *home = getenv("HOME");
	  if( !home )
		  Error( "Cannot find home" );
	  sfilename = rc_file = malloc(NAME_LEN);
	  sprintf(rc_file, "%s/.awmrc", home);
	  if ((yyin = fopen(rc_file, "r")) != NULL) {
	       Lineno = 1;
	       yyparse();
	       fclose(yyin);
	       if (Startup_File_Error)
		    Error("Bad .awmrc file...aborting");
	  }
     }
     /* 
      * Read in and parse the startup file from the command line, if
      * specified.
      */
     if (Startup_File[0] != '\0') {
	  sfilename = Startup_File;
	  if ((yyin = fopen(Startup_File, "r")) == NULL) {
	       sprintf(message, "Cannot open startup file '%s'", Startup_File);
	       Error(message);
	  }
	  Lineno = 1;
	  yyparse();
	  fclose(yyin);
	  if (Startup_File_Error)
	       Error("Bad startup file...aborting");
     }

     if (Startup_File_Error)
	  Error("Bad startup file...aborting");

     /*
      * Catch some of the basic signals so we don't get rudely killed without
      * cleaning up first.
      */
#ifdef titan
          signal(SIGHUP, Quit);
     signal(SIGTERM, Quit);
     signal(SIGQUIT, Quit);
     signal(SIGINT, Quit);

     /* ignore SIGTTOU */

     signal(SIGTTOU, SIG_IGN);
#else

     if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
	  signal(SIGHUP, Quit);
     if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
	  signal(SIGTERM, Quit);
     if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
	  signal(SIGTERM, Quit);
     if (signal(SIGINT, SIG_IGN) != SIG_IGN)
	  signal(SIGINT, Quit);
#endif

     /*
      * If the root window has not been named, name it.
      */
     status = XFetchName(dpy, RootWindow(dpy, scr), &root_name);
     if (root_name == NULL) 
	  XStoreName(dpy, RootWindow(dpy, scr), " X Root Window ");
     else
	  XFree(root_name);
     /* register the root window */
     RegisterWindow(RootWindow(dpy, scr));

     ScreenHeight = DisplayHeight(dpy, scr);
     ScreenWidth = DisplayWidth(dpy, scr);

     /*
      * Create the menus. This function also sticks the RTL menu "handle"
      * into the appropriate binding after it's been created and initialized.
      */
     Create_Menus();

     /*
      * check the gadgets.
      */
     if (CheckGadgets())
	  Error("Error in gadget declarations. Exiting...\n");

     /*
      * Store all the cursors.
      */
     StoreCursors();
     
     /* 
      * grab the mouse buttons according to the map structure
      */
     Grab_Buttons();
     
     /* 
      * watch for initial window mapping and window destruction
      */
     
     errorStatus = False;
     swa.event_mask = (SubstructureRedirectMask | FocusChangeMask |
		       (NeedRootInput ? EVENTMASK |
			OwnerGrabButtonMask : 0));
     XChangeWindowAttributes(dpy, RootWindow(dpy, scr), CWEventMask, &swa);
     XSync(dpy, False);
     if (errorStatus)
	  Error("Hmmm.. Looks like you're running another window manager!\n");
     /*
      * Before we go creating more windows, we buzz through the ones that
      * are currently mapped and reparent and/or select on them as necessary
      * (for autoraise and titles).
      */
     
     if (XQueryTree(dpy, DefaultRootWindow(dpy), &junk, &junk, &kiddies,
		    &nkids)) {
	  int i;

	  for (i = 0; i < (int)nkids; i++) {
	       XWindowAttributes xwa;
	       Window transient;
	       AwmInfoPtr awi;
	       unsigned long event_mask;
	       
	       XGetWindowAttributes(dpy, kiddies[i], &xwa);
	       
	       /* check to see if it's a popup or something */
	       XGetTransientForHint(dpy, kiddies[i], &transient);
	       if (xwa.class == InputOutput && xwa.map_state == IsViewable &&
		   xwa.override_redirect == False && transient == None) {
		    awi = RegisterWindow(kiddies[i]);
		    awi->state |= ST_PLACED;
		    /* Possibly add a frame */
		    FDecorate(kiddies[i],0,0,0,0);
		    event_mask = PropertyChangeMask | FocusChangeMask;
		    if (!awi->frame || !FrameFocus)
			 event_mask |= (EnterWindowMask | LeaveWindowMask);
#ifndef RAINBOW
		    SetBorderPixmaps(awi, GrayPixmap);
#else
		    SetBorderPixmaps(awi, awi->grayPixmap);
#endif
		    XSelectInput(dpy, kiddies[i], event_mask);
	       }
	  }
	  if (kiddies && *kiddies)
	      XFree(kiddies);
     }
     /*
      * Calculate size of the resize pop-up window.
      */
     valuemask = CWBorderPixel | CWBackPixel;
     swa.border_pixel = PBorder;
     swa.background_pixel = PBackground;
     if (SaveUnder) {
	  swa.save_under = True;
	  valuemask |= CWSaveUnder;
     }
     str_width = XTextWidth(PFontInfo, PText, strlen(PText));
     pop_width = str_width + (PPadding << 1);
     PWidth = pop_width + (PBorderWidth << 1);
     pop_height = PFontInfo->ascent + PFontInfo->descent + (PPadding << 1);
     PHeight = pop_height + (PBorderWidth << 1);

     /*
      * Create the pop-up window.  Create it at (0, 0) for now.  We will
      * move it where we want later.
      */
     Pop = XCreateWindow(dpy, RootWindow(dpy, scr),
			 0, 0,
			 pop_width, pop_height,
			 PBorderWidth,
			 0,
			 CopyFromParent,
			 CopyFromParent,
			 valuemask,
			 &swa);
     if (Pop == FAILURE)
	  Error("Can't create pop-up dimension display window.");
     
     /*
      * Create graphics context.
      */
     xgc.font = IFontInfo->fid;
     xgc.graphics_exposures = FALSE;
     xgc.foreground = IForeground;
     xgc.background = IBackground;
     IconGC = XCreateGC(dpy, 
			RootWindow(dpy, scr),
			(GCForeground | GCBackground | GCGraphicsExposures |
			 GCFont), &xgc);
     
     xgc.foreground = PForeground;
     xgc.background = PBackground;
     xgc.font = PFontInfo->fid;
     PopGC = XCreateGC(dpy, 
		       RootWindow(dpy, scr),
		       (GCForeground | GCBackground | GCFont), &xgc);
     xgc.line_width = DRAW_WIDTH;
     xgc.foreground = DRAW_VALUE;
     xgc.function = DRAW_FUNC;
     xgc.subwindow_mode = IncludeInferiors;
     DrawGC = XCreateGC(dpy, RootWindow(dpy, scr), 
			GCLineWidth | GCForeground | GCFunction |
			GCSubwindowMode, &xgc);

     /*
      * As our last "startup" task, invoke the execfile if was specified.
      */
     if (!desktop_execd) {
	  if (access(execfile, X_OK) == 0) {
	       if (fork() == 0) {
#ifdef SYSV
		   setpgrp();
#else
		   setpgrp(0, getpid());
#endif
		    signal(SIGHUP, SIG_DFL);
		    signal(SIGQUIT, SIG_DFL);
		    signal(SIGINT, SIG_DFL);
		    execl("/bin/sh", "sh", "-c", execfile, 0);
		    _exit(127);
	       }
	       else
		    desktop_execd = TRUE;
	  }
     }
     /*
      * Tell the user we're alive and well.
      */
     XBell(dpy, VOLUME_PERCENTAGE(Volume));
     
     /* 
      * Main command loop.
      */
     while (TRUE) {
	  
	  delta_done = func_stat = FALSE;
	  
	  /*
	   * Get the next mouse button event.  Spin our wheels until
	   * a ButtonPressed event is returned.
	   * Note that mouse events within an icon window are handled
	   * in the "GetButton" function or by the icon's owner if
	   * it is not awm.
	   */
	  while (TRUE) {
	       if (!GetButton(&button_event))
		    continue;
	       else if (button_event.type == ButtonPress)
		    break;
	  }
	  bwin = button_event.xbutton.window;
	  /* save mouse coords in case we want them later for a delta action */
	  down_x = button_event.xbutton.x;
	  down_y = button_event.xbutton.y;
	  
	  /*
	   * Okay, determine the event window and mouse coordinates.
	   */
	  status = XTranslateCoordinates(dpy, 
					 bwin,
					 RootWindow(dpy, scr),
					 button_event.xbutton.x, 
					 button_event.xbutton.y,
					 &x, &y,
					 &event_win);
	  if (status == 0)
	       continue;
	  
	  awi = GetAwmInfo(bwin);
	  if (!awi)
	       continue;

          if (awi->frame == bwin)
               context = BORDER;
	  else if (awi->title == bwin)
	       context = TITLE;
	  else if (IsGadgetWin(bwin, &num))
	       context = GADGET | (1 << (num + BITS_USED));
	  else if (awi->icon == bwin)
	       context = ICON;
	  else if (awi->client == RootWindow(dpy, scr)) {
	       event_win = RootWindow(dpy, scr);
	       context = ROOT;
	  }
	  else
	       context = WINDOW;

	  /*
	   * Get the button event detail.
	   */
	  lo = button_event.xbutton.button;
	  hi = button_event.xbutton.state;
	  
	  /*
	   * Determine which function was selected and invoke it.
	   */
	  for(bptr = Blist; bptr; bptr = bptr->next) {
	       if ((bptr->button != lo) ||
		   (((int)bptr->mask & ModMask) != hi))
		    continue;
	       
	       if ((bptr->context & context) != context) {
		    continue;
               }
	       if (!(bptr->mask & ButtonDown))
		    continue;
	       
	       /*
		* Found a match! Invoke the function.
		*/
	       if ((*bptr->func)(event_win, (int)bptr->mask & ModMask,
				 bptr->button,
				 x, y,
				 bptr->menu, bptr->menuname))
		    func_stat = TRUE;
	       break;
	  }
	  
	  /*
	   * If the function ate the ButtonUp event, then restart the loop.
	   */
	  
	  if (func_stat)
	       continue;
	  while (TRUE) {
	       /*
		* Wait for the next button event.
		*/
	       if (XPending(dpy) && GetButton(&button_event)) {
		    bwin = button_event.xbutton.window;
		    /*
		     * If it's not a release of button that was pressed,
		     * don't do the function bound to 'ButtonUp'.
		     */
		    if (button_event.type != ButtonRelease)
			 break;
		    if (lo != button_event.xbutton.button)
			 break;
		    if ((hi | ButtonMask(lo)) != button_event.xbutton.state)
			 break;
		    /*
		     * Okay, determine the event window and mouse coordinates.
		     */
		    status = XTranslateCoordinates(dpy, 
						   bwin,
						   RootWindow(dpy, scr),
						   button_event.xbutton.x,
						   button_event.xbutton.y,
						   &x, &y,
						   &event_win);
		    if (status == BadWindow)
			 break;
		    awi = GetAwmInfo(bwin);
		    if (!awi)
			 continue;
		    if (awi->frame == bwin)
			 context = BORDER;
		    else if (awi->title == bwin)
			 context = TITLE;
		    else if (IsGadgetWin(bwin, &num))
			 context = GADGET | (1 << (num + BITS_USED));
		    else if (awi->icon == bwin)
			 context = ICON;
		    else if (awi->client == RootWindow(dpy, scr)) {
			 event_win = RootWindow(dpy, scr);
			 context = ROOT;
		    }
		    else
			 context = WINDOW;
		    /*
		     * Determine which function was selected and invoke it.
		     */
		    for(bptr = Blist; bptr; bptr = bptr->next) {
			 
			 if ((bptr->button != lo) ||
			     (((int)bptr->mask & ModMask) != hi))
			      continue;
			 
			 if (!((bptr->context & context) == context)) {
			      continue;
                         }
			 
			 if (!(bptr->mask & ButtonUp))
			      continue;
			 
			 /*
			  * Found a match! Invoke the function.
			  */
			 
			 (*bptr->func)(event_win,
				       (int)bptr->mask & ModMask,
				       bptr->button,
				       x, y,
				       bptr->menu, bptr->menuname);
			 break;
		    }
		    break;
	       }
	       XQueryPointer(dpy, RootWindow(dpy, scr),
			     &root, &junk, &root_x, &root_y, &cur_x, &cur_y,
			     &ptrmask);
	       if (!delta_done &&
		   (abs(cur_x - x) > Delta || abs(cur_y - y) > Delta)) {
		    /*
		     * Delta functions are done once (and only once.)
		     */
		    delta_done = TRUE;
		    
		    /*
		     * Determine the new event window's coordinates from the
		     * original ButtonPress event.
		     */
		    status = XTranslateCoordinates(dpy, bwin,
						   RootWindow(dpy, scr),
						   down_x, down_y, &x, &y,
						   &junk);
		    if (status == 0)
			 break;

		    /*
		     * Determine the event window and context.
		     */
		    if (awi->frame == bwin)
			 context = BORDER;
		    else if (awi->title == bwin)
			 context = TITLE;
		    else if (IsGadgetWin(bwin, &num))
			 context = GADGET | (1 << (num + BITS_USED));
		    else if (awi->icon == bwin)
			 context = ICON;
		    else if (awi->client == RootWindow(dpy, scr)) {
			 event_win = RootWindow(dpy, scr);
			 context = ROOT;
		    }
		    else
			 context = WINDOW;
		    /*
		     * Determine which function was selected and invoke it.
		     */
		    for(bptr = Blist; bptr; bptr = bptr->next) {
			 
			 if ((bptr->button != lo) ||
			     (((int)bptr->mask & ModMask) != hi))
			      continue;
			 
			 if (!((bptr->context & context) == context))
			      continue;
			 
			 if (!(bptr->mask & DeltaMotion))
			      continue;
			 
			 /*
			  * Found a match! Invoke the function.
			  */
			 
			 if ((*bptr->func)(event_win,
					   (int)bptr->mask & ModMask,
					   bptr->button,
					   x, y,
					   bptr->menu, bptr->menuname)) {
			      func_stat = TRUE;
			      break;
			 }
		    }
		    /*
		     * If the function ate the ButtonUp event,
		     * then restart the loop.
		     */
		    
		    if (func_stat)
			 break;
	       }
	  }
     }
}

/*
 * Get defaults from the resource manager. Most of these things used to be
 * in the rc file, but they really belong here, I think.
 */
Get_Defaults()
{
     /*
      * Get the pixmap search path, if it exists.
      */
     awmPath = GetStringRes("path", (char *) NULL);

     /* Default foreground/background colors (text) */
     Foreground = GetStringRes("foreground", "black");
     Background = GetStringRes("background", "white");
     Reverse = GetBoolRes("reverse", FALSE);
     
     if (Reverse) { /* Swap the foreground and background */
	  char *tmp;
	  
	  tmp = Foreground;
	  Foreground = Background;
          Background = tmp;
     }
     WBorder = GetStringRes("border.foreground", Foreground);
     
     Autoselect = GetBoolRes("autoselect", FALSE);
     Autoraise = GetBoolRes("autoraise", FALSE);
     Borders = GetBoolRes("borderContexts", FALSE);
     ConstrainResize = GetBoolRes("constrainResize", FALSE);
     FrameFocus = GetBoolRes("frameFocus", FALSE);
     Freeze = GetBoolRes("freeze", FALSE);
     Grid = GetBoolRes("grid", FALSE);
     InstallColormap = GetBoolRes("installColormap", FALSE);
     Titles = GetBoolRes("titles", FALSE);
     IconLabels = GetBoolRes("icon.labels", FALSE);
     ILabelTop = GetBoolRes("icon.labelTop", FALSE);
     PushDown = GetBoolRes("title.push", FALSE);
     UseGadgets = GetBoolRes("gadgets", FALSE);
     Hilite = GetBoolRes("hilite", FALSE);
     BorderHilite = GetBoolRes("border.hilite", Hilite);
     RootResizeBox = GetBoolRes("rootResizeBox", FALSE);
     ResizeRelative = GetBoolRes("resizeRelative", FALSE);
     NIcon = GetBoolRes("normali", TRUE);
     ShowName = GetBoolRes("showName", TRUE);
     NWindow = GetBoolRes("normalw", TRUE);
     Push = GetBoolRes("pushRelative", FALSE);
     SaveUnder = GetBoolRes("saveUnder", FALSE);
     Wall = GetBoolRes("wall", FALSE);
     WarpOnRaise = GetBoolRes("warpOnRaise", FALSE);
     WarpOnIconify = GetBoolRes("warpOnIconify", FALSE);
     WarpOnDeIconify = GetBoolRes("warpOnDeIconify", FALSE);
     Zap = GetBoolRes("zap", FALSE);
     
     HIconPad = GetIntRes("icon.hPad", DEF_ICON_PAD);
     VIconPad = GetIntRes("icon.vPad", DEF_ICON_PAD);
     RaiseDelay = GetIntRes("raiseDelay", DEF_RAISE_DELAY);
     PBorderWidth = GetIntRes("popup.borderWidth", DEF_POP_BORDER_WIDTH);
     IBorderWidth = GetIntRes("icon.borderWidth", DEF_ICON_BORDER_WIDTH);
     PPadding = GetIntRes("popup.pad", DEF_POP_PAD);
     MBorderWidth = GetIntRes("menu.borderWidth", DEF_MENU_BORDER_WIDTH);
     MItemBorder = GetIntRes("menu.itemBorder", 1);
     MDelta = GetIntRes("menu.delta", DEF_MENU_DELTA);
     MPad = GetIntRes("menu.pad", DEF_MENU_PAD);
     Delta = GetIntRes("delta", DEF_DELTA);
     Volume = GetIntRes("volume", DEF_VOLUME);
     Pushval = GetIntRes("push", DEF_PUSH);
     BContext = GetIntRes("borderContext.width", DEF_BCONTEXT_WIDTH);

     ForeColor = GetColorRes("foreground", BlackPixel(dpy, scr));
     BackColor = GetColorRes("background", WhitePixel(dpy, scr));
     IForeground = GetColorRes("icon.foreground", ForeColor);
     IBackground = GetColorRes("icon.background", BackColor);
     IBorder = GetColorRes("icon.border", IForeground);
     ITextForeground = GetColorRes("icon.text.foreground", IForeground);
     ITextBackground = GetColorRes("icon.text.background", IBackground);
     PBorder = GetColorRes("popup.border", ForeColor);
     PForeground = GetColorRes("popup.foreground", PBorder);
     PBackground = GetColorRes("popup.background", BackColor);
     MForeground = GetColorRes("menu.foreground", ForeColor);
     MBackground = GetColorRes("menu.background", BackColor);
     MBorder = GetColorRes("menu.border", MForeground);

     /*
      * Create and store the grey and solid pixmaps, and default icon pixmap
      */
     GrayPixmap = XCreatePixmapFromBitmapData(dpy, RootWindow(dpy, scr),
					      gray_bits,
					      gray_width, gray_height,
					      ForeColor,
					      BackColor,
					      (unsigned)DefaultDepth(dpy,scr));

     SolidPixmap = XCreatePixmapFromBitmapData(dpy, RootWindow(dpy, scr),
					       solid_bits,
					       solid_width, solid_height,
					       ForeColor,
					       BackColor,
					       (unsigned)DefaultDepth(dpy,
								      scr));
     
     IDefPixmap = XCreatePixmapFromBitmapData(dpy, RootWindow(dpy, scr),
					       xlogo32_bits,
					       xlogo32_width, xlogo32_height,
					       IForeground,
					       IBackground,
					       (unsigned)DefaultDepth(dpy,
								      scr));
 
     IFontInfo = GetFontRes("icon.font", DEF_ICON_FONT);
     PFontInfo = GetFontRes("popup.font", DEF_POPUP_FONT);
     MFontInfo = GetFontRes("menu.font", DEF_MENU_FONT);
     MBoldFontInfo = GetFontRes("menu.boldFont", DEF_BOLD_FONT);

     
     IBackPixmap = GetPixmapRes("icon.backpixmap", GrayPixmap, IForeground,
				IBackground);
     IDefPixmap =  GetPixmapRes("icon.defpixmap", IDefPixmap, IForeground,
				IBackground);

#ifdef NEATEN
     AbsMinWidth = GetIntRes("neaten.absMinWidth", DEFAULT_ABS_MIN);
     AbsMinHeight = GetIntRes("neaten.absMinHeight", DEFAULT_ABS_MIN);
     
     RetainSize = GetBoolRes("neaten.retainSize", TRUE);
     KeepOpen = GetBoolRes("neaten.keepOpen", TRUE);
     Fill = GetBoolRes("neaten.fill", TRUE);
     UsePriorities = GetBoolRes("neaten.usePriorities", TRUE);
     FixTopOfStack = GetBoolRes("neaten.fixTopOfStack", TRUE);
     
     PrimaryIconPlacement = GetStringRes("neaten.primaryIconPlacement",
					 DEF_PRIMARY_PLACEMENT);
     SecondaryIconPlacement = GetStringRes("neaten.secondaryIconPlacement",
					   DEF_SECONDARY_PLACEMENT);
#endif
     
     Leave_void
}

/*
 * Look up string resource "string". If undefined, return "def_val"
 */
char *GetStringRes(string, def_val)
char *string, *def_val;
{
     char *cp;
     
     Entry("GetStringRes")
     
     if ((cp = XGetDefault(dpy, NAME, string)) ||
	 (cp = XGetDefault(dpy, CLASS, string))) {
	  if (!strlen(cp))
	       Leave(def_val)
	  else
	       Leave(cp)
     }
     Leave(def_val)
}

/*
 * Look up boolean resource "string". If undefined, return "def_val"
 */
Boolean GetBoolRes(string, def_val)
char *string;
Boolean def_val;
{
     char *cp;
     
     Entry("GetBoolRes")
     
     if ((cp = XGetDefault(dpy, NAME, string)) ||
	 (cp = XGetDefault(dpy, CLASS, string)))
	  if (Pred(cp) > 0)
		def_val = TRUE;
     Leave(def_val)
}

/*
 * Look up integer resource "string". If undefined or non-numeric,
 * return def_val.
 */
int GetIntRes(string, def_val)
char *string;
int def_val;
{
     char *cp;
     
     Entry("GetIntRes")
     
     if ((cp = XGetDefault(dpy, NAME, string)) ||
	 (cp = XGetDefault(dpy, CLASS, string))) {
	  if (!strlen(cp) || !((*cp >= '0' && *cp <= '9') || *cp == '-'))
	       Leave(def_val)
	  Leave(atoi(cp))
     }
     Leave(def_val)
}

/*
 * Try to load pixmap file named by resource "string". Return 0 if
 * unsuccessful. Otherwise, set width, height and return data.
 */
char *GetPixmapDataRes(string, wide, high)
char *string;
unsigned int *wide, *high;
{
     char *cp, *cp2;

     Entry("GetPixmapDataRes")
     
     if ((cp = XGetDefault(dpy, NAME, string)) ||
	 (cp = XGetDefault(dpy, CLASS, string))) {
	  char *data;
	  int junk;
	  
	  cp2 = expand_from_path(cp);
	  if (!cp2) {
	       fprintf(stderr, "awm: Can't find pixmap file '%s' for '%s'\n",
		       cp, string);
	       Leave(0)
	  }
	  if (XmuReadBitmapDataFromFile(cp2, wide, high, &data, &junk, &junk)
	      != BitmapSuccess) {
	       fprintf(stderr, "awm: Can't read pixmap file '%s' for '%s'.\n",
		       cp, string);
	  }
	  else {
	       Leave(data)
	  }
     }
     Leave(0)
}

/*
 * Try to allocate pixmap resources named by "string", return "def_pix"
 * if not found.
 */
Pixmap GetPixmapRes(string, def_pix, fg, bg)
char *string;
Pixmap def_pix;
Pixel fg, bg;
{
     char *data;
     Pixmap tmp;
     unsigned int wide, high;

     Entry("GetPixmapRes")

     if (data = GetPixmapDataRes(string, &wide, &high)) {
	  tmp = XCreatePixmapFromBitmapData(dpy, RootWindow(dpy, scr),
					    data, wide, high, fg, bg,
					    (unsigned) DefaultDepth(dpy, scr));
	  XFree(data);
	  if (!tmp) {
	       fprintf(stderr,
		       "awm: Can't create pixmap '%s', using default.\n",
		       string);
	       tmp = def_pix;
	  }
     }
     else
	  tmp = def_pix;
     Leave(tmp)
     }

/*
 * Try to allocate color resource named by "string", return "color"
 * if not found.This routine is only used for allocating colors from
 * the default colormap.
 */
Pixel GetColorRes(string, color)
char *string;
Pixel color;
{
     char *cp;
     Pixel tmp_color;
     Boolean status;
     extern Pixel LookupColor();
     
     Entry("GetColorRes")
     
     if ((cp = XGetDefault(dpy, NAME, string)) ||
	 (cp = XGetDefault(dpy, CLASS, string))) {
	  tmp_color = LookupColor(cp, DefaultColormap(dpy, scr), &status);
	  if (!status) /* lookup succeeded */
	       Leave(tmp_color)
	  else
	       tmp_color = color;
     }
     else
	  tmp_color = color;
     Leave(tmp_color)
}

/*
 * Try and get font resource "string", using "default" if not found. If
 * neither are available, use server default.
 */

XFontStruct *GetFontRes(string, dflt)
char *string, *dflt;
{
     char *cp;
     XFontStruct *tmp;
     static XFontStruct *def_font = 0;
     
     Entry("GetFontRes")
     
     if (!def_font)
	  def_font = XLoadQueryFont(dpy, DEF_FONT);
     
     if ((cp = XGetDefault(dpy, NAME, string)) ||
	 (cp = XGetDefault(dpy, CLASS, string))) {
	  if (tmp = XLoadQueryFont(dpy, cp))
	       Leave(tmp)
	  else if (dflt)
	       fprintf(stderr, "awm: Can't load %s '%s', trying '%s'.\n",
		       string, cp, dflt);
     }
     if (!dflt) /* NULL means we're not supposed to try again */
	  Leave(NULL)
     if (tmp = XLoadQueryFont(dpy, dflt))
	  Leave(tmp)
     else
	  fprintf(stderr, "awm: Can't open default font '%s', using server default.\n", dflt);
     Leave(def_font)
}

AwmInfoPtr GetAwmInfo(w)
Window w;
{
     static AwmInfoPtr tmp;
     
     Entry("GetAwmInfo")
     
     if (!XFindContext(dpy, w, AwmContext, (caddr_t *) &tmp))
	  Leave(tmp)
     else
	  Leave((AwmInfoPtr)NULL)
}

AwmInfoPtr RegisterWindow(w)
Window w;
{
     AwmInfoPtr tmp;
     XClassHint clh;
     XWMHints *wm_hints;
     char *cp;

     Entry("RegisterWindow")
     
     tmp = (AwmInfoPtr)malloc(sizeof(AwmInfo));
     if (tmp == (AwmInfoPtr)NULL) {
	  fprintf(stderr, "awm: Woe! No memory to register window.\n");
	  Leave((AwmInfoPtr)NULL)
     }
     tmp->client = w;
     tmp->title = tmp->frame = tmp->icon = (Window)0;
     tmp->gadgets = (Window *)0;
     tmp->name = (char *)0;
     tmp->own = (Boolean)FALSE;
     tmp->back = tmp->bold = tmp->BC_back = tmp->BC_bold =
	  tmp->iconPixmap = (Pixmap)0;
     tmp->state = ST_WINDOW;
#ifdef WMSTATE
     tmp->wm_state.icon=0;
     tmp->wm_state.state=NormalState;
     XChangeProperty(dpy,w,wm_state_atom,wm_state_atom,32,PropModeReplace,
		     (char *) &tmp->wm_state,2);
#endif /* WMSTATE */
     tmp->winGC = XCreateGC(dpy, w, (unsigned long)0, (XGCValues *) NULL);
     /*
      * Determine attribute set by first turning on all attributes
      * that are set by booleans and then (possibly) override them
      * by checking to see what an individual window might want.
      */
     tmp->attrs = AT_NONE;
     if (Titles)
	  tmp->attrs |= AT_TITLE;
     if (UseGadgets)
	  tmp->attrs |= AT_GADGETS;
     if (Borders)
	  tmp->attrs |= AT_BORDER;
     if (Autoraise)
	  tmp->attrs |= AT_RAISE;
     if (IconLabels)
	  tmp->attrs |= AT_ICONLABEL;
     if (wm_hints = XGetWMHints(dpy, w)) {
	  if (wm_hints->input)
     		tmp->attrs |= AT_INPUT;
	  XFree(wm_hints);
     }
#ifdef RAINBOW
     tmp->foreColor = ForeColor;
     tmp->backColor = BackColor;
     tmp->grayPixmap = GrayPixmap;
     tmp->solidPixmap = SolidPixmap;
#endif
     clh.res_name = clh.res_class = (char *)NULL;
     XGetClassHint(dpy, w, &clh);
     if (clh.res_class) {
#ifdef RAINBOW
	  Pixel tmp_color;
	  Boolean status;
	  
	  if (cp = (XGetDefault(dpy, clh.res_class, "wm_option.foreground"))){
		  tmp_color = LookupColor( cp, DefaultColormap( dpy, scr ), & status );
		  if( !status )
			  tmp->foreColor = tmp_color;
	  }

	  if (cp = (XGetDefault(dpy, clh.res_class, "wm_option.background"))){
		  tmp_color = LookupColor( cp, DefaultColormap( dpy, scr ), & status );
		  if( !status )
			  tmp->backColor = tmp_color;
	  }
#endif
	  
	  if (cp = (XGetDefault(dpy, clh.res_class, "wm_option.title")))
	       tmp->attrs = SetOptFlag(tmp->attrs, AT_TITLE, Pred(cp));
	  
	  if (cp = (XGetDefault(dpy, clh.res_class, "wm_option.gadgets")))
	       tmp->attrs = SetOptFlag(tmp->attrs, AT_GADGETS, Pred(cp));

	  if (cp = (XGetDefault(dpy, clh.res_class,"wm_option.borderContext")))
	       tmp->attrs = SetOptFlag(tmp->attrs, AT_BORDER, Pred(cp));

	  if (cp = (XGetDefault(dpy, clh.res_class, "wm_option.autoRaise")))
	       tmp->attrs = SetOptFlag(tmp->attrs, AT_RAISE, Pred(cp));

	  if (cp = (XGetDefault(dpy, clh.res_class, "wm_option.icon.labels")))
	       tmp->attrs = SetOptFlag(tmp->attrs, AT_ICONLABEL, Pred(cp));
     }
     if (clh.res_name) {
#ifdef RAINBOW
	  Pixel tmp_color;
	  Boolean status;

	  if (cp = (XGetDefault(dpy, clh.res_name, "wm_option.foreground"))){
		  tmp_color = LookupColor( cp, DefaultColormap( dpy, scr ), & status );
		  if( !status )
			  tmp->foreColor = tmp_color;
	  }

	  if (cp = (XGetDefault(dpy, clh.res_name, "wm_option.background"))){
		  tmp_color = LookupColor( cp, DefaultColormap( dpy, scr ), & status );
		  if( !status )
			  tmp->backColor = tmp_color;
	  }
#endif
	  if (cp = (XGetDefault(dpy, clh.res_name, "wm_option.title")))
	       tmp->attrs = SetOptFlag(tmp->attrs, AT_TITLE, Pred(cp));
	  
	  if (cp = (XGetDefault(dpy, clh.res_name, "wm_option.gadgets")))
	       tmp->attrs = SetOptFlag(tmp->attrs, AT_GADGETS, Pred(cp));

	  if (cp = (XGetDefault(dpy, clh.res_name, "wm_option.borderContext")))
	       tmp->attrs = SetOptFlag(tmp->attrs, AT_BORDER, Pred(cp));

	  if (cp = (XGetDefault(dpy, clh.res_name, "wm_option.autoRaise")))
	       tmp->attrs = SetOptFlag(tmp->attrs, AT_RAISE, Pred(cp));

	  if (cp = (XGetDefault(dpy, clh.res_name, "wm_option.icon.labels")))
	       tmp->attrs = SetOptFlag(tmp->attrs, AT_ICONLABEL, Pred(cp));
     }
#ifdef RAINBOW
     /* Has a different fore/back colour been given? */
     if( (tmp->foreColor != ForeColor) || (tmp->backColor != BackColor) ){
	  /*
	   * Create and store the grey and solid pixmaps
	   */
	     tmp->grayPixmap = 
	       XCreatePixmapFromBitmapData(dpy, RootWindow(dpy, scr),
					   gray_bits,
					   gray_width, gray_height,
					   tmp->foreColor,
					   tmp->backColor,
					   (unsigned)DefaultDepth(dpy, scr));
	     
	     tmp->solidPixmap = 
	       XCreatePixmapFromBitmapData(dpy, RootWindow(dpy, scr),
					   solid_bits,
					   solid_width, solid_height,
					   tmp->foreColor,
					   tmp->backColor,
					   (unsigned)DefaultDepth(dpy, scr));
	     
     }
#endif
     XSaveContext(dpy, w, AwmContext, (caddr_t) tmp);
     Leave(tmp)
}

/*
 * Sets bit "flag" conditionally, based on state of "mask" and
 * "predicate" (mask denotes current state, predicate denotes
 * whether change is desired).
 */
int SetOptFlag(mask, flag, predicate)
int mask, flag, predicate;
{
     Entry("SetOptFlag")

     switch (predicate) {
     case -1:
	  Leave(mask)
          break;

     case 0:
          if (mask & flag)
	       Leave(mask ^ flag)
          else
	       Leave(mask)
	  break;

     case 1:
	  Leave(mask | flag)
	  break;
     }
     Leave(-1);
}

/*
 * check whether a string denotes an "on" or "off" value. Return 0
 * if "off", 1 if "on" and -1 if undefined (or null).
 */
Pred(s)
char *s;
{
     int i, len;
     char *tmp;
     Boolean ret = -1;
     Entry("Pred")

     if (!s)
	  Leave(-1)
     len = strlen(s);
     if (!len)
	  Leave(-1)
     tmp = (char *)malloc(len + 1);
     if (!tmp) {
	  fprintf(stderr, "awm: Pred: Can't allocate storage for '%s'!\n", s);
	  Leave(-1)
     }
     strcpy(tmp, s);
     for (i = 0; i < len; i++)
	  if (tmp[i] >= 'A' && tmp[i] <= 'Z')
	       tmp[i] += 32;
     if (*tmp == 'y' || !strcmp(tmp, "on")
	 || !strcmp(tmp, "true")
	 || !strncmp(tmp, "enable", 6))
	  ret = 1;
     else if (*tmp == 'n' || !strcmp(tmp, "off")
	      || !strcmp(tmp, "false")
	      || !strncmp(tmp, "disable", 7))
	  ret = 0;
     free(tmp);
     Leave(ret)
}

/*
 * Initialize the default bindings.  First, write the character array
 * out to a temp file, then point the parser to it and read it in.
 * Afterwards, we unlink the temp file.
 */
InitBindings()
{
     char *mktemp();
     char *tempfile;
     register FILE *fp;		/* Temporary file pointer. */
     register char **ptr;	/* Default bindings string array pointer. */
     
     Entry("InitBindings")
     
     /*
      * Create and write the temp file.
      */
     /*
      * This used to just call mktemp() on TEMPFILE, which was very
      * evil as it involved writing on a string constant. This extra
      * mastication is necessary to prevent that.
      */
     tempfile = (char *)malloc(strlen(TEMPFILE) + 1);
     strcpy(tempfile, TEMPFILE);
     sfilename = mktemp(tempfile);
     if ((fp = fopen(tempfile, "w")) == NULL) {
	  perror("awm: cannot create temp file");
	  exit(1);
     }
     for (ptr = DefaultBindings; *ptr; ptr++) {
	  fputs(*ptr, fp);
	  fputc('\n', fp);
     }
     fclose(fp);
     
     /*
      * Read in the bindings from the temp file and parse them.
      */
     if ((yyin = fopen(tempfile, "r")) == NULL) {
	  perror("awm: cannot open temp file");
	  exit(1);
     }
     Lineno = 1;
     yyparse();
     fclose(yyin);
     unlink(tempfile);
     free(tempfile);
     if (Startup_File_Error)
	  Error("Bad default bindings...aborting");
     
     /*
      * Parse the system startup file, if one exists.
      */
     if ((yyin = fopen(SYSFILE, "r")) != NULL) {
	  sfilename = SYSFILE;
	  Lineno = 1;
	  yyparse();
	  fclose(yyin);
	  if (Startup_File_Error)
	       Error("Bad system startup file...aborting");
     }
     Leave_void
}

/*
 * Create the menus and alter any appropriate bindings so that the RTL menu
 * handle is passed along in subsequent actions.
 */
Create_Menus()
{
     Binding *bptr;
     MenuInfo *minfo;
     MenuLink *lnk;
     extern MenuInfo *FindMenu();
     extern RTLMenu create_menu();
     
     Entry("Create_Menus")
     
     /*
      * We start with the bindings list because we don't want to bother
      * creating a menu that's been declared but not referenced.
      */
     for(bptr = Blist; bptr; bptr = bptr->next) {
	  if (bptr->func == DoMenu) {
	       if (minfo = FindMenu(bptr->menuname))
		    bptr->menu = create_menu(minfo);
	       else {
		    fprintf(stderr, "awm: non-existent menu reference: \"%s\"\n",
			    bptr->menuname);
		    Startup_File_Error = TRUE;
	       }
	  }
     }
     for (lnk = Menus; lnk; lnk = lnk->next) {
	  free(lnk->menu);
	  free(lnk);
     }
     Leave_void
}

/*
 * Grab the mouse buttons according to the bindings list.
 */

Grab_Buttons()
{
     Binding *bptr;
     
     Entry("Grab_Buttons")
     
     /*
      * don't grab buttons if you don't have to - allow application
      * access to buttons unless context includes window.
      */
     for (bptr = Blist; bptr; bptr = bptr->next) {
	  if (bptr->context == ROOT)
	       NeedRootInput = TRUE;
	  else if (bptr->context & WINDOW) /* We gotta grab on windows */
	       GrabAll(bptr->mask);
     }
     Leave_void
}

/*
 * Register a grab on all windows in the hierarchy. This is better than
 * registering a grab on the RootWindow since it leaves button/key chords
 * available for other contexts.
 */
GrabAll(mask)
unsigned int mask;
{
     unsigned int junk, nkids;
     Window *kiddies;
     
     Entry("GrabAll")
     
     if (XQueryTree(dpy, DefaultRootWindow(dpy), &junk, &junk, &kiddies,
		    &nkids)) {
	  int i;
	  
	  for (i = 0; i < (int)nkids; i++) {
	       Window transient;
	       XWindowAttributes xwa;
	       
	       /* check to see if it's a popup or something */
	       XGetWindowAttributes(dpy, kiddies[i], &xwa);
	       XGetTransientForHint(dpy, kiddies[i], &transient);
	       if (xwa.class == InputOutput && xwa.map_state == IsViewable &&
		   xwa.override_redirect == False && transient == None)
		    Grab(mask, kiddies[i]);
	  }
     }
     else
	  Error("awm: Can't XQueryTree in GrabAll!\n");
     Leave_void
}

/*
 * Grab a mouse button according to the given mask.
 */
Grab(mask, w)
unsigned int mask;
Window w;
{
     unsigned int m = LeftMask | MiddleMask | RightMask;
     
     Entry("Grab")
     
     switch (mask & m) {
     case LeftMask:
	  XGrabButton(dpy, LeftButton, mask & ModMask, w, TRUE, EVENTMASK,
		      GrabModeAsync, GrabModeAsync, None, LeftButtonCursor);
	  break;
	  
     case MiddleMask:
	  XGrabButton(dpy, MiddleButton, mask & ModMask, w, TRUE, EVENTMASK,
		      GrabModeAsync, GrabModeAsync, None, MiddleButtonCursor);
	  break;
	  
     case RightMask:
	  XGrabButton(dpy, RightButton, mask & ModMask, w, TRUE, EVENTMASK,
		      GrabModeAsync, GrabModeAsync, None, RightButtonCursor);
	  break;
     }
     Leave_void
}

/*
 * Restore cursor to normal state.
 */
ResetCursor(button)
int button;
{
     Entry("ResetCursor")
     
     switch (button) {
     case LeftButton:
	  XChangeActivePointerGrab(dpy, EVENTMASK, LeftButtonCursor,
				   CurrentTime);
	  break;
	  
     case MiddleButton:
	  XChangeActivePointerGrab(dpy, EVENTMASK, MiddleButtonCursor,
				   CurrentTime);
	  break;
	  
     case RightButton:
	  XChangeActivePointerGrab(dpy, EVENTMASK, RightButtonCursor,
				   CurrentTime);
	  break;
     }
     Leave_void
}

/*
 * error routine for .awmrc parser
 */
yyerror(s)
char*s;
{
     Entry("yyerror")
     
     fprintf(stderr, "awm: %s: Line %d: %s\n", sfilename, Lineno, s);
     Startup_File_Error = TRUE;
     Leave_void
}

/*
 * warning routine for .awmrc parser
 */
yywarn(s)
char*s;
{
     Entry("yywarn")
     
     fprintf(stderr, "awm: Warning: %s: Line %d: %s\n", sfilename, Lineno, s);
     Leave_void
}

/*
 * Print usage message and quit.
 */
Usage()
{
     Entry("Usage")
     
     fputs("Usage: awm [-b] [-i] [-f <file>] [-e <file>] [<host>:<display>]\n\n",
	   stderr);
     fputs("The -b option bypasses system and default bindings\n", stderr);
     fputs("The -i option ignores the $HOME/.awmrc file\n", stderr);
     fputs("The -f option specifies an alternate startup file\n", stderr);
     fputs("The -e option specifies a program/script to exec after startup\n",
	   stderr);
     exit(1);
}

/*
 * error handler for X I/O errors
 */
/*ARGSUSED*/
XIOError(dsp)
Display *dsp;
{
     /* perror("awm"); */
     exit(3);
}
