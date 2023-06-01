/* Companion source code for "flex & bison", published by O'Reilly
 * Media, ISBN 978-0-596-15597-1
 * Copyright (c) 2009, Taughannock Networks. All rights reserved.
 * See the README file for license conditions and contact info.
 * $Header: /home/johnl/flnb/code/RCS/fb3-2.l,v 2.1 2009/11/08 02:53:18 johnl Exp $
 */
/* recognize tokens for the calculator */

%option noyywrap nodefault yylineno
%{
# include "fb3-2.h"
# include "fb3-2.tab.h"

extern int lines;
extern int chars;
extern int words;
int flexVal;
char *buf[100];

%}

/* float exponent */
EXP	([Ee][-+]?[0-9]+)

%%
.			{ return yytext[0]; }
\n			{ return yytext[0]; }
"printf"	{ return PRINTF; }
"scanf"		{ return SCANF; }
%%

