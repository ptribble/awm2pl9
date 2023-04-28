#ifndef lint
     static char sccs_id[] = "%W%  %H%";
#endif

/*
 *                             XLOCK V1.4
 *
 *                      A Terminal Locker for X11
 *
 *              Copyright (c) 1988 by Patrick J. Naughton
 *
 *                         All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation.
 *
 * Original version posted to comp.windows.x by:
 * Walter Milliken
 * (milliken@heron.bbn.com)
 * BBN Advanced Computers, Inc., Cambridge, MA
 *
 * Comments and additions may be sent the author at:
 *
 * naughton@sun.soe.clarkson.edu
 *
 * or by Snail Mail:
 * Patrick J. Naughton
 * 23 Pleasant Street, #3
 * Potsdam, NY  13676
 *         or
 * (315) 265-2853 (voice)
 *
 *
 * Revision History:
 * 12-Apr-88: Added root password override.
 *            Added screen saver override.
 *            Removed XGrabServer/XUngrabServer (Bad idea Phil...)
 *            Added access control handling instead.
 * 01-Apr-88: Added XGrabServer/XUngrabServer for more security.
 * 30-Mar-88: Removed startup password requirement (why did I add that?)
 *            Removed cursor to avoid phosphor burn.
 * 27-Mar-88: Rotate fractal by 45 degrees clockwise. (aesthetics)
 * 23-Mar-88: Added HOPALONG routines from Scientific American Sept. 86 p. 14.
 *            added password requirement for invokation
 *            removed option for command line password
 *            added requirement for display to be "unix:0".
 * 22-Mar-88: Recieved Walter Milliken's comp.windows.x posting.
 *
 * 20-Dec-88: Incorporated into awm. -jkh
 *
 */

#ifdef LOCKSCR
#include <pwd.h>
#include <math.h>
#include "awm.h"
#include <X11/Xutil.h>
char *crypt();
void ReadXString(), lockscreen(), GrabHosts(), UngrabHosts();
void iterate(), inithop();

static char no_bits[] = {0};

Window w;			/* window used to cover screen */
GC gc;

unsigned int width;     	/* width of screen */
unsigned int height;		/* height of screen */

Window rootw;			/* root window */
int screen;			/* current screen */
Colormap cmap;			/* colormap of current screen */
Cursor mycursor;                /* blank cursor */

unsigned long black_pixel;	/* pixel value for black */
unsigned long white_pixel;	/* pixel value for white */
XColor black_color;		/* color value for black */
XColor white_color;		/* color value for white */

int centerx, centery, iter, maxiter, range, color;
double a, b, c, i, j;

Display *dsp;


void ReadXString(s, slen)
char *s;
int slen;
{
     int bp;
     char c;
     XEvent evt;
     XKeyEvent *kpevt = (XKeyEvent *) &evt;
     char keystr[20];
     
     Entry("ReadXString")
	  
     bp = 0;
     while (1) {
	  if (XPending(dsp)) {
	       XNextEvent(dsp, &evt);
	       if (evt.type == KeyPress) {
		    if (XLookupString(kpevt, keystr, 20, (KeySym *) NULL, 
				      (XComposeStatus *) NULL) > 0) {
			 c = keystr[0];
			 switch (c) {
			 case 8:			/* ^H */
			      if (bp > 0) bp--;
			      break;
			 case 13:		/* ^M */
			      s[bp] = '\0';
			      Leave_void
			      case 21:		/* ^U */
				   bp = 0;
			      break;
			 default:
			      s[bp] = c;
			      if (bp < slen-1) bp++;
			 }
		    }
	       }
	  }
	  else iterate();
     }
     Leave_void
}


