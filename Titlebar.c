


#ifndef lint
static char *rcsid_TitleBar_c = "$Header: /usr/graph2/X11.3/contrib/windowmgrs/awm/RCS/Titlebar.c,v 1.2 89/02/07 21:23:59 jkh Exp $";
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
 * MODIFICATION HISTORY
 *
 * 002 -- Jordan Hubbard, U.C. Berkeley.
 Title bar support routines.
 * 1.2 -- Fairly major restructuring to uncouple title bars and
 * border contexts.
 * 1.3 -- Handling of colors revamped significantly to deal with
 * different colormaps.
 */

#include "awm.h"
#include "X11/Xutil.h"
#include "X11/cursorfont.h"

unsigned int TB_wide, TB_high, TBB_wide, TBB_high, BC_wide, BC_high;
unsigned int BCB_wide, BCB_high;

extern XContext AwmContext;

#define BLANK_WIDE	16
#define BLANK_HIGH	16
static char blank_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static Boolean Title_res, Frame_res;

void Init_Titles()
{
  extern XFontStruct *GetFontRes();

     TFontInfo = GetFontRes("title.font", DEF_TITLE_FONT);
     TFontBoldInfo = GetFontRes("title.boldFont", (char *) NULL);
     TBackPixmapData = GetPixmapDataRes("title.pixmap", &TB_wide, &TB_high);
     TBoldPixmapData = GetPixmapDataRes("title.boldPixmap", &TBB_wide,
					&TBB_high);
     TForeground = GetStringRes("title.foreground", Foreground);
     TBackground = GetStringRes("title.background", Background);
     TTextForeground = GetStringRes("title.text.foreground", TForeground);
     TTextBackground = GetStringRes("title.text.background", TBackground);

     TCursor = GetIntRes("title.cursor", DEF_TITLE_CURSOR);

     TitleHeight = GetIntRes("title.height", 0);
     TitlePad = GetIntRes("title.pad", DEF_TITLE_PAD);
     NameOffset = GetIntRes("windowName.offset", 0);
     titleHeight = TFontInfo->max_bounds.ascent +
	  TFontInfo->max_bounds.descent + (TitlePad * 2);
     /* we've specified a bold pixmap, but no back pixmap. Use blank. */
     if (TBoldPixmapData && !TBackPixmapData) {
	  TBackPixmapData = blank_bits;
	  TB_wide = BLANK_WIDE;
	  TB_high = BLANK_HIGH;
     }
     if (TFontBoldInfo) {
	  int foo;
	  
	  foo = TFontBoldInfo->max_bounds.ascent +
	       TFontBoldInfo->max_bounds.descent + (TitlePad * 2);
	  if (foo > titleHeight)
	       titleHeight = foo;
     }
     if (titleHeight < gadgetHeight)
	  titleHeight = gadgetHeight;
     if (TitleHeight) /* Specified height overrides derived height */
	  titleHeight = TitleHeight;
     TitleCursor = XCreateFontCursor(dpy, TCursor);
     if (!TitleCursor) {
       char temp[80];
       sprintf("Init_Titles: Can't get title cursor #%d!\n", TCursor);
       Error(temp);
     }
     /* Gadgets can't exist without titles, so we'll init them here */
     GadgetPad = GetIntRes("gadget.pad", DEF_GADGET_PAD);
     GadgetBorder = GetIntRes("gadget.border", DEF_GADGET_BORDER);

     Title_res = TRUE;
}

void Init_Frames()
{
     
     Entry("Init_Frames")

     BForeground = GetStringRes("borderContext.foreground", Foreground);
     BBackground = GetStringRes("borderContext.background", Background);

     BCursor = GetIntRes("borderContext.cursor", DEF_BCONTEXT_CURSOR);

     BBackPixmapData = GetPixmapDataRes("borderContext.pixmap", &BC_wide,
					&BC_high);
     BBoldPixmapData = GetPixmapDataRes("borderContext.boldPixmap", &BCB_wide,
					&BCB_high);
     /* we've specified a bold BC pixmap, but no back pixmap. Use blank. */
     if (BBoldPixmapData && !BBackPixmapData) {
	  BBackPixmapData = blank_bits;
	  BC_wide = BLANK_WIDE;
	  BC_high = BLANK_HIGH;
     }
     FrameCursor = XCreateFontCursor(dpy, BCursor);
     if (!FrameCursor) {
       char temp[80];
       sprintf("Init_Frames: Can't get border cursor #%d!\n", BCursor);
       Error(temp);
     }
     Frame_res = TRUE;
     Leave_void
}

