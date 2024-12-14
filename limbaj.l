%{
#include <stdio.h>
#include "limbaj.tab.h"
%}
%option noyywrap
%%
"intreg"|"real"|"caracter"|"sir"|"bool" {yylval.string = strdup(yytext);return TYPE;}

"clasa" {return CLASS;}
"constant" {return CONST;}
"start_programel" {return BGIN;}
"sfarsit_programel" {return END;}

"start_global" {return BGINGLOBAL;}
"sfarsit_global" {return ENDGLOBAL;}
"start_vars" {return BGINVARS;}
"sfarsit_vars" {return ENDVARS;}
"start_clasa" {return BGINCLASS;}
"sfarsit_clasa" {return ENDCLASS;}
"start_functions" {return BGINFUNC;}
"sfarsit_functions" {return ENDFUNC;}

"=="         { return EQ; }
"!="         { return NEQ; }
">"          { return GT; }
"<"          { return LT; }
">="         { return GTE; }
"<="         { return LTE; }
"&&"         { return AND; }
"||"         { return OR; }
"!"          { return NOT; }
"Print" {return PRINT;}
"TypeOf" {return TYPEOF;}
"Eval" {return EVAL;}
"daca" {return IF;}
"altfel" {return ELSE;}
"catTimp" {return WHILE;}
"adevarat"  { yylval.true_or_false = true; return BOOL; }
"fals" { yylval.true_or_false = false; return BOOL; }
\"[0-9a-zA-Z ]*\" { yylval.string = strdup(yytext); return STRING; }
['][0-9a-zA-Z]['] { yylval.caracter = yytext[1]; return CHAR; }
[1-9]+[0-9]*[.][0-9]+[0-9]* {    yylval.real_number = atof(yytext);return FLOAT;}
[0].[0-9]+[0-9]* {yylval.real_number = atof(yytext); return FLOAT;}
[a-zA-Z][a-zA-Z0-9]* {yylval.string = strdup(yytext); return ID;}
"<-" {return ASSIGN;}
[1-9]+[0-9]* {yylval.number = atoi(yytext); return INT;}
0 {yylval.number = atoi(yytext); return INT;}
[ \t] ;
\n {yylineno++;}
. {return yytext[0];}

