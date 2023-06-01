/* Companion source code for "flex & bison", published by O'Reilly
 * Media, ISBN 978-0-596-15597-1
 * Copyright (c) 2009, Taughannock Networks. All rights reserved.
 * See the README file for license conditions and contact info.
 * $Header: /home/johnl/flnb/code/RCS/fb3-2funcs.c,v 2.1 2009/11/08 02:53:18 johnl Exp $
 */
/*
 * helper functions for fb3-2
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "fb3-2.h"
#include "fb3-2.tab.h"

//The Max Value of Stack 
#define MAX 100
#define NHASH 9997
struct symbol symtab[NHASH];

//global stack array & top index
int stack[MAX];
int top;

extern int flexVal;
extern int yyparse (void);
/* symbol table */
int lines = 0;
int chars = 0;
int words = 0;

int totchars = 0;
int totwords = 0;
int totlines = 0;

//initialize stack
void init_stack(void)
{
	top = -1;
}

//push stack
int push(int t)
{
	if(top >= MAX-1)
	{
		printf("Stack Overflow.\n");
		exit(1);
	}
	
	stack[++top] = t;
	
	return t;
}


//pop stack
int pop(void)
{
	if(top < 0)
	{
		printf("Stack UnderFlow.\n");
		exit(1);
	}
	
	return stack[top--];
}

//Get a top of stack val 
int get_stack_top(void)
{
	return (top < 0) ? -1 : stack[top];
}

//Check Empty state in Stack 
int is_stack_empty(void)
{
	return (top < 0);
}


//Check Argument Whether is operator
int is_operator(int k)
{
	int retVal = 0;
	
	switch(k)
	{
		case '+':
			retVal = '+';
			break;
		case '-':
			retVal = '-';
			break;
		case '*':
			retVal = '*';
			break;
		case '/':
			retVal = '/';
			break;
		default:
			break;
	}
	
	return retVal;
}

//
int is_legal(char *s)
{
	int f = 0;
	
	while(*s)
	{
		while(*s == ' ')
			s++;
		if(is_operator(*s))
			f--;
		else
		{
			f++;
			
			while(*s != ' ')
				s++;
		}
		
		if(f < 1) 
			break;
		
		s++;
	}
	
	if(f == 1)
		return 1;
	else
		return f;
}


int precedence(int op)
{
	if(op == '(')
		return 0;
	
	if(op == '+'||op == '-')
		return 1;
	
	if(op == '*'||op == '/')
		return 2;
	else
		return 3;
}


void postfix(char *dst, char *src)
{
	char c;
	init_stack();
	
	while(*src)
	{
		if(*src == '(')
		{
			push(*src);
			src++;
		}
		else if(*src == ')')
		{
			while(get_stack_top() != '(')
			{
				*dst++ = pop();
				*dst++ = ' ';
			}
			
			pop();
			src++;
		}
		else if(is_operator(*src))
		{
			while(!is_stack_empty() && precedence(get_stack_top()) >= precedence(*src))
			{
				*dst++ = pop();
				*dst++ = ' ';
			}
			
			push(*src);
			src++;
		}
		else if(*src >= '0' && *src <= '9')
		{
			do{
				*dst++ = *src++;
			}while(*src >= '0' && *src <= '9');
			
			*dst++ = ' ';
		}
		else
		{
			src++;
		}
	}
	
	while(!is_stack_empty())
	{
		*dst++ = pop();
		*dst++ = ' ';
	}
	
	dst--;
	*dst = 0;
}


int calc(char *p)
{
	int i;
	init_stack();
	
	while(*p)
	{
		if(*p >= '0' && *p <= '9')
		{
			i=0;
			
			do{
				i=i*10+*p-'0';
				p++;
			}while(*p >= '0' && *p <= '9');
			
		push(i);
		}
		else if(*p == '+')
		{
			push(pop() + pop());
			p++;
		}
		else if(*p == '*')
		{
			push(pop()*pop());
			p++;
		}
		else if(*p == '-')
		{
			i = pop();
			push(pop() - i);
			p++;
		}
		else if(*p == '/')
		{
			i = pop();
			push(pop()/i);
			p++;
		}
		else
		{
			p++;
		}
	}
	
	return pop();
}
		

		