char *GetTitleName(w)
Window w;
{
     char *cp = 0, *tmp;
     
     Entry("GetTitleName")
	  
     if (!XFetchName(dpy, w, &cp) || !cp)
	  cp = "Untitled Window";
     tmp = (char *)malloc(strlen(cp) + 3);
     
     /*
      * We add a space on both ends for asthetic effect
      * (only apparent if we're using inverse video).
      */
     tmp[0] = ' ';
     strcpy(tmp + 1, cp);
     strcat(tmp, " ");
     Leave(tmp)
}

/*
 * Reparent window 'w' into a frame, making the frame the right
 * size to hold any other window related decorations that may be
 * added later. If window 'w' already has a frame, then it resizes
 * it if necessary.
 */
Window Reparent(w)
Window w;
{
     Window foster;
     XWindowAttributes xwa;
     XSetWindowAttributes swa;
     XWMHints *wm_hints;
     XSizeHints sz_hints;
     XClassHint class_hints;
     AwmInfoPtr awi;
     char *icon_name;
     int bch, bcv, x, y;
     unsigned int width, height, bw;
     int clnt_x, clnt_y;
     unsigned long valuemask;
     Boolean decorated = FALSE;
     Colormap cmap;
     Pixel bfore, bback;

     Entry("Reparent")

     awi = GetAwmInfo(w);
     if (!awi) {
	  fprintf(stderr, "Reparent: Window %x isn't registered!\n", w);
	  Leave((Window)NULL)
     }
     if (awi->frame && (awi->state & ST_DECORATED))
	  Leave((Window)NULL)

     if (!Frame_res)
	  Init_Frames();

     if (awi->attrs & AT_TITLE && !Title_res)
	  Init_Titles();

     if (!awi->frame)
	  XGetWindowAttributes(dpy, awi->client, &xwa);
     else
	  XGetWindowAttributes(dpy, awi->frame, &xwa);
     valuemask = CWEventMask | CWBorderPixel | CWBackPixel | CWColormap;
     swa.event_mask = (SubstructureRedirectMask | SubstructureNotifyMask);
     bw = xwa.border_width;
     x = xwa.x;
     y = xwa.y;
     height = xwa.height;
     width = xwa.width;
     cmap = xwa.colormap;
     bfore = GetPixel(BForeground, cmap);
     bback = GetPixel(BBackground, cmap);
     clnt_y = clnt_x = 0;
     if (awi->attrs & AT_BORDER) {
	  XSetWindowAttributes cswa;
     
	  swa.event_mask |= (ButtonPressMask | ButtonReleaseMask);
          bcv = BContext + 1;
	  bch = bcv * 2;

	  cswa.do_not_propagate_mask =
	       (ButtonPressMask | ButtonReleaseMask);
	  XChangeWindowAttributes(dpy, awi->client, CWDontPropagate, &cswa);
     }
     else
          bch = bcv = 0;
     if (FrameFocus)
	  swa.event_mask |= (LeaveWindowMask | EnterWindowMask);
     if (awi->attrs & AT_BORDER) {
	  swa.background_pixel = bfore;
	  swa.border_pixel = bback;
	  decorated = TRUE;
     }
     else if (awi->attrs & AT_TITLE) {
	  swa.background_pixel = GetPixel(TBackground, cmap);
	  swa.border_pixel = GetPixel(TForeground, cmap);
     }
     else {
	  swa.background_pixel = GetPixel(Background, cmap);
	  swa.border_pixel = GetPixel(Foreground, cmap);
     }
     swa.colormap = cmap;
     if (PushDown == FALSE) {
	  if (awi->attrs & AT_TITLE)
	       y -= (titleHeight + 2);
	  else if (awi->attrs & AT_BORDER)
	       y -= BContext;
     }
     /* calculate size for frame */
     if (awi->attrs & AT_TITLE)
	  height += titleHeight + 2;
     if (awi->attrs & AT_BORDER) {
	  if (!(awi->attrs & AT_TITLE))
	       height += (bcv * 2);
	  else
	       height += bcv;
	  width += bch;
     }
     if (awi->attrs & AT_TITLE)
	  clnt_y = titleHeight + 2;
     
     if (awi->attrs & AT_BORDER) {
	  if (!(awi->attrs & AT_TITLE))
	       clnt_y = BContext;
	  clnt_x = BContext;
     }

     if (!awi->frame) {
	  foster = XCreateWindow(dpy, RootWindow(dpy, scr), x, y, width,
				 height, (bw ? bw : (unsigned int) 1),
				 xwa.depth, xwa.class,
				 xwa.visual, valuemask, &swa);
	  if (!foster)
	       Error("Reparent: Can't create foster parent!");

	  if (awi->attrs & AT_BORDER)
	       XDefineCursor(dpy, foster, FrameCursor);

	  XAddToSaveSet(dpy, awi->client);
          XSetWindowBorderWidth(dpy, awi->client,
			       (unsigned int)((awi->attrs&AT_BORDER) ? 1 : 0));
	  XReparentWindow(dpy, awi->client, foster, clnt_x, clnt_y);
	  XSync(dpy, FALSE);

	  if (BBackPixmapData) { /* we have a border context tile */
	       awi->BC_back = GetPixmapFromCache(foster,
						 BBackPixmapData,
						 BC_wide, BC_high,
						 bfore, bback,
						 xwa.depth);
	       if (!awi->BC_back)
		    Error("Reparent: Can't get pixmap for border context.");
	       else
		    XSetWindowBackgroundPixmap(dpy, foster, awi->BC_back);
	  }
	  if (BBoldPixmapData) { /* we have a bold border context tile */
	       awi->BC_bold = GetPixmapFromCache(foster,
						 BBoldPixmapData,
						 BCB_wide, BCB_high,
						 bfore, bback,
						 xwa.depth);
	       if (!awi->BC_bold)
		    Error("Reparent: Can't get bold pixmap for border context.");
	  }
	  awi->frame = foster;
	  awi->border_width = bw;
	  awi->name = GetTitleName(awi->client);

	  /* Make titlebar "frame" inherit characteristics of victim */
	  XStoreName(dpy, foster, awi->name);
	  if (XGetIconName(dpy, w, &icon_name)) {
	       XSetIconName(dpy, foster, icon_name);
	       XFree(icon_name);
	  }
	  if (wm_hints = XGetWMHints(dpy, awi->client)) {
	       XSetWMHints(dpy, foster, wm_hints);
	       XFree(wm_hints);
	  }
	  XGetNormalHints(dpy, awi->client, &sz_hints);
	  XSetNormalHints(dpy, foster, &sz_hints);
	  XGetZoomHints(dpy, awi->client, &sz_hints);
	  XSetZoomHints(dpy, foster, &sz_hints);
	  class_hints.res_name = class_hints.res_class = (char *)NULL;
	  if (XGetClassHint(dpy, awi->client, &class_hints) == Success) {
	       if (class_hints.res_name || class_hints.res_class)
		    XSetClassHint(dpy, foster, &class_hints);
	  }
	  XSaveContext(dpy, foster, AwmContext, (caddr_t) awi);
     }
     else {
	  XMoveResizeWindow(dpy, awi->frame, xwa.x, xwa.y, width, height);
	  XMoveWindow(dpy, awi->client, clnt_x, clnt_y);
	  if (awi->attrs & AT_BORDER)
	       decorated = TRUE;
	  foster = awi->frame;
     }
#ifdef RAINBOW
     if (BorderHilite)
	  SetBorderPixmaps(awi, awi->grayPixmap);
#endif
     if (decorated)
	  awi->state |= ST_DECORATED;
     Leave(foster)
}
     
