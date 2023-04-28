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
 * 001 -- Loretta Guarino Reid, DEC Ultrix Engineering Group
 *  Western Software Lab. Convert to X11.
 * 002 -- Jordan Hubbard, U.C. Berkeley. New keywords. Menu
 * changes, gadget boxes, title bars, the kitchen sink.
 * 1.6 -- Memory leaks fixed.
 */

%{




#ifndef lint
static char *rcsid_gram_y = "$Header: /usr/graph2/X11.3/contrib/windowmgrs/awm/RCS/gram.y,v 1.2 89/02/07 21:25:05 jkh Exp $";
#endif	lint

#include <stdlib.h>
#include "X11/Xlib.h"
#include "X11/Xutil.h"
#include "awm.h"
#include <signal.h>

/*
 * Values returned by complex expression parser.
 */
#define C_STRING	1	/* IsString. */
#define C_MENU		2	/* IsMenu. */
#define C_MAP		3	/* IsMap. */
#define C_MENUMAP	4	/* IsMenuMap */
#define C_PIXMAP	5	/* IsPixmap */
#define C_ACTION	6	/* IsAction */
 
#define GADGET_TOKEN "gadget"

     static int ki;			/* Keyword index. */
     static int gadgnum;		/* # of gadget we're initing */
     static int g_offset;		/* The gadget offset specified */
     static int g_gravity;		/* The gadget gravity specified */
     static char *g_forecolor;		/* gadget foreground color */
     static char *g_backcolor;		/* gadget background color */
     static XFontStruct *g_font;	/* gadget font */
     static int bkmask;			/* Button/key mask. */
     static int cmask;			/* Context mask. */
     static char msg[BUFSIZ];		/* Error message buffer. */
     static char *menu_name;		/* Menu name. */
     static char *menu_pixmap;		/* Name of pixmap for menu label */
     static MenuInfo *menu_info;	/* Menu info. */
     static ActionLine *ml_ptr;		/* Temporary menu line pointer. */
     static ActionLine *action;		/* Temporary action pointer */
     MenuLink *menu_link;		/* Temporary menu link pointer. */
     GadgetDecl **Gadgets;		/* Pointer to gadget info structs */

     %}

%union {
     char *sval;
     int ival;
     short shval;
     struct _actionline *alval;
     struct _menuinfo *mival;
     char **cval;
}

%token NL
     %token <sval> STRING
     %token <ival> COMMENT
     %type <sval> pixmap_file
     %type <ival> gadget_subscript
     %type <ival> keyword
     %type <ival> compexpr
     %type <ival> keyexpr
     %type <ival> kmask
     %type <ival> contexpr
     %type <ival> contmask
     %type <ival> buttmodexpr
     %type <ival> buttmodifier
     %type <ival> buttexpr
     %type <sval> menuname
     %type <sval> strings
     %type <alval> textaction
     %type <alval> menuexpr
     %type <alval> menulist
     %type <alval> menuline
     %type <alval> menuaction

     %%	/* beginning of rules section */
     
 input:	|	input command
     |	input error command { yyerrok; }
;

 command:	boolvar term
     |	expr term
     |	COMMENT	{ Lineno++; }
     |	term
     ;

 term:		NL	{ Lineno++; }