/* hash a symbol */
static unsigned
symhash(char *sym)
{
  unsigned int hash = 0;
  unsigned c;

  while(c = *sym++) hash = hash*9 ^ c;

  return hash;
}

struct symbol *lookup(char* sym)
{
  struct symbol *sp = &symtab[symhash(sym)%NHASH];
  int scount = NHASH;		/* how many have we looked at */

  while(--scount >= 0) {
    if(sp->name && !strcmp(sp->name, sym)) { return sp; }

    if(!sp->name) {		/* new entry */
      sp->name = strdup(sym);
      sp->value = 0;
      sp->func = NULL;
      sp->syms = NULL;
      return sp;
    }

    if(++sp >= symtab+NHASH) sp = symtab; /* try the next entry */
  }
  yyerror("symbol table overflow\n");
  abort(); /* tried them all, table is full */

}



struct ast *newast(int nodetype, struct ast *l, struct ast *r)
{
  struct ast *a = malloc(sizeof(struct ast));
  
  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = nodetype;
  a->l = l;
  a->r = r;
  return a;
}

struct ast *newnum(double d)
{
  struct numval *a = malloc(sizeof(struct numval));
  
  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = 'K';
  a->number = d;
  return (struct ast *)a;
}

struct ast *newcmp(int cmptype, struct ast *l, struct ast *r)
{
  struct ast *a = malloc(sizeof(struct ast));
  
  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = '0' + cmptype;
  a->l = l;
  a->r = r;
  return a;
}

struct ast *newfunc(int functype, struct ast *l)
{
  struct fncall *a = malloc(sizeof(struct fncall));
  
  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = 'F';
  a->l = l;
  a->functype = functype;
  return (struct ast *)a;
}

struct ast *newcall(struct symbol *s, struct ast *l)
{
  struct ufncall *a = malloc(sizeof(struct ufncall));
  
  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = 'C';
  a->l = l;
  a->s = s;
  return (struct ast *)a;
}

struct ast *newref(struct symbol *s)
{
  struct symref *a = malloc(sizeof(struct symref));
  
  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = 'N';
  a->s = s;
  return (struct ast *)a;
}

struct ast *newasgn(struct symbol *s, struct ast *v)
{
  struct symasgn *a = malloc(sizeof(struct symasgn));
  
  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = '=';
  a->s = s;
  a->v = v;
  return (struct ast *)a;
}

struct ast *newflow(int nodetype, struct ast *cond, struct ast *tl, struct ast *el)
{
  struct flow *a = malloc(sizeof(struct flow));
  
  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = nodetype;
  a->cond = cond;
  a->tl = tl;
  a->el = el;
  return (struct ast *)a;
}

struct symlist *newsymlist(struct symbol *sym, struct symlist *next)
{
  struct symlist *sl = malloc(sizeof(struct symlist));
  
  if(!sl) {
    yyerror("out of space");
    exit(0);
  }
  sl->sym = sym;
  sl->next = next;
  return sl;
}

void symlistfree(struct symlist *sl)
{
  struct symlist *nsl;

  while(sl) {
    nsl = sl->next;
    free(sl);
    sl = nsl;
  }
}

/* define a function */
void dodef(struct symbol *name, struct symlist *syms, struct ast *func)
{
  if(name->syms) symlistfree(name->syms);
  if(name->func) treefree(name->func);
  name->syms = syms;
  name->func = func;
}

static double callbuiltin(struct fncall *);
static double calluser(struct ufncall *);

