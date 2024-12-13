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
%token  BGIN END ASSIGN NR BGINCLASS ENDCLASS BGINGLOBAL ENDGLOBAL BGINFUNC ENDFUNC 
%token EQ NEQ GT LT GTE LTE AND OR NOT
%token PRINT TYPEOF EVAL IF ELSE WHILE
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

decl_class :   CLASS ID  '{' membs_list '}' ';'

membs_list : memb_list
           | membs_list memb_list
           ;

memb_list : ID ':' TYPE ';'
          | altfeldacavreiasadacorect
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

statement: ID '(' call_list ')'
         | ID '('')'
         | declarations
         | 


          
         | ID ASSIGN e
         | ID ASSIGN ID
         | ID ASSIGN INT
         | ID ASSIGN BOOL
         | ID ASSIGN CHAR
         | ID ASSIGN STRING
         | ID ASSIGN FLOAT
         ;
        
call_list :  call_list ',' e
           | e
           ;

e : e '+' e  
  | e '*' e  
  |'(' e ')'
  | ID 
  | NR
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