|	';'
     ;

 expr:		keyword '=' compexpr
{
     switch (KeywordTable[$1].type) {
     case IsQuitFunction:
     case IsFunction:
	  if ($3 == C_MAP) {
	       bindtofunc($1, bkmask, cmask, NULL);
	  } else yyerror("illegal construct");
	  break;

     case IsDownFunction:
	  if (bkmask & ButtonUp) {
	       sprintf(msg,
		       "cannot bind %s to button up",
		       KeywordTable[$1].name);
	       yyerror(msg);
	  }
	  if ($3 == C_MAP) {
	       bindtofunc($1, bkmask, cmask, NULL);
	  } else yyerror("illegal construct");
	  break;

     case IsMenuMap:
	  if (bkmask & ButtonUp) {
	       sprintf(msg,
		       "cannot bind %s to button up",
		       KeywordTable[$1].name);
	       yyerror(msg);
	  }
	  if ($3 == C_MENUMAP) {
	       bindtofunc
		    ($1, bkmask, cmask, menu_name);
	  } else yyerror("illegal construct");
	  break;

     case IsAction:
	  if ($3 == C_ACTION) {
	       /*
		* We pass a structure pointer here where a char pointer
		* is supposed to go. It was a hack, what can I say.
		*/
	       bindtofunc
		    ($1, bkmask, cmask, (char *)action);
	  } else yyerror("illegal construct");
	  break;

     case IsMenu:
	  if ($3 == C_MENU) {
	       /*
		* create a menu definition entry.
		*/
	       menu_info = stashmenuinfo(menu_name, ml_ptr, menu_pixmap);
	       menu_link = stashmenulink(menu_info);
	       Menus = appendmenulink(Menus, menu_link);
	  } else yyerror("illegal menu construct");
	  break;

     case IsGadget:
	  if (gadgnum < 0) {
	       sprintf(msg, "Gadget number must be >= 0\n");
	       yyerror(msg);
	  }
	  /* Bump NumGadgets if necessary */
	  else if (gadgnum >= NumGadgets)
	       NumGadgets = gadgnum + 1;
	  if (NumGadgets > MAX_GADGETS) {
	       sprintf(msg, "\"numgadgets\" (%d) is > MAX_GADGETS (%d)\n",
		       NumGadgets, MAX_GADGETS);
	       yyerror(msg);
	  }
	  else {
	       if ($3 != C_STRING && $3 != C_PIXMAP)
		    yyerror("Illegal gadget assignment");
	       else
		    stashGadget(gadgnum, yylval.sval, $3);
	  }
	  break;

     default:
	  yyerror("internal binding error");
	  break;
     }
}
;
 pixmap_file:	'(' STRING ')' {
      $$ = $2;
 }
;
 compexpr:	keyexpr ':' contexpr ':' buttexpr
{
     $$ = C_MAP;
     bkmask = $1 | $5;
     cmask = $3;
}
|	keyexpr ':' contexpr ':' buttexpr ':' menuname
{
     $$ = C_MENUMAP;
     bkmask = $1 | $5;
     cmask = $3;
     menu_name = $7;
}
|	keyexpr ':' contexpr ':' buttexpr ':' textaction
{
     $$ = C_ACTION;
     bkmask = $1 | $5;
     cmask = $3;
     action = $7;
}
|	STRING menuexpr
{
     $$ = C_MENU;
     menu_name = $1;
     menu_pixmap = 0;
     ml_ptr = $2;
}
|	pixmap_file STRING menuexpr
{
     $$ = C_MENU;
     menu_name = $2;
     menu_pixmap = $1;
     ml_ptr = $3;
}
|	STRING '^' gadgetspec
{ yylval.sval = $1; $$ = C_STRING; }
|	pixmap_file '^' gadgetspec
{ yylval.sval = $1; $$ = C_PIXMAP; }
|	STRING
{
     $$ = C_STRING;
     /* just in case it's a gadget, set defaults */
     g_forecolor = Foreground;
     g_backcolor = Background;
     g_gravity = NoGadgetGravity;
     g_offset = 0;
     g_font = 0;
}
|	pixmap_file
{
     $$ = C_PIXMAP;
     /* just in case it's a gadget, set defaults */
     g_forecolor = Foreground;
     g_backcolor = Background;
     g_gravity = NoGadgetGravity;
     g_offset = 0;
     g_font = 0;
}     
;
 gadgetspec:	 offset
