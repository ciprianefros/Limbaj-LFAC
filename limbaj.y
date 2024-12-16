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
%left '*' '/' '%'
%left '^' 
%left NOT
%left '[' ']' '(' ')'

%%
/*Programelul nostru*/
progr                     :    global_declarations main { 
                                   if (errorCount == 0) 
                                       std::cout << "The program is correct!" << std::endl; 
                               }
                          ;

/*Global scope*/
global_declarations       :    BGINGLOBAL declarations ENDGLOBAL
                          |    
                          ;

/*Partea de global scope ( variabile, clase, functii) */
declarations              :    vars_declarations class_declarations func_declarations
                          ;

/*Sectiunea de declarare a variabilelor globale*/
vars_declarations         :    BGINVARS decl_vars ENDVARS    
                          |    
                          ;

/*Declararea de variabile recursiv*/
decl_vars                 :    decl_var ';'
                          |    decl_vars decl_var ';'
                          ;


/*Declararea de variabile sau array-uri*/
decl_var                  :    TYPE ID  {
                                   if (!current->existsId($2)) {
                                       current->addVar($1, $2);
                                   } else {
                                       errorCount++; 
                                       yyerror("Variable already defined");
                                   }
                               }
                          |    TYPE ID '[' list_array ']' {
                                   if (!current->existsId($2)) {
                                       current->addVar($1, $2);
                                   } else {
                                       errorCount++; 
                                       yyerror("Variable already defined");
                                   }
                               }
                          ;
/*Partea in program de declarare a claselor*/
class_declarations        :    BGINCLASS decl_classes ENDCLASS
                          |    
                          ;
/*Declarearea a oricator clase*/
decl_classes              :    decl_class
                          |    decl_classes decl_class
                          ;

/*Declararea unei clase*/
decl_class                :    CLASS ID '{' membs_list methods_list '}' ';'
                          ;

/*Lista de membri a unei clase*/
membs_list                :    class_memb
                          |    membs_list class_memb
                          ;

/*Permite definirea oricator metode la o clasa*/
methods_list              :    methods_list method
                          |    
                          ;

/*Definirea unei metode intr-o clasa*/
method                    :    TYPE ID '(' list_param ')' '{' { 
                                   /* create function symtable, update current */ 
                               } fblock '}' {
                                   /* update current pointer to match the new scope */
                                   /* if ID does not exist in current scope, add function info to the current symtable */
                               }
                          ;
/*Definirea membrilor unei clase: A.x*/
class_memb                :    ID ':' TYPE ';'
                          ;

func_declarations         :    BGINFUNC decl_funcs ENDFUNC
                          |    
                          ;
/*Declararea de functii*/
decl_funcs                :    def_func
                          |    decl_funcs def_func
                          ;

/*Definirea unei functii*/
def_func                  :    TYPE ID '(' list_param ')' '{' {
                                   /* create function symtable, update current */ 
                               } fblock '}' {
                                   /* update current pointer to match the new scope */
                                   /* if ID does not exist in current scope, add function info to the current symtable */
                               }
                          |    TYPE ID '(' list_param ')' ';'
                          ;
/*Lista de parametri pentru functii*/
list_param                :    param
                          |    list_param ',' param
                          |    
                          ;                      
/*Definirea unui array: uni, multi dimensional*/
list_array                :    list_array ',' INT
                          |    INT
                          ;
/*Blocul unei functii*/
fblock                    :     fblock decl_var ';'
                          |     fblock statement
                          |     
                          ;

/*Parametrii pentru functii*/
param                     :     TYPE ID
                          ;
/*main function*/
main                      :     BGIN list END
                          ;
/*Expresiile acceptate in main, pentru a fi recursive!*/
list                      :     statement ';'
                          |     list statement ';'
                          ;
/*Tot felul de expresii din interiorul programului*/
statement                 :     call_expr
                          |     PRINT '(' arithm_expr ')'
                          |     TYPEOF '(' ID ')'
                          |     ID ID
                          |     ID ID '{' init_instante '}'
                          |     IF '(' bool_expr ')' '{' list '}'
                          |     IF '(' bool_expr ')' '{' list '}' ELSE '{' list '}'
                          |     WHILE '(' bool_expr ')' '{' list '}'
                          |     DO '{' list '}' WHILE '(' bool_expr ')'
                          |     LOOP '{' list '}'
                          |     FOR '(' assignment_stmt ';' bool_expr ';' assignment_stmt ')' '{' list '}'
                          |     assignment_stmt
                          |     CONTINUE
                          |     BREAK
                          ;
/*Asignari ale membrilor unei clase*/
init_instante             :     ID ASSIGN arithm_expr ';'
                          |     init_instante ID ASSIGN arithm_expr ';'
                          ;
/*Expresii de asignare pentru variabile, clase si array-uri*/
assignment_stmt           :     TYPE ID ASSIGN arithm_expr
                          |     ID ASSIGN arithm_expr
                          |     ID '[' list_array ']' ASSIGN arithm_expr
                          |     ID '[' list_array ']' ASSIGN call_expr
                          |     ID '.' ID ASSIGN arithm_expr
                          ;

/*Apeluri de functii*/
call_expr                 :     ID '(' call_list ')'
                          |     ID '(' ')'
                          ;

/*Parametrii de apel al unei functii*/
call_list                 :     call_list ',' arithm_expr
                          |     call_list ',' ID '(' call_list ')'
                          |     arithm_expr
                          |     ID '(' call_list ')'
                          ;

/*Expresii boolene (true sau false)*/
bool_expr                 :     '(' bool_expr NEQ bool_expr ')'
                          |     bool_expr NEQ bool_expr
                          |     '(' bool_expr EQ bool_expr ')'
                          |     bool_expr EQ bool_expr
                          |     '(' bool_expr LT bool_expr ')'
                          |     bool_expr LT bool_expr
                          |     '(' bool_expr LTE bool_expr ')' 
                          |     bool_expr LTE bool_expr 
                          |     '(' bool_expr GT bool_expr ')'
                          |     bool_expr GT bool_expr 
                          |     '(' bool_expr GTE bool_expr ')'
                          |     bool_expr GTE bool_expr  
                          |     '(' bool_expr AND bool_expr ')'
                          |     bool_expr AND bool_expr
                          |     '(' bool_expr OR bool_expr ')'
                          |     bool_expr OR bool_expr
                          |     '(' NOT bool_expr ')'
                          |     NOT bool_expr
                          |     arithm_expr
                          ;

/*Expresii aritmetice*/
arithm_expr               :     arithm_expr '+' arithm_expr  
                          |     arithm_expr '*' arithm_expr  
                          |     arithm_expr '/' arithm_expr
                          |     arithm_expr '-' arithm_expr
                          |     arithm_expr '%' arithm_expr
                          |     '(' arithm_expr ')'
                          |     '-' arithm_expr
                          |     ID
                          |     INT
                          |     FLOAT
                          |     CHAR
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