double eval(struct ast *a)
{
  double v;

  if(!a) {
    yyerror("internal error, null eval");
    return 0.0;
  }

  switch(a->nodetype) {
    /* constant */
  case 'K': v = ((struct numval *)a)->number; break;

    /* name reference */
  case 'N': v = ((struct symref *)a)->s->value; break;

    /* assignment */
  case '=': v = ((struct symasgn *)a)->s->value =
      eval(((struct symasgn *)a)->v); break;

    /* expressions */
  case '+': v = eval(a->l) + eval(a->r); break;
  case '-': v = eval(a->l) - eval(a->r); break;
  case '*': v = eval(a->l) * eval(a->r); break;
  case '/': v = eval(a->l) / eval(a->r); break;
  case '|': v = fabs(eval(a->l)); break;
  case 'M': v = -eval(a->l); break;

    /* comparisons */
  case '1': v = (eval(a->l) > eval(a->r))? 1 : 0; break;
  case '2': v = (eval(a->l) < eval(a->r))? 1 : 0; break;
  case '3': v = (eval(a->l) != eval(a->r))? 1 : 0; break;
  case '4': v = (eval(a->l) == eval(a->r))? 1 : 0; break;
  case '5': v = (eval(a->l) >= eval(a->r))? 1 : 0; break;
  case '6': v = (eval(a->l) <= eval(a->r))? 1 : 0; break;

  /* control flow */
  /* null if/else/do expressions allowed in the grammar, so check for them */
  case 'I': 
    if( eval( ((struct flow *)a)->cond) != 0) {
      if( ((struct flow *)a)->tl) {
	v = eval( ((struct flow *)a)->tl);
      } else
	v = 0.0;		/* a default value */
    } else {
      if( ((struct flow *)a)->el) {
        v = eval(((struct flow *)a)->el);
      } else
	v = 0.0;		/* a default value */
    }
    break;

  case 'W':
    v = 0.0;		/* a default value */
    
    if( ((struct flow *)a)->tl) {
      while( eval(((struct flow *)a)->cond) != 0)
	v = eval(((struct flow *)a)->tl);
    }
    break;			/* last value is value */
	              
  case 'L': eval(a->l); v = eval(a->r); break;

  case 'F': v = callbuiltin((struct fncall *)a); break;

  case 'C': v = calluser((struct ufncall *)a); break;

  default: printf("internal error: bad node %c\n", a->nodetype);
  }
  return v;
}

static double callbuiltin(struct fncall *f)
{
  enum bifs functype = f->functype;
  double v = eval(f->l->l);
  double x = eval(f->l->r);

 switch(functype) {
 case B_sqrt:
   return sqrt(v);
 case B_exp:
   return exp(v);
 case B_log:
   return log(v);
 case B_print:
   printf("= %4.4g\n", v);
   return v;
 case B_log10:
   return log10(v);
 case B_sin:
   return sin(v);
 case B_cos:
   return cos(v);
 case B_tan:
   return tan(v);
 case B_asin:
   return asin(v);
 case B_acos:
   return acos(v);
 case B_atan:
   return atan(v);
 case B_sinh:
   return sinh(v);
 case B_cosh:
   return cosh(v);
 case B_tanh:
   return tanh(v);
 case B_ceil:
   return ceil(v);
 case B_floor:
   return floor(v);
 case B_pow:
   return pow(v, x);
 case B_fmod:
   return fmod(v, x);
 default:
   yyerror("Unknown built-in function %d", functype);
   return 0.0;
 }
}

static double calluser(struct ufncall *f)
{
  struct symbol *fn = f->s;	/* function name */
  struct symlist *sl;		/* dummy arguments */
  struct ast *args = f->l;	/* actual arguments */
  double *oldval, *newval;	/* saved arg values */
  double v;
  int nargs;
  int i;

  if(!fn->func) {
    yyerror("call to undefined function", fn->name);
    return 0;
  }

  /* count the arguments */
  sl = fn->syms;
  for(nargs = 0; sl; sl = sl->next)
    nargs++;

  /* prepare to save them */
  oldval = (double *)malloc(nargs * sizeof(double));
  newval = (double *)malloc(nargs * sizeof(double));
  if(!oldval || !newval) {
    yyerror("Out of space in %s", fn->name); return 0.0;
  }
  
  /* evaluate the arguments */
  for(i = 0; i < nargs; i++) {
    if(!args) {
      yyerror("too few args in call to %s", fn->name);
      free(oldval); free(newval);
      return 0;
    }

    if(args->nodetype == 'L') {	/* if this is a list node */
      newval[i] = eval(args->l);
      args = args->r;
    } else {			/* if it's the end of the list */
      newval[i] = eval(args);
      args = NULL;
    }
  }
		     
  /* save old values of dummies, assign new ones */
  sl = fn->syms;
  for(i = 0; i < nargs; i++) {
    struct symbol *s = sl->sym;

    oldval[i] = s->value;
    s->value = newval[i];
    sl = sl->next;
  }

  free(newval);

  /* evaluate the function */
  v = eval(fn->func);

  /* put the dummies back */
  sl = fn->syms;
  for(i = 0; i < nargs; i++) {
    struct symbol *s = sl->sym;

    s->value = oldval[i];
    sl = sl->next;
  }

  free(oldval);
  return v;
}