|  offset '|' gravity
|  offset '|' gravity '|' forecolor
|  offset '|' gravity '|' forecolor '|' backcolor
|  offset '|' gravity '|' forecolor '|' backcolor '|' fontspec
;
 offset: /* empty */
{ g_offset = 0; }
| STRING
{
	g_offset = y_atoi($1);
	g_gravity = NoGadgetGravity;
	g_forecolor = Foreground;
	g_backcolor = Background;
	g_font = (XFontStruct *)NULL;
}
;
 gravity: /* empty */
{ g_gravity = NoGadgetGravity; }
| STRING
{
	g_gravity = gravitylookup($1);
	g_forecolor = Foreground;
	g_backcolor = Background;
	g_font = (XFontStruct *)NULL;
}
;
 forecolor: /* empty */
{ g_forecolor = Foreground; }
| STRING
{
	g_forecolor = $1;
	g_backcolor = Background;
	g_font = (XFontStruct *)NULL;
}
;
 backcolor: /* empty */
{ g_backcolor = Background; }
| STRING
{
	g_backcolor = $1;
	g_font = (XFontStruct *)NULL;
}
;
 fontspec: /* empty */
{ g_font = 0; }
| STRING
{
     g_font = XLoadQueryFont(dpy, $1);
     if (!g_font) {
	  sprintf(msg, "Can't open gadget font '%s'\n", $1);
	  yywarn(msg);
     }
}
;

 boolvar:	STRING
{
     ki = keywordlookup(yylval.sval);
     switch (KeywordTable[ki].type) {
     case IsParser:
	  (*KeywordTable[ki].fptr)();
	  break;
     default:
	  yyerror("keyword error");
     }
}
;

 keyword:	STRING	{
      $$ = keywordlookup(yylval.sval);
 }
|	STRING gadget_subscript {
     char *ptr;

     ptr = malloc(strlen(GADGET_TOKEN) + 1);
     if (ptr == NULL) {
       Error("out of space");
     }
     strcpy(ptr, GADGET_TOKEN);
     $$ = keywordlookup(ptr);
     gadgnum = $2;
}
;

 gadget_subscript:	'[' STRING ']' {
      $$ = y_atoi(yylval.sval);
 }
;

 keyexpr:	/* empty */
{ $$ = 0; }
|	kmask
{ $$ = $1; }
|	kmask '|' keyexpr
{ $$ = $1 | $3; }
;

 contexpr:	/* empty */
{ $$ = 0xffffffff; }
|	contmask
{ $$ = $1; }
|	contmask '|' contexpr
{ $$ = $1 | $3; }
;

 buttexpr:	buttmodexpr
{ $$ = CheckButtonState($1); }
;

 kmask:		STRING { $$ = keyexprlookup(yylval.sval); }

 contmask:	STRING
{ $$ = contexprlookup(yylval.sval); }
|	STRING gadget_subscript
{ 
     if ($2 < 0 || $2 >= NumGadgets) {
	  sprintf(msg, "Bad subscript, gadget #%d must be >= 0 and < %d\n",
		  $2, NumGadgets);
	  yyerror(msg);
     }
     else {
         char *ptr;

	 ptr = malloc(strlen(GADGET_TOKEN) + 1);
	 if (ptr == NULL) {
	   Error("out of space");
	 }
	 strcpy(ptr, GADGET_TOKEN);
	  $$ = contexprlookup(ptr) | (1 << (BITS_USED + $2));
     }
}
;
 buttmodexpr: 	buttmodifier
{ $$ = $1; }
|	buttmodexpr buttmodifier
{ $$ = $1 | $2; }
;

 buttmodifier:	STRING
{ $$ = buttexprlookup(yylval.sval); }
;

 menuname:	STRING
{ $$ = $1; }
;

 menuexpr:	'{' menulist '}'
{ $$ = $2; }
;

 menulist:	menuline
{ $$ = $1; }
|	menulist menuline
{ $$ = appendmenuline($1, $2); }
|	menulist COMMENT
{
     Lineno++;
     $$ = $1;
}
|	COMMENT
{
     Lineno++;
     $$ = NULL;
}
|	term
{ $$ = NULL; }
|	menulist term
{ $$ = $1; }
|	error term
{
     $$ = NULL;
     yyerrok;
}
;

 menuline:	strings ':' menuaction term
{
     $3->name = $1;
     $3->pixmapname = (char *)0;
     $$ = $3;
}
|	'(' strings ')' ':' menuaction term
{
     $5->pixmapname = $2;
     $5->name = $2;
     $$ = $5;
}
;

 menuaction:	STRING
{
     ki = keywordlookup(yylval.sval);
     if ((ki != -1) &&
	 (KeywordTable[ki].type != IsFunction) &&
	 (KeywordTable[ki].type != IsImmFunction) &&
	 (KeywordTable[ki].type != IsQuitFunction) &&
	 (KeywordTable[ki].type != IsBoolean) &&
	 (KeywordTable[ki].type != IsDownFunction)) {
	  sprintf(msg,
		  "menu action \"%s\" not a function or variable",
		  KeywordTable[ki].name);
	  yyerror(msg);
     }
     ml_ptr = AllocActionLine();
     if (KeywordTable[ki].type == IsQuitFunction ||
	KeywordTable[ki].type == IsImmFunction)
	  ml_ptr->type = IsImmFunction;
     else if (KeywordTable[ki].type == IsBoolean) {
	  ml_ptr->type = IsVar;
	  ml_ptr->text = (char *)KeywordTable[ki].bptr;
     }
     else
	  ml_ptr->type = IsUwmFunction;
     ml_ptr->func = KeywordTable[ki].fptr;
     $$ = ml_ptr;
}
|	STRING ':' menuname
{
     ki = keywordlookup($1);
     if (ki != -1 &&
	 KeywordTable[ki].type != IsMenuMap) {
	  sprintf(msg,
		  "menu action \"%s\" not a menu function",
		  KeywordTable[ki].name);
	  yyerror(msg);
     }
     ml_ptr = AllocActionLine();
     ml_ptr->type = IsMenuFunction;
     ml_ptr->text = $3;
     $$ = ml_ptr;
}
|	textaction
{ $$ = $1; }
;

 textaction: '!' strings
{
     $$ = StashActionLine(IsShellCommand, $2);
}
|	'^' strings
{
     $$ = StashActionLine(IsTextNL, $2);
}
|	'|' strings
{
     $$ = StashActionLine(IsText, $2);
}
;

 strings:	STRING	{ $$ = yylval.sval; }
