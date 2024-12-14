%{
#include <iostream>
#include <vector>
#include "SymTable.h"
#include "AST.h"
extern FILE* yyin;
extern char* yytext;
extern int yylineno;
extern int yylex();
void yyerror(const char * s);
class SymTable* current;
vector<SymTable*> tables;

int errorCount = 0;
%}
%union 
{
     int number;
     float real_number;
     bool true_or_false;
     char caracter;
     char* string;
}
%token BGIN END ASSIGN NR BGINGLOBAL ENDGLOBAL BGINVARS ENDVARS BGINCLASS ENDCLASS BGINFUNC ENDFUNC CLASS CONST
%token EQ NEQ GT LT GTE LTE AND OR NOT
%token PRINT TYPEOF EVAL IF ELSE WHILE FOR DO LOOP BREAK CONTINUE
%token<string> ID TYPE STRING CHAR
%token<real_number> FLOAT
%token<number> INT
%token<true_or_false> BOOL
%start progr

%left OR
%left AND
%left EQ NEQ
%left GT LT LTE GTE
%left '+' '-' 
%left '*' '/' 
%left '^' 
%left NOT
%left '[' ']' '(' ')'

%%
progr :  global_declarations main {if (errorCount == 0) cout<< "The program is correct!" << endl;}
      ;

global_declarations   :    BGINGLOBAL declarations ENDGLOBAL
                      |    
                      ;

declarations   : vars_declarations class_declarations func_declarations

vars_declarations   : BGINVARS decl_vars ENDVARS    
                    |
                    ;

decl_vars      : decl_var
               | decl_vars decl_var
               ;

class_declarations  : BGINCLASS decl_classes ENDCLASS
                    | 
                    ;

decl_classes   : decl_class
               | decl_classes decl_class
               ;

func_declarations   : BGINFUNC decl_funcs ENDFUNC
                    |
                    ;

decl_funcs     : def_func
               | decl_funcs def_func
               ;


decl_var  :    TYPE ID ';'    { 
                                   if(!current->existsId($2)) 
                                   {
                                             current->addVar($1,$2);
                                   } 
                                   else 
                                   {
                                        errorCount++; 
                                        yyerror("Variable already defined");
                                   }
                              }
          |    TYPE ID '[' list_array ']' ';'     {
                                                       if(!current->existsId($2)) 
                                                       {
                                                            current->addVar($1,$2);
                                                       } 
                                                       else 
                                                       {
                                                            errorCount++; 
                                                            yyerror("Variable already defined");
                                                       } 
                                                  }    
           ;

decl_class :   CLASS ID  '{' membs_list methods_list '}' ';'

membs_list : class_memb
           | membs_list class_memb
           ;

methods_list : methods_list method
             |
             ;

method : TYPE ID '(' list_param ')' '{'{/*create function symtable,update current*/}  fblock '}'
                                   {
                                        /*update current pointer to match the new scope*/
                                        /*if ID does not exist in current scope, add function info to the current symtable */
                                   }
       ;

class_memb : ID ':' TYPE ';' 
           ;

def_func : TYPE ID '(' list_param ')' '{'{/*create function symtable,update current*/}  fblock '}'
                                   {
                                        /*update current pointer to match the new scope*/
                                        /*if ID does not exist in current scope, add function info to the current symtable */
                                   }
          | TYPE ID  '(' list_param ')' ';'

list_array     :    list_array ',' INT
               |    INT

fblock : fblock decl_var
       | fblock statement
       |
       ;

     
list_param : param
            | list_param ','  param 
            |
            ;
            
param : TYPE ID 
      ; 

main : BGIN list END  
     ;
     
list :  statement ';' 
     | list statement ';'
     ;

statement: ID '(' call_list ')'
         | ID '(' ')'
         | TYPE ID ASSIGN e
         | ID ASSIGN e
         | ID '[' INT ']' ASSIGN e
         | ID '.' ID ASSIGN e
         | IF '(' bool_expr ')' '{' list '}' 
         | IF '(' bool_expr ')' '{' list '}' ELSE '{' list '}'
         | WHILE '(' bool_expr ')' '{' list '}'
         | DO '{' list '}' WHILE '(' bool_expr ')'
         | LOOP '{' list '}'
         | FOR '(' assignment_stmt ';' bool_expr ';' assignment_stmt ')' '{' list '}'
         | CONTINUE
         | BREAK 
         ;

assignment_stmt : TYPE ID ASSIGN e
                | ID ASSIGN e
                ;

bool_expr : bool_expr NEQ bool_expr 
          | bool_expr EQ bool_expr
          | bool_expr LT bool_expr
          | bool_expr LTE bool_expr 
          | bool_expr GT bool_expr 
          | bool_expr GTE bool_expr 
          | bool_expr AND bool_expr
          | bool_expr OR bool_expr
          | NOT bool_expr
          | e
          ;

call_list :  call_list ',' e
          | e
          ;

e : e '+' e  
  | e '*' e  
  | e '/' e
  | e '-' e
  |'(' e ')'
  | '-' e
  | ID
  | INT
  | FLOAT
  | CHAR
  ;
%%
void yyerror(const char * s){
     cout << "error:" << s << " at line: " << yylineno << endl;
}

int main(int argc, char** argv){
     yyin=fopen(argv[1],"r");
     current = new SymTable("global");
     yyparse();
     cout << "Variables:" <<endl;
     current->printVars();
     delete current;
}