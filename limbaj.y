%{
#include <iostream>
#include <vector>
#include "SymTable.h"
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
     ASTNode ast;
     char* string;
}

%token BGIN END ASSIGN NR BGINGLOBAL ENDGLOBAL BGINVARS ENDVARS BGINCLASS ENDCLASS BGINFUNC ENDFUNC CLASS
%token EQ NEQ GT LT GTE LTE AND OR NOT
%token PRINT TYPEOF EVAL IF ELSE WHILE FOR DO
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

decl_funcs     : decl_func
               | decl_funcs decl_func
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

list_array     :    list_array ',' NR
               |    NR

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

primaexpfor :
            | e    
            | decl_var
            ;
adouaexpfor :
            | e     
            ;
atreiaexprfor :
              | e     
              ;


statement: ';'
         | decl_var
         | decl_func
         | e ';'
         | PRINT '(' e ')' ';'
         | RETURN e ';'
         | IF '(' e ')' '{' statement '}'      
         | IF '(' e ')' '{' statement '}' ELSE '{' statement '}'   
         | WHILE '(' e ')' '{' statement '}'    
         | DO '{'statement '}' WHILE '(' e ')' ';' 
         | FOR '(' primaexpfor ';' adouaexpfor ';' atreiaexprfor ')' '{' statement '}'
         | ID '(' list ')'
         | ID '('')'
         | ID ASSIGN e
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