void AddTitle(w)
Window w;
{
     int bch;
     Window title;
     AwmInfoPtr awi;
     unsigned long valuemask;
     XWindowAttributes xwa;
     XSetWindowAttributes swa;
     XWindowChanges wc;
     Pixel tfore, tback;

     Entry("AddTitle")

     awi = GetAwmInfo(w);

     if (!awi) {
	  fprintf(stderr, "AddTitle: Window %x is not registered!\n", w);
	  Leave_void
     }
     if (!awi->frame) 
	  Leave_void
     if (awi->title)
	  Leave_void
     if (!(awi->attrs & AT_TITLE))
	  Leave_void

     XGetWindowAttributes(dpy, awi->client, &xwa);
     swa.event_mask = (ExposureMask | ButtonPressMask | ButtonReleaseMask);
     swa.colormap = xwa.colormap;
     tfore = GetPixel(TForeground, xwa.colormap);
     tback = GetPixel(TBackground, xwa.colormap);
     swa.background_pixel = tback;
     swa.border_pixel = tfore;
     valuemask = (CWEventMask | CWColormap | CWBackPixel | CWBorderPixel);
     if (awi->attrs & AT_BORDER)
	  bch = (BContext * 2) + 2;
     else
	  bch = 0;
     title = XCreateWindow(dpy, awi->frame, 0, 0, (xwa.width - 2) + bch,
			   titleHeight,
			   1, xwa.depth, xwa.class, xwa.visual,
			   valuemask, &swa);
     if (!title)
	  Error("AddTitle: Can't create title bar!");
     if (TBackPixmapData) {
	  awi->back = GetPixmapFromCache(title, TBackPixmapData,
					 TB_wide, TB_high,
					 tfore, tback,
					 xwa.depth);
	  if (!awi->back)
	       Error("AddTitle: Can't create pixmap for title background");
	  else
	       XSetWindowBackgroundPixmap(dpy, title, awi->back);
     }
     if (TBoldPixmapData) {
	  awi->bold = GetPixmapFromCache(title, TBoldPixmapData,
					 TBB_wide, TBB_high,
					 tfore, tback,
					 xwa.depth);
	  if (!awi->bold)
	       Error("AddTitle: Can't create pixmap for bold background");
     }
     wc.sibling = awi->client;
     wc.stack_mode = Above;
     XConfigureWindow(dpy, title, CWSibling|CWStackMode, &wc);
     XDefineCursor(dpy, title, TitleCursor);
     XSelectInput(dpy, title, (ExposureMask | ButtonPressMask |
		       ButtonReleaseMask));
     XMapWindow(dpy, title);
     awi->title = title;
     awi->state |= ST_DECORATED;
     XSaveContext(dpy, title, AwmContext, (caddr_t) awi);
}