void lockscreen(dpy)
Display *dpy;
{
     char buf[10];
     char rootpass[10];
     XSetWindowAttributes attrs;
     XGCValues xgcv;
     struct passwd *pw;
     Pixmap lockc, lockm;   
     int timeout, interval, blanking, exposures; /* screen saver parameters */
     extern GC XCreateGC();
     
     Entry("lockscreen")
	  
     color = GetBoolRes("lock.useColor", FALSE);
     dsp = dpy;
     
     pw = getpwuid(0);
     strcpy(rootpass, pw->pw_passwd);
     
     rootw = DefaultRootWindow(dsp);
     screen = DefaultScreen(dsp);
     width = DisplayWidth(dsp, screen);
     height = DisplayHeight(dsp, screen);
     centerx = width / 2;
     centery = height / 2;
     range = (int) sqrt((double)centerx*centerx+(double)centery*centery);
     cmap = DefaultColormap(dsp, screen);
     
     black_pixel = BlackPixel(dsp, screen);
     black_color.pixel = black_pixel;
     XQueryColor(dsp, cmap, &black_color);
     
     white_pixel = WhitePixel(dsp, screen);
     white_color.pixel = white_pixel;
     XQueryColor(dsp, cmap, &white_color);
     
     attrs.background_pixel = black_pixel;
     attrs.override_redirect = True;
     attrs.event_mask = KeyPressMask;
     w = XCreateWindow(dsp, rootw, 0, 0, width, height, 0,
		       CopyFromParent, InputOutput, CopyFromParent,
		       CWOverrideRedirect | CWBackPixel | CWEventMask, &attrs);
     
     lockc = XCreateBitmapFromData(dsp, w, no_bits, 8, 1);
     lockm = XCreateBitmapFromData(dsp, w, no_bits, 8, 1);
     mycursor = XCreatePixmapCursor(dsp, lockc, lockm,
				    &black_color, &black_color,
				    0, 0);
     XFreePixmap(dsp, lockc);
     XFreePixmap(dsp, lockm);
     
     XMapWindow(dsp, w);
     
     xgcv.foreground = white_pixel;
     xgcv.background = black_pixel;
     gc = XCreateGC(dsp, w, GCForeground | GCBackground, &xgcv);
     
     XGetScreenSaver(dsp, &timeout, &interval, &blanking, &exposures);
     XSetScreenSaver(dsp, 0, 0, 0, 0); /* disable screen saver */
     
     XGrabKeyboard(dsp, w, True, GrabModeAsync, GrabModeAsync, CurrentTime);
     XGrabPointer(dsp, w, False, -1, GrabModeAsync, GrabModeAsync, None,
		  mycursor, CurrentTime);
     
     GrabHosts();
     
     pw = getpwuid(getuid());
     
     srandom(time(NULL));
     do {
	  inithop();
	  ReadXString(buf, 10);
     } while ((strcmp(crypt(buf, pw->pw_passwd), pw->pw_passwd))
	      && (strcmp(crypt(buf, rootpass), rootpass)));
     
     UngrabHosts();
     
     XUngrabPointer(dsp, CurrentTime);
     XUngrabKeyboard(dsp, CurrentTime);
     
     XSetScreenSaver(dsp, timeout, interval, blanking, exposures);
     XDestroyWindow(dsp, w);
     XFlush(dsp);
     Leave_void
}

XHostAddress *hosts;
int num_hosts, state;

void GrabHosts()
{
     Entry("GrabHosts")
	  
     XDisableAccessControl(dsp);
     hosts = XListHosts(dsp, &num_hosts, &state);
     XRemoveHosts(dsp, hosts, num_hosts);
     Leave_void
}


void UngrabHosts()
{
     Entry("UngrabHosts")
	  
     XEnableAccessControl(dsp);
     XAddHosts(dsp, hosts, num_hosts);
     XFree(hosts);
     Leave_void
}


void iterate()
{
     double oldj;
     register int k;
     
     Entry("iterate")
	  
     for (k=0;k<500;k++) {
	  oldj = j;
	  j = a - i;
	  i = oldj + ((i < 0) ? sqrt(fabs(b*i - c)) : -sqrt(fabs(b*i - c))); 
	  if (color)
	       XSetForeground(dsp, gc, (iter % 25) * 10 + 5); 

	  XDrawPoint(dsp, w, gc,
		     centerx + (int)(i+j), /* sneaky way to rotate +45 deg. */
		     centery - (int)(i-j));
	  iter++;
     }
     if (iter > maxiter)
	  inithop();
     Leave_void
}

void inithop()
{
     Entry("inithop")
	  
     a = random() % (range * 100) * (random()%2?-1.0:1.0) / 100.0;
     b = random() % (range * 100) * (random()%2?-1.0:1.0) / 100.0;
     c = random() % (range * 100) * (random()%2?-1.0:1.0) / 100.0;
     
     if (!(random()%3))
	  a /= 10.0;
     if (!(random()%2))
	  b /= 100.0;
     
     maxiter = (color?10000+random()%20000:20000+random()%50000);
     iter = 0;
     i = j = 0.0;
     XClearWindow(dsp, w);
     Leave_void
}
#endif LOCKSCR

