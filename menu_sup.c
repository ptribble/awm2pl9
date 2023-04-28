


#ifndef lint
static char *rcsid_menu_sup_c = "$Header: /usr/graph2/X11.3/contrib/windowmgrs/awm/RCS/menu_sup.c,v 1.3 89/02/07 22:40:04 jkh Exp $";
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

#include "awm.h"
#include <signal.h>
#

/* interface functions for RTL menus */

do_nothing()	/* like it says... */
{
}

Boolean check_booleans(menu, item)
RTLMenu menu;
RTLMenuItem item;
{
     Boolean *foo;

     Entry("check_booleans")

     foo = (Boolean *)RTLMenu_Data(menu, item);
     Leave(*foo)
}

Boolean toggle_booleans(menu, item)
RTLMenu menu;
RTLMenuItem item;
{
     Boolean *foo;

     Entry("togglet_booleans")

     foo = (Boolean *)RTLMenu_Data(menu, item);
     *foo = !(*foo);
     Leave(*foo)
}

/*ARGSUSED*/
int do_shell(menu, item, window)	/* Do a shell command */
RTLMenu menu;
RTLMenuItem item;
Window window;			/* not used */
{
     char *cmd;
     int status, pid, w;
     void (*istat)(), (*qstat)();

     Entry("do_shell")

     cmd = (char *)RTLMenu_Data(menu, item);
     if ((pid = fork()) == 0) {
	  setpgrp(0, getpid());
          signal(SIGHUP, SIG_DFL);
          signal(SIGQUIT, SIG_DFL);
          signal(SIGINT, SIG_DFL);
	  execl("/bin/sh", "sh", "-c", cmd, 0);
	  _exit(127);
     }
     istat = (int (*)())signal(SIGINT, SIG_IGN);
     qstat = (int (*)())signal(SIGQUIT, SIG_IGN);
     while ((w = wait(&status)) != pid && w != -1);
     if (w == -1)
	  status = -1;
     signal(SIGINT, istat);
     signal(SIGQUIT, qstat);
     Leave(status)
}

/*ARGSUSED*/
int do_text(menu, item, window)
RTLMenu menu;
RTLMenuItem item;
Window window;
{
     char *buff;

     Entry("do_text")

     buff = (char *)RTLMenu_Data(menu, item);
     XStoreBytes(dpy, buff, strlen(buff));
     Leave_void
}

/*ARGSUSED*/
int do_text_nl(menu, item, window)
RTLMenu menu;
RTLMenuItem item;
Window window;
{
     char *buff1, *buff2;

     Entry("do_text_nl")

     buff1 = (char *)RTLMenu_Data(menu, item);
     buff2 = (char *)malloc(strlen(buff1) + 2);
     strcpy(buff2, buff1);
     strcat(buff2, "\n");
     XStoreBytes(dpy, buff2, strlen(buff2));
     free(buff2);
     Leave_void
}

int do_awm_func(menu, item, window)
RTLMenu menu;
RTLMenuItem item;
Window window;
{
     int x, y, button;
     Boolean (*func)();
     extern Window Select_Window();

     Entry("do_awm_func")

     XSync(dpy, FALSE);
     if (window == RootWindow(dpy, scr))
          window = Select_Window(&x, &y, &button);
     func = (Boolean(*)())RTLMenu_Data(menu, item);
     Leave((*func)(window, 0, button, x, y))
}

/*ARGSUSED*/
int do_imm_func(menu, item, window)
RTLMenu menu;
RTLMenuItem item;
Window window;
{
     Boolean (*func)();

     Entry("do_imm_func")

     func = (Boolean(*)())RTLMenu_Data(menu, item);
     Leave((*func)(RootWindow(dpy, scr), 0, 0, 0, 0))
}

MenuInfo *FindMenu(s)
register char *s;
{
     MenuLink *ml;

     Entry("FindMenu")

     for (ml = Menus; ml; ml = ml->next)
	  if (!strcmp(s, ml->menu->name))
	       Leave(ml->menu)
     Leave((MenuInfo *) 0)
}

RTLMenu create_menu(m)
MenuInfo *m;
{
     ActionLine *ln;
     RTLMenuItem tmp;
     MenuInfo *side_menu;

     Entry("create_menu")

     if (!m)
	  yyerror("Internal error, create_menu passed null pointer");
     /*
      * Were we already created? This is possible if we were referenced
      * by somebody created before us.
      */
     if (m->menu)
	  Leave(m->menu)
     m->menu = RTLMenu_Create();
     /* make a name (or picture) label for this menu */
     tmp = RTLMenu_Append_Call(m->menu, m->name, m->pixmapname, do_nothing, 0);
     RTLMenu_Label_Entry(m->menu, tmp);
     ln = m->line;
     if (!ln) {
	  yyerror("Internal error in create_menu.");
	  fprintf(stderr, "Menu '%s' has no line list.\n", m->name);
	  exit(1);
     }
     if (!ln->name && !ln->pixmapname) {
	  fprintf(stderr, "awm: Action in menu '%s' has no name or backing pixmap\n",
		  m->name);
	  yyerror(".. aborting\n");
	  exit(1);
     }
     while (ln) {
	  switch (ln->type) {
	  case IsVar:
	       ln->item = RTLMenu_Append_Checkback(m->menu, ln->name,
						   ln->pixmapname,
						   check_booleans,
						   toggle_booleans,
						   ln->text);
	       break;
	       
	  case IsImmFunction:
	       ln->item = RTLMenu_Append_Call(m->menu, ln->name,
					      ln->pixmapname,
					      do_imm_func, ln->func);
	       break;

	  case IsUwmFunction:
	       ln->item = RTLMenu_Append_Call(m->menu, ln->name,
					      ln->pixmapname,
					      do_awm_func, ln->func);
	       break;

	  case IsMenuFunction:
	       if (!(side_menu = FindMenu(ln->text))) {
		    fprintf(stderr, "Unknown menu \"%s\" referenced in ",
			ln->text);
		    yyerror(" ..");
		    exit(1);
	       }
	       /* If we haven't created the referenced menu yet, create it now */
	       if (!side_menu->menu)
		    side_menu->menu = create_menu(side_menu);
	       ln->item = RTLMenu_Append_Submenu(m->menu, ln->name,
						 ln->pixmapname,
						 side_menu->menu);
	       break;
	       
	  case IsText:
	       ln->item = RTLMenu_Append_Call(m->menu, ln->name,
					      ln->pixmapname,
					      do_text, ln->text);
	       break;

	  case IsTextNL:
	       ln->item = RTLMenu_Append_Call(m->menu, ln->name,
					      ln->pixmapname,
					      do_text_nl, ln->text);
	       break;
	       
	  case IsShellCommand:
	       ln->item = RTLMenu_Append_Call(m->menu, ln->name,
					      ln->pixmapname,
					      do_shell, ln->text);
	       break;
	       
	  default:
	       fprintf(stderr, "create_menu, Unknown menu entry type %d\n",
		  ln->type);
	       break;
	  }
	  free(ln);
	  ln = ln->next;
     }
     Leave(m->menu)
}
