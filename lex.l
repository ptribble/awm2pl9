%{




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
 * 001 -- Jordan Hubbard, Ardent Computer
 *  Handle comments differently.
 */

#include "y.tab.h"
#include "X11/Xos.h"

#undef	YYLMAX

#ifdef	LEXBUFSIZ
#define YYLMAX	LEXBUFSIZ
#else
#define YYLMAX	1024
#endif

char *dequote();
char *stash();

extern int Lineno;

%}
W	[ \t]+
%%

#.*\n				{ return(COMMENT); }	/* Comments. */
\\\n				{ Lineno++; }		/* Continuation. */
{W}				{ ; }	/* Whitespace. */
[^ \"\t\n#=\^\!\|\(\)\[\]\{\}:;]+	{ yylval.sval=stash(yytext);return(STRING); }
\"[^\"]*\"			{ yylval.sval=stash(dequote(yytext));
				  return(STRING); }
\n				{ return(NL); }
.				{ return(*yytext); }

%%

/*
 * Remove the quotes enclosing a quoted string.
 */
char *
dequote(s)
char *s;
{
    char *p;

    ++s;
    p = rindex(s, '"');
    *p = '\0';
    return(s);
}

/*
 * Save string away in memory and return a pointer to it.  If
 * no space could be found, quit with a fatal error.
 */
char *stash(s)
char *s;
{
    char *ptr;

    if (!s || !strlen(s))
	return(NULL);
    ptr = malloc(strlen(s) + 1);
    if (ptr == NULL) {
        Error("out of space");
    }
    return(strcpy(ptr, s));
}
