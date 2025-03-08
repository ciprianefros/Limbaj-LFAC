%{
#include <stdio.h>
#include "src/limbaj.tab.h"
%}
%option noyywrap
%%

"intreg" { return INT;}
"real" { return FLOAT;}
"caracter" { return CHAR;}
"sir" { return STRING;}
"bool" { return BOOL;}

"clasa" {return CLASS;}

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
"daca" {return IF;}
"altfel" {return ELSE;}
"pentru" {return FOR;}
"lucreaza" {return DO;}
"catTimp" {return WHILE;}
"bucla" {return LOOP;}
"rupe" {return BREAK;}
"continua" {return CONTINUE;}
"returneaza" {return RETURN;}
"adevarat"  { yylval.true_or_false = true; return BVAL; }
"fals" { yylval.true_or_false = false; return BVAL; }
\"([a-zA-Z0-9 .,!?;:_\t\n\r\\\"']|\\.)*\" { yylval.string = strdup(yytext); return SVAL; }
['][0-9a-zA-Z]['] { yylval.caracter = yytext[1]; return CVAL; }
[0].[0-9]+[0-9]*|[1-9]+[0-9]*[.][0-9]+[0-9]* {    yylval.real_number = atof(yytext);return FVAL;}
[_a-zA-Z][a-zA-Z0-9_]* {yylval.string = strdup(yytext); return ID;}
"<-" {return ASSIGN;}
0|[1-9]+[0-9]* {yylval.number = atoi(yytext); return IVAL;}
\/\*([^*]|\*+[^\/])*\*+\/ { /* Ignorăm comentariile */ }
[ \t] ;
\n {yylineno++;}
. {return yytext[0];}