|	strings STRING
{ $$ = strconcat($1, $2); }
;

%%

/*
 * Look up color named by "string" and return pixel value.
 */
Pixel LookupColor(string, cmap, fail)
char *string;
Colormap cmap;
Boolean *fail;
{
     XColor vis_ret;

     Entry("LookupColor")

     if (!(string && XParseColor(dpy, cmap, string, &vis_ret) &&
	XAllocColor(dpy, cmap, &vis_ret))) {
	 sprintf(msg, "Can't allocate color '%s', using default\n", string);
	 yywarn(msg);
	 if (fail)
	      *fail = TRUE;
	 Leave((Pixel)0)
     }
     if (fail)
	  *fail = FALSE;
     Leave(vis_ret.pixel)
}

/*
 * Like LookupColor, but provides its own fallback in case of failure
 * (currently 0).
 */
Pixel GetPixel(string, cmap)
char *string;
Colormap cmap;
{
     XColor vis_ret;

     Entry("GetPixel")

     if (!string)
	 Leave((Pixel)0)
     if (!(string && XParseColor(dpy, cmap, string, &vis_ret) &&
	XAllocColor(dpy, cmap, &vis_ret)))
	 Leave((Pixel)0)
     Leave(vis_ret.pixel)
}

/*
 * Look up a string in the keyword table and return its index, else
 * return -1.
 */
int keywordlookup(string)
char *string;
{
     int i;
     
     Entry("keywordlookup")

     for (i = 0; KeywordTable[i].name; i++) {
          if (!strcmp(KeywordTable[i].name, string)) {
	       free(string);
	       Leave(i)
	  }
     }
     sprintf(msg,"keyword error: \"%s\"", string);
     yyerror(msg);
     free(string);
     Leave(-1)
}

/*
 * Look up a string in the key expression table and return its mask, else
 * return -1.
 */
int keyexprlookup(string)
char *string;
{
     int i;
     
     Entry("keyexprlookup")

     for (i = 0; KeyExprTbl[i].name; i++) {
	  if (!strcmp(KeyExprTbl[i].name, string)) {
	       free(string);
	       Leave(KeyExprTbl[i].mask)
	  }
     }
     sprintf(msg,"key expression error: \"%s\"", string);
     yyerror(msg);
     free(string);
     Leave(-1)
}