void treefree(struct ast *a)
{
  switch(a->nodetype) {

    /* two subtrees */
  case '+':
  case '-':
  case '*':
  case '/':
  case '1':  case '2':  case '3':  case '4':  case '5':  case '6':
  case 'L':
    treefree(a->r);

    /* one subtree */
  case '|':
  case 'M': case 'C': case 'F':
    treefree(a->l);

    /* no subtree */
  case 'K': case 'N':
    break;

  case '=':
    free( ((struct symasgn *)a)->v);
    break;

  case 'I': case 'W':
    free( ((struct flow *)a)->cond);
    if( ((struct flow *)a)->tl) free( ((struct flow *)a)->tl);
    if( ((struct flow *)a)->el) free( ((struct flow *)a)->el);
    break;

  default: printf("internal error: free bad node %c\n", a->nodetype);
  }	  
  
  free(a); /* always free the node itself */

}

void yyerror(char *s, ...)
{
  va_list ap;
  va_start(ap, s);

  fprintf(stderr, "%d: error: ", yylineno);
  vfprintf(stderr, s, ap);
  fprintf(stderr, "\n");
}

int main(int argc, char **argv)
{

	int tok;
	int result=0;
	char* sbuf[2000]={0};
	char* dbuf[2000]={0};



	if(argc < 2) {
	yylex();
	printf("%8d%8d%8d\n", lines, words, chars);
	return 0;
	}

    FILE *f = fopen(argv[1], "r");
  
    if(!f) {
      perror(argv[1]);
      return (1);
    }
	
	
    yyrestart(f);
	
	while(tok = yylex()) 
	{
		if(tok == PRINTF)
			printf("= %d", result);
		else if(tok == SCANF)
		{
			scanf("%s", sbuf);
			postfix(dbuf, sbuf);
			result = calc(dbuf);
		}
	
	}

    fclose(f);

 // printf("> "); 
 // return yyparse();
}

/* debugging: dump out an AST */
int debug = 0;
void dumpast(struct ast *a, int level)
{

  printf("%*s", 2*level, "");	/* indent to this level */
  level++;

  if(!a) {
    printf("NULL\n");
    return;
  }

  switch(a->nodetype) {
    /* constant */
  case 'K': printf("number %4.4g\n", ((struct numval *)a)->number); break;

    /* name reference */
  case 'N': printf("ref %s\n", ((struct symref *)a)->s->name); break;

    /* assignment */
  case '=': printf("= %s\n", ((struct symref *)a)->s->name);
    dumpast( ((struct symasgn *)a)->v, level); return;

    /* expressions */
  case '+': case '-': case '*': case '/': case 'L':
  case '1': case '2': case '3':
  case '4': case '5': case '6': 
    printf("binop %c\n", a->nodetype);
    dumpast(a->l, level);
    dumpast(a->r, level);
    return;

  case '|': case 'M': 
    printf("unop %c\n", a->nodetype);
    dumpast(a->l, level);
    return;

  case 'I': case 'W':
    printf("flow %c\n", a->nodetype);
    dumpast( ((struct flow *)a)->cond, level);
    if( ((struct flow *)a)->tl)
      dumpast( ((struct flow *)a)->tl, level);
    if( ((struct flow *)a)->el)
      dumpast( ((struct flow *)a)->el, level);
    return;
	              
  case 'F':
    printf("builtin %d\n", ((struct fncall *)a)->functype);
    dumpast(a->l, level);
    return;

  case 'C': printf("call %s\n", ((struct ufncall *)a)->s->name);
    dumpast(a->l, level);
    return;

  default: printf("bad %c\n", a->nodetype);
    return;
  }
}