/*ARGSUSED*/
Boolean FNoDecorate(w, mask, button, x, y)
Window w;
int mask, button, x, y;
{
     Entry("FNoDecorate")
	  
     if (w == RootWindow(dpy, scr))
	  Leave(FALSE)
     NoDecorate(w, FALSE);
     Leave(TRUE)
}

void NoDecorate(w, destroy)
Window w;
Boolean destroy;
{
     XWindowAttributes xwa;
     AwmInfoPtr awi;
     extern void DestroyGadgets();

     Entry("NoDecorate")

     awi = GetAwmInfo(w);
     if (!awi || !awi->frame)
	 Leave_void

     if (!(awi->state & ST_DECORATED))
	 Leave_void
     if (awi->gadgets)
	  DestroyGadgets(w);
     if (awi->title) {
	  XDeleteContext(dpy, awi->title, AwmContext);
	  if (awi->back)
	       FreePixmapFromCache(awi->back);
	  if (awi->bold)
	       FreePixmapFromCache(awi->bold);
	  XDestroyWindow(dpy, awi->title);
	  awi->title = awi->back = awi->bold = (Drawable)NULL;
     }
     if (destroy) {
	  XDeleteContext(dpy, awi->frame, AwmContext);
	  if (awi->BC_back)
	       FreePixmapFromCache(awi->BC_back);
	  if (awi->BC_bold)
	       FreePixmapFromCache(awi->BC_bold);
	  free(awi->name);
	  awi->name = (char *)NULL;
	  XDestroyWindow(dpy, awi->frame);
	  awi->frame = awi->BC_back = awi->BC_bold = (Drawable)NULL;
     }
     else {
	  XGetWindowAttributes(dpy, awi->client, &xwa);
	  XResizeWindow(dpy, awi->frame, (unsigned int) xwa.width,
			(unsigned int) xwa.height);
	  XMoveWindow(dpy, awi->client, 0, 0);
	  awi->state ^= ST_DECORATED;
     }
     XSync(dpy, FALSE);
     Leave_void
}

/*ARGSUSED*/
Boolean FDecorate(window, mask, button, x, y)
Window window;
int mask;
int button;
int x, y;
{
     Window fr;

     Entry("FDecorate")

     if (window == RootWindow(dpy, scr))
	  Leave(FALSE)
     fr = Decorate(window);
     if (fr)
	  XMapWindow(dpy, fr);
     XSync(dpy, FALSE);
     Leave(FALSE)
}

Window Decorate(w)
Window w;
{
     Window ret;

     Entry("Decorate")

     ret = Reparent(w);
     if (ret) {
	  AddTitle(ret);
	  PutGadgets(ret);
     }
     Leave(ret)
}