int gravitylookup(string)
char *string;
{
     int i;

     Entry("gravitylookup")

     for (i = 0; GravityExprTbl[i].name; i++) {
	  if (!strcmp(GravityExprTbl[i].name, string)) {
	       free(string);
	       Leave(GravityExprTbl[i].mask)
	  }
     }
     sprintf(msg, "gravity expression error: \"%s\"", string);
     yyerror(msg);
     free(string);
     Leave(-1);
}
    
/*
 * Look up a string in the context expression table and return its mask, else
 * return -1.
 */
contexprlookup(string)
char *string;
{
     int i;
     
     Entry("contexprlookup")

     for (i = 0; ContExprTbl[i].name; i++) {
	  if (!strcmp(ContExprTbl[i].name, string)) {
	       free(string);
	       Leave(ContExprTbl[i].mask)
	  }
     }
     sprintf(msg,"context expression error: \"%s\"", string);
     yyerror(msg);
     free(string);
     Leave(-1)
}

/*
 * Look up a string in the button expression table and return its mask, else
 * return -1.
 */
buttexprlookup(string)
char *string;
{
     int i;
     
     Entry("buttexprlookup")

     for (i = 0; ButtModTbl[i].name; i++) {
	  if (!strcmp(ButtModTbl[i].name, string)) {
	       free(string);
	       Leave(ButtModTbl[i].mask)
	  }
     }
     sprintf(msg,"button modifier error: \"%s\"", string);
     yyerror(msg);
     free(string);
     Leave(-1)
}

/*
 * Scan a string and return an integer.  Report an error if any
 * non-numeric characters are found.
 */
y_atoi(s)
char *s;
{
     int n = 0;
     
     Entry("y_atoi")

     while (*s) {
	  if (*s >= '0' && *s <= '9')
	       n = 10 * n + *s - '0';
	  else {
	       yyerror("non-numeric argument");
	       Leave(-1)
	  }
	  s++;
     }
     Leave(n)
}

/*
 * Append s2 to s1, extending s1 as necessary.
 */
char *
     strconcat(s1, s2)
char *s1, *s2;
{
     char *p;
     
     Entry("strconcat")

     p = malloc(strlen(s1) + strlen(s2) + 2);
     sprintf(p, "%s %s", s1, s2);
     free(s1);
     free(s2);
     s1 = p;
     Leave(s1)
}

/*
 * Check a button expression for errors.
 */
int CheckButtonState(expr)
int expr;
{
     Entry("CheckButtonState")

     /*
      * Check for one (and only one) button.
      */
     switch (expr & (LeftMask | MiddleMask | RightMask)) {
     case 0:
	  yyerror("no button specified");
	  break;
     case LeftMask:
	  break;
     case MiddleMask:
	  break;
     case RightMask:
	  break;
     default:
	  yyerror("more than one button specified");
     }
     
     /*
      * Check for one (and only one) up/down/motion modifier.
      */
     switch (expr & (ButtonUp | ButtonDown | DeltaMotion)) {
     case 0:
	  yyerror("no button action specified");
	  break;
     case ButtonUp:
	  break;
     case ButtonDown:
	  break;
     case DeltaMotion:
	  break;
     default:
	  yyerror("only one of up/down/motion may be specified");
     }
     Leave(expr)
}

/*
 * Bind button/key/context to a function.
 */
bindtofunc(index, mask, context, name)
int index;		/* Index into keyword table. */
int mask;		/* Button/key/modifier mask. */
int context;		/* ROOT, WINDOW, TITLE, ICON, GADGET or BORDER */
char *name;		/* Menu, if needed. */
{
     Entry("bindtofunc")

     setbinding(context, index, mask, name);
     Leave_void
}

/*
 * Allocate a Binding type and return a pointer.
 */
Binding *
     AllocBinding()
{
     Binding *ptr;
     
     Entry("AllocBinding")

     if (!(ptr = (Binding *)calloc(1, sizeof(Binding)))) {
	  sprintf(msg, "Can't allocate binding--out of space\n");
	  yyerror(msg);
	  exit(1);
     }
     Leave(ptr)
}

