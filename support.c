


#ifndef lint
static char *rcsid_support_c = "$Header: /usr/graph2/X11.3/contrib/windowmgrs/awm/RCS/support.c,v 1.2 89/02/07 21:25:47 jkh Exp $";
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
 * REVISION HISTORY:
 *
 * 1.2 -- Version of strlen() that accepts NULL strings added (#ifdef it
 * in if your system needs it).
 */

#include "support.h"
#include <stdio.h>
     
int _rtn_level;
int _rtn_trace;

static struct func_stack {
     char *rtn_name;
     struct func_stack *next;
} *Head;

char *curr_rtn()
{
     if (!Head)
	  return((char *)0);
     else
	  return(Head->rtn_name);
}

void push_rtn(s)
register char *s;
{
     if (!Head) {
	  Head = (struct func_stack *)malloc(sizeof(struct func_stack));
	  if (!Head) {
	       fprintf(stderr, "Couldn't malloc new func_stack entry!\n");
	       exit(1);
	  }
	  Head->rtn_name = s;
	  Head->next = 0;
     }
     else {
	  struct func_stack *ptr;
	  
	  ptr =  (struct func_stack *)malloc(sizeof(struct func_stack));
	  if (!ptr) {
	       fprintf(stderr, "Couldn't malloc new func_stack entry!\n");
	       exit(1);
	  }
	  ptr->rtn_name = s;
	  ptr->next = Head;
	  Head = ptr;
     }
     _rtn_level++;
     if (_rtn_trace) {
	  int i;
	  
	  for (i = 0; i < _rtn_level; i++)
	       putchar('\t');
	  printf("%s(%d)\n", Head->rtn_name, _rtn_level);
     }
}

void pop_rtn()
{
     struct func_stack *ptr;

     if (!Head)
	  return;
     ptr = Head;
     Head = Head->next;
     free(ptr);
     _rtn_level--;
}

/*
 * Some systems retch when you pass strlen() a NULL pointer.
 * Here's a way of getting around it.
 */

#if defined(hlh) || defined(titan)
strlen(s)
char *s;
{
	int count = 0;

	if (s == 0)
		return(0);
	while (*s++)
		count++;
	return(count);
}
#endif /* hlh */