/*ARGSUSED*/
void PaintTitle(w, reversed)
Window w;
Boolean reversed;
{
     int x, y;
     XWindowAttributes xwa;
     XFontStruct *f_info;
     AwmInfoPtr awi;
     XGCValues gcv;
     Colormap cmap;

     Entry("PaintTitle")

     if (!(awi = IsTitled(w)))
	 Leave_void
     XGetWindowAttributes(dpy, awi->title, &xwa);
     cmap = xwa.colormap;
     if (reversed && TFontBoldInfo)
	  f_info = TFontBoldInfo;
     else
	  f_info = TFontInfo;
     gcv.font = f_info->fid;
     gcv.foreground = (reversed && !TFontBoldInfo) ?
	  GetPixel(TTextBackground, cmap) : GetPixel(TTextForeground, cmap);
     gcv.background = (reversed && !TFontBoldInfo) ?
	  GetPixel(TTextForeground, cmap) : GetPixel(TTextBackground, cmap);
     XChangeGC(dpy, awi->winGC, (GCFont | GCForeground | GCBackground), &gcv);
     XClearWindow(dpy, awi->title);
     if (ShowName) {	/* print window names? */
	  int textlen;

	  textlen = XTextWidth(f_info, awi->name, strlen(awi->name));

	  if (!NameOffset)
	       x = (xwa.width - textlen) / 2;
	  else {
	       if (NameOffset < 0)
		    x = (xwa.width - textlen) + NameOffset;
	       else
		    x = NameOffset;
	       if (x + textlen > xwa.width)
		    x = (xwa.width - textlen) + 1;
	  }
	  y = (xwa.height + f_info->max_bounds.ascent -
	       f_info->max_bounds.descent) / 2;

	  XDrawImageString(dpy, awi->title, awi->winGC, x, y, awi->name,
			   strlen(awi->name));
     }
     Leave_void
}

AwmInfoPtr IsTitled(w)
Window w;
{
     AwmInfoPtr tmp;

     Entry("IsTitled")

     if (tmp = GetAwmInfo(w))
	  if (tmp->title)
	       Leave(tmp)
     Leave((AwmInfoPtr)NULL)
}

/*
 * These routines deal with a sort of "pixmap cache" that is needed
 * to minimize the number of pixmaps created by awm. Awm operates on the
 * assumption that the user may have an arbitrary number of windows at
 * arbitrary depths. Since awm likes to create resources at the same
 * depth as the client window, some sort of mechanism is needed to insure
 * that as few are created as possible.
 */

struct _resInfo {
     struct _resInfo *next;
     Drawable res; 
     char *res_data;
     Pixel res_fore, res_back;
     int res_depth;
     int ref_count;
} *CacheHead;

struct _resInfo *allocResNode()
{
     struct _resInfo *ptr;

     Entry("allocResNode")

     ptr = (struct _resInfo *)malloc(sizeof(struct _resInfo));
     if (!ptr)
	  fprintf(stderr, "allocResNode: Out of memory!\n");
     Leave(ptr)
}

/*
 * Look for pixmap in cache and returns it or allocate new pixmap,
 * store it in cache and return it.
 */
Drawable GetPixmapFromCache(d, data, wide, high, fore, back, depth)
Drawable d;
char *data;
unsigned int wide, high;
Pixel fore, back;
int depth;
{
     struct _resInfo *cptr, *iptr;

     cptr = iptr = CacheHead;
     while (cptr) {
	  if (cptr->res_depth > depth)
	       break;
	  if (cptr->res_depth == depth) {
	       if (cptr->res_data > data)
		    break;
	       else if (cptr->res_data == data) {
		    if (cptr->res_fore == fore && cptr->res_back == back) {
			 cptr->ref_count++;
			 Leave(cptr->res)
		    }
	       }
	  }
	  iptr = cptr;
	  cptr = cptr->next;
     }
     /* fell through, create another resource */
     if (!iptr || cptr == CacheHead)	/* need to initialize list */
	  CacheHead = iptr = allocResNode();
     else {
	  iptr->next = allocResNode();
	  iptr = iptr->next;
     }
     iptr->next = cptr;
     iptr->res_depth = depth;
     iptr->res_data = data;
     iptr->res_fore = fore;
     iptr->res_back = back;
     iptr->res = XCreatePixmapFromBitmapData(dpy, d, data, wide, high,
					     fore, back, (unsigned int) depth);
     iptr->ref_count = 1;
     Leave(iptr->res)
}

/*
 * Free pixmap (and associated cache cell) if no longer needed.
 */
void FreePixmapFromCache(pix)
Pixmap pix;
{
     struct _resInfo *ptr, *tmp;

     ptr = tmp = CacheHead;
     while (ptr) {
	  if (ptr->res == pix)
	       break;
	  tmp = ptr;
	  ptr = ptr->next;
     }
     if (!ptr)
	  Leave_void
     if (--ptr->ref_count == 0) { /* last ref */
	  if (ptr == CacheHead)
	       CacheHead = CacheHead->next;
	  else
	       tmp->next = ptr->next;
	  XFreePixmap(dpy, ptr->res);
	  free(ptr);
     }
}