/*
 * Stash the data in a Binding.
 */
setbinding(cont, i, m, mname)
int cont;		/* Context: ROOT, WINDOW, or ICON. */
int i;			/* Keyword table index. */
int m;		/* Key/button/modifier mask. */
char *mname;		/* Pointer to menu name, if needed. */
{
     Binding *ptr;

     Entry("setbinding")

     ptr = AllocBinding();
     ptr->context = cont;
     ptr->mask = m;
     ptr->func = KeywordTable[i].fptr;
     ptr->menuname = mname;
     switch (m & (LeftMask | MiddleMask | RightMask)) {
     case LeftMask:
	  ptr->button = LeftButton;
	  break;
     case MiddleMask:
	  ptr->button = MiddleButton;
	  break;
     case RightMask:
	  ptr->button = RightButton;
	  break;
     }
     appendbinding(ptr);
     Leave_void
}

/*
 * Append a Binding to the Bindings list.
 */
appendbinding(binding)
Binding *binding;
{
     Binding *ptr;
     
     Entry("appendbinding")

     if (Blist == NULL)
	  Blist = binding;
     else {
	  for(ptr = Blist; ptr->next; ptr = ptr->next) /* NULL */;
	  ptr->next = binding;
	  ptr = ptr->next;
	  ptr->next = NULL;
     }
     Leave_void
}

/*
 * Allocate an action line and return a pointer.
 */
ActionLine *AllocActionLine()
{
     ActionLine *ptr;
     
     Entry("AllocActionLine")

     if (!(ptr = (ActionLine *)calloc(1, sizeof(ActionLine)))) {
	  sprintf(msg, "Can't allocate action line--out of space\n");
	  yyerror(msg);
     }
     Leave(ptr)
}

/*
 * Allocate a MenuInfo structure and return a pointer.
 */
MenuInfo *AllocMenuInfo()
{
     MenuInfo *ptr;
     
     Entry("AllocMenuInfo")

     if (!(ptr = (MenuInfo *)calloc(1, sizeof(MenuInfo)))) {
	  sprintf(msg, "Can't allocate menu storage--out of space\n");
	  yyerror(msg);
     }
     Leave(ptr)
}

/*
 * Allocate a MenuLink structure and return a pointer.
 */
MenuLink *AllocMenuLink()
{
     MenuLink *ptr;
 
     Entry("AllocMenuLink")

     if (!(ptr = (MenuLink *)calloc(1, sizeof(MenuLink)))) {
	  sprintf(msg, "Can't allocate menu linked list storage--out of space\n");
	  yyerror(msg);
     }
     Leave(ptr)
}

/*
 * Return storage for Gadgets[] array.
 */
GadgetDecl **allocate_gadgets()
{
     GadgetDecl **tmp;
     int i;

     Entry("allocate_gadgets")

     tmp = (GadgetDecl **)malloc(MAX_GADGETS * sizeof(GadgetDecl *));
     if (!tmp) {
	  sprintf(msg, "Can't allocate storage for Gadgets -- out of space\n");
	  yyerror(msg);
	  Leave(NULL)
     }
     for (i = 0; i < MAX_GADGETS; i++)
	  tmp[i] = (GadgetDecl *)0;
     Leave(tmp)
}

/*
 * Stash a gadget record
 */
