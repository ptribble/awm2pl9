#include <unistd.h>



#ifndef lint
static char *rcsid_exp_path_c = "$Header: /usr/graph2/X11.3/contrib/windowmgrs/awm/RCS/exp_path.c,v 1.2 89/02/07 21:24:42 jkh Exp $";
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

#if defined(PCS) || defined(titan)
#include <unistd.h>
#endif
#include <pwd.h>
#include <stdio.h>
#include "awm.h"

/*
 * WARNING: This code is obscure.
 * Modify at your Own Risk.
 */
char *expand_file(s)
register char *s;
{
     static char tmp[256], *cp, err[80];

     Entry("expand_file")
     
     /* zero tmp */
     tmp[0] = 0;
     if (*s == '/')
	  Leave(s)
     if (*s == '~') {
	  if (s[1] == '/') { /* It's $HOME */
	       if (!(cp = (char *)getenv("HOME"))) {
		    sprintf(err, "expand: Can't find $HOME!\n");
		    yywarn(err);
		    Leave(0)
	       }
	       strcpy(tmp, cp);
	       strcat(tmp, s + 1);
	       Leave(tmp)
	  }
	  else { /* it's another user */
	       struct passwd *pwd;
	       char uname[32];
	       int i;

	       for (i = 1; s[i] != '/'; i++)
		    uname[i - 1] = s[i];
	       uname[i] = '\0';
	       pwd = getpwnam(uname);
	       if (!pwd) {
		    sprintf(err, "expand: user '%s' not found.\n", uname);
		    yywarn(err);
		    Leave(0)
	       }
	       strcpy(tmp, pwd->pw_dir);
	       strcat(tmp, s + i);
	       Leave(tmp)
	  }
     }
     else
	  Leave(s)
}

char *deblank(s)
register char *s;
{
     Entry("deblank")

     if (!s)
	  Leave(s)
     while (*s && (*s == ' ' || *s == '\t'))
	  s++;
     Leave(s)
}

char *expand_from_path(s)
register char *s;
{
     char tmp[256], *tm;
     int i, plen;

     Entry("expand_from_path")

     tmp[0] = '\0';
     s = deblank(s);
     if (!s || !*s)
	  Leave(0)
     s = expand_file(s);
     if (!s)
	  Leave(0)
     if (!access(s, R_OK))
	  Leave(s)
     /*
      * If it starts with a slash, we know it either expanded and couldn't
      * be found, or that it started with a slash in the first place and
      * just plain couldn't be found.
      */
     if (*s == '/')
	  Leave(0)
     /*
      * At this stage we haven't found the file by name, so it's time to
      * search the path.
      */
     if (!awmPath || !*awmPath)
	  Leave(0)
     plen = strlen(awmPath);
     i = 0;
     while (1) {
	  int p, l;
	  
	  tmp[0] = '\0';
	  while (i < plen && awmPath[i] == ' ' || awmPath[i] == '\t')
	       i++;
	  for (p = i; p < plen && awmPath[p] != ' ' && awmPath[p] != '\t'; p++)
	       tmp[p - i] = awmPath[p];
	  if (!*tmp)
	       Leave(0)
	  tmp[p - i] = '\0';
	  i = p;
	  tm = expand_file(tmp);
	  if (!tm || !*tm)
	       continue;
	  l = strlen(tm);
	  if (l < 1)
	       continue;
	  if (tm[l - 1] != '/') { /* append / if necessary */
	       tm[l] = '/';
	       tm[++l] = '\0';
	  }
	  strcat(tm, s);
	  if (!access(tm, R_OK))
	       Leave(tm)
     }
}
