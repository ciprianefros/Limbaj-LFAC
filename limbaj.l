%{
#include <stdio.h>
#include "limbaj.tab.h"
%}
%option noyywrap
%%
"intreg"|"real"|"caracter"|"sir"|"bool"|"clasa" {yylval.string = strdup(yytext);return TYPE;}
"start_programel" {return BGIN;}
"sfarsit_programel" {return END;}
[a-zA-Z][a-zA-Z0-9]* {yylval.string = strdup(yytext); return ID;}
"<-" {return ASSIGN;}
[0-9]+ {return NR;}
[ \t] ;
\n {yylineno++;}
. {return yytext[0];}