GadgetDecl *stashGadget(n, s, type)
int n;
char *s;
int type;
{
     GadgetDecl *tmp;

     Entry("stashGadget")

     if (!Gadgets)
	  Gadgets = (GadgetDecl **)allocate_gadgets();
     if (n < 0 || n >= NumGadgets) {
	  sprintf(msg, "stashGadget on gadget #%d when maxgadget = %d\n",
		  n, NumGadgets);
	  yyerror(msg);
	  Leave(NULL)
     }
     if (Gadgets[n]) {
	  sprintf(msg, "gadget #%d redefined\n", n);
	  yywarn(msg);
	  FreeGadget(n);
          tmp = Gadgets[n];
     }
     else
	  Gadgets[n] = tmp = (GadgetDecl *)malloc(sizeof(GadgetDecl));
     if (!Gadgets[n]) {
	  sprintf(msg, "Can't allocate new gadget, out of space!\n");
	  yyerror(msg);
	  Leave(NULL)
     }
     tmp->data = (unsigned char *)NULL;
     tmp->name = (unsigned char *)NULL;
     tmp->high = tmp->wide = 0;
     tmp->gravity = g_gravity;
     tmp->offset = g_offset;
     tmp->forecolor = g_forecolor;
     tmp->backcolor = g_backcolor;
     tmp->fontInfo = g_font;
     if (type != C_STRING && type != C_PIXMAP) {
	  sprintf(msg, "Invalid gadget specification for gadget #%d\n", n);
	  yyerror(msg);
	  NumGadgets = 0;
          Leave(NULL)
     }
     if (type == C_PIXMAP) {
	  int junk;
	  char *nm = s;

	  s = expand_from_path(s);
	  if (!s) {
	       sprintf(msg, "Can't find pixmap file '%s' for gadget #%d\n",
		       nm, n);
	       yywarn(msg);
	       tmp->data = (unsigned char *)gray_bits;
	       tmp->high = gray_height;
	       tmp->wide = gray_width;
	  }
	  else if (XmuReadBitmapDataFromFile (s, &(tmp->wide),
				  &(tmp->high), &(tmp->data), &junk, &junk)
	      != BitmapSuccess) {
	       sprintf(msg, "Can't open pixmap file '%s' for gadget #%d.\n", s, n);
	       yyerror(msg);
	       tmp->data = (unsigned char *)gray_bits;
	       tmp->high = gray_height;
	       tmp->wide = gray_width;
	  }
	  if (tmp->high > gadgetHeight)
	       gadgetHeight = tmp->high;
     }
     else if (type == C_STRING) { /* it's a label */
	  tmp->name = expand_metachars(s);
	  if (!tmp->fontInfo) {
	       if (!GFontInfo) {
		    GFontInfo = GetFontRes("gadget.font", DEF_GADGET_FONT);
		    if (!GFontInfo) {
			 sprintf(msg, "Can't get a default gadget font.\n");
			 yyerror(msg);
			 Leave(NULL)
		    }
	       }
	       tmp->fontInfo = GFontInfo;
	  }
	  if (strlen(tmp->name) > 1) {
	       tmp->wide = XTextWidth(tmp->fontInfo, tmp->name, strlen(tmp->name));
	       tmp->high = tmp->fontInfo->max_bounds.ascent +
		    tmp->fontInfo->max_bounds.descent + 2;
	  }
	  else {
	       XCharStruct chinfo;
	       int asc, desc, dir;

	       XTextExtents(tmp->fontInfo, tmp->name, 1, &dir, &asc,
			    &desc, &chinfo);
	       tmp->wide = chinfo.width;
	       tmp->high = chinfo.ascent + chinfo.descent;
	  }
	  tmp->wide += 2 * GadgetBorder;
          if (tmp->high > gadgetHeight)
	       gadgetHeight = tmp->high;
     }
     Leave(tmp)
}

/*
 * This routine expands '\' notation in a string, ala C. Mostly useful for
 * imbedding weird characters in strings that turn into interesting symbols
 * from some font. Unlike C, however, numeric constants (\nnn) are in
 * decimal, not octal. This was done because the most popular glyphs
 * (in cursorfont.h) are identified in decimal. There are some other cute
 * "metacharacters" that expand to the window name, icon name, etc.
 */
unsigned char *expand_metachars(s)
register unsigned char *s;
{
     register int i, len, val, n;
     unsigned char *cp, num[5];

     Entry("expand_metachars")

     if (!s)
	  Leave(s)
     len = strlen(s);
     for (i = 0; i < len; i++)
	  if (s[i] == '\\')
	       break;
     if (i == len)
	  Leave(s)
     /* we know the string is going to get shorter, len is correct */
     cp = (unsigned char *)malloc(len);
     i = n = 0;
     while (*s) {
	  if (*s == '\\') {
	       s++;
	       while (*s && *s >= '0' && *s <= '9') {
		    num[n++] = *s;
		    s++;
	       }
	       if (n) {
		    if (n > 4)
			 n = 4;
		    num[n] = '\0';
		    cp[i++] = (char)atoi(num);
		    n = 0;
	       }
	       else if (*s) {
		    switch(*s) {

		    case 'b':
			 val = 8;
			 break;

		    case 'f':
			 val = 12;

		    case 'n':
			 val = 10;
			 break;

		    case 'r':
			 val = 13;
			 break;

		    case 't':
			 val = 9;
			 break;

		    default:
			 val = *s;
			 break;
		    }
		    cp[i++] = val;
		    s++;
	       }
	  }
	  else {
	       cp[i++] = *s;
	       s++;
	  }
     }
     cp[i] = '\0';
     Leave(cp)
}
	  
/*
 * Stash the data in an action line.
 */
ActionLine *StashActionLine(type, string)
int type;
char *string;
{
     ActionLine *ptr;
     
     Entry("StashActionLine")

     ptr = AllocActionLine();
     ptr->type = type;
     ptr->text = string;
     Leave(ptr)
}

/*
 * Stash menu data in a MenuInfo structure;
 */
MenuInfo *stashmenuinfo(name, line, pixmap)
char *name;
ActionLine *line;
char *pixmap;
{
     MenuInfo *ptr;
     
     Entry("stashmenuinfo")

     ptr = AllocMenuInfo();
     ptr->name = name;
     ptr->line = line;
     ptr->pixmapname = pixmap;
     ptr->menu = 0;
     Leave(ptr)
}

/*
 * Stash menu info data in a MenuLink structure;
 */
MenuLink *stashmenulink(menuinfo)
MenuInfo *menuinfo;
{
     MenuLink *ptr;
     
     Entry("stashmenulink")

     ptr = AllocMenuLink();
     ptr->next = NULL;
     ptr->menu = menuinfo;
     Leave(ptr)
}

/*
 * Append an action line to a linked list of menu lines.
 */
ActionLine *appendmenuline(list, line)
ActionLine *list;
ActionLine *line;
{
     ActionLine *ptr;
     
     Entry("appendmenuline")

     if (list == NULL)
	  list = line;
     else {
	  for(ptr = list; ptr->next; ptr = ptr->next) /* NULL */;
	  ptr->next = line;
	  ptr = ptr->next;
	  ptr->next = NULL;
     }
     Leave(list)
}

/*
 * Append a menu to a linked list of menus.
 */
MenuLink *
     appendmenulink(list, link)
MenuLink *list;
MenuLink *link;
{
     MenuLink *ptr;
     
     Entry("appendmenulink")

     if (list == NULL)
	  list = link;
     else {
	  for(ptr = list; ptr->next; ptr = ptr->next) /* NULL */;
	  ptr->next = link;
	  ptr = ptr->next;
	  ptr->next = NULL;
     }
     Leave(list)
}
     
/*
 * Reset all previous bindings and free the space allocated to them.
 */
Boolean ResetBindings()
{
     Binding *ptr, *nextptr;
     
     Entry("ResetBindings")

     for(ptr = Blist; ptr; ptr = nextptr) {
	  nextptr = ptr->next;
	  free(ptr);
     }
     Blist = NULL;
     Leave_void
}

/*
 * De-allocate all menus.
 */
Boolean ResetMenus()
{
     MenuLink *mptr, *next_mptr;
     register ActionLine *lptr, *next_lptr;
 
     Entry("ResetMenus")
     if (!Menus)
	Leave_void
     for(mptr = Menus; mptr; mptr = next_mptr) {
	  free(mptr->menu->name);
	  RTLMenu_Destroy(mptr->menu->menu);
	  for(lptr = mptr->menu->line; lptr; lptr = next_lptr) {
	       free(lptr->name);
	       if (lptr->text) free(lptr->text);
	       next_lptr = lptr->next;
	       free(lptr);
	  }
	  next_mptr = mptr->next;
	  free(mptr);
     }
     Menus = NULL;
     Leave_void
}
