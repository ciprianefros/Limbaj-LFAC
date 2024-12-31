%{
#include <iostream>
#include <vector>
#include "utils.h"
extern FILE* yyin;
extern char* yytext;
extern int yylineno;
extern int yylex();

void yyerror(const char * s);

class SymTable* current;
vector<SymTable*> tables;
SymTable* globalTable = new SymTable("global");
SymTable* currentTable = globalTable;


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

%token BGIN END ASSIGN NR BGINGLOBAL ENDGLOBAL BGINVARS ENDVARS BGINCLASS ENDCLASS BGINFUNC ENDFUNC CLASS
%token EQ NEQ GT LT GTE LTE AND OR NOT
%token PRINT TYPEOF EVAL IF ELSE WHILE FOR DO LOOP BREAK CONTINUE RETURN
%token<string> ID SVAL
%token CHAR STRING FLOAT INT BOOL
%token<real_number> FVAL
%token<number> IVAL
%token<true_or_false> BVAL
%token<caracter> CVAL
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
global_declarations       :     BGINGLOBAL   {
                                    current = globalTable;
                                    tables.push_back(globalTable);
                                }
                                declarations ENDGLOBAL
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

TYPE : INT 
     {
          currentVariable.type.typeName = TYPE_INT;
          currentFunction.returnType = TYPE_INT;
     }
     | FLOAT 
     {
          currentVariable.type.typeName = TYPE_FLOAT;
          currentFunction.returnType = TYPE_FLOAT;
     }
     | STRING 
     {
          currentVariable.type.typeName = TYPE_STRING;
          currentFunction.returnType = TYPE_STRING;
     }
     | BOOL 
     {
          currentVariable.type.typeName = TYPE_BOOL;
          currentFunction.returnType = TYPE_BOOL;
     }
     | CHAR  
     {
          currentVariable.type.typeName = TYPE_CHAR;
          currentFunction.returnType = TYPE_CHAR;
     }
     ;

/*Declararea de variabile sau array-uri*/
decl_var                  :    TYPE ID  {
                                   if (!currentTable->existsId($2)) {
                                        currentVariable.name = $2;
                                        currentTable->addVar(currentVariable.type.typeName, $2);
                                   } else {
                                       errorCount++; 
                                       yyerror(("Variable already defined at line: " + std::to_string(yylineno)).c_str());
                                   }
                               }
                          |    TYPE ID '[' list_array ']' {
                                   if (!currentTable->existsId($2)) {
                                       currentTable->addVar(currentVariable.type.typeName, $2);
                                   } else {
                                       errorCount++; 
                                       yyerror(("Variable already defined at line: " + std::to_string(yylineno)).c_str());
                                   }
                               }
                          | assignment_stmt
                          ;

init_list                 : init_list ',' expr
                          | expr 
                          ;

/*Partea in program de declarare a claselor*/
class_declarations        :    BGINCLASS decl_classes ENDCLASS
                          |    
                          ;
/*Declararea a oricator clase*/
decl_classes              :    decl_class
                          |    decl_classes decl_class
                          ;

/*Declararea unei clase*/
decl_class
    : CLASS ID '{' {
        // Creează tabelă pentru clasa curentă
        if (currentTable->existsId($2)) { // Verifică dacă clasa există deja
            errorCount++;
            yyerror(("Class already defined at line: " + std::to_string(yylineno)).c_str());
        } else {
            currentTable->addClass($2);
            currentTable = new SymTable($2, currentTable);  // Creează un nou tabel de simboluri pentru clasă
            tables.push_back(currentTable); // Adaugă în lista globală de tabele
        }
    } membs_list methods_list '}' ';' {
        // Revenire la scopul părinte
        currentTable = currentTable->prev;
    }
    ;

/*Lista de membri a unei clase*/
membs_list                :    class_memb
                          |    membs_list class_memb
                          ;

/*Definirea membrilor unei clase: A.x*/
class_memb                :    ID ':' TYPE ';'
                                {
                                   if (!currentTable->existsId($1)) {
                                       currentTable->addVar(currentVariable.type.typeName, $1);
                                   } else {
                                       errorCount++; 
                                       yyerror(("Variable already defined at line: " + std::to_string(yylineno)).c_str());
                                   }
                               }

/*Permite definirea oricator metode la o clasa*/
methods_list              :    methods_list method
                          |    
                          ;

/*Definirea unei metode într-o clasă*/
method
    : TYPE ID '(' list_param ')' '{'  {
        // Verifică dacă funcția este deja definită
        if (currentTable->existsFunc($2)) 
        { 
            errorCount++;
            yyerror(("Function " + std::string($2) + " already defined at line: " + std::to_string(yylineno)).c_str());
        } else {
            // Adaugă funcția în tabela simbolurilor a clasei
            currentTable->addFunc(currentFunction.returnType, $2, currentParams);
            currentTable = new SymTable($2, currentTable);  // Creează un nou tabel de simboluri pentru clasă
            tables.push_back(currentTable);
        }
    } 
    fblock '}'
    {
        // Revenire la tabelul simbolurilor al clasei
        currentTable = currentTable->prev;
        //current = current->prev;
        //stergerea parametrilor
        currentParams.clear();
    }
    ;

/*Partea de definire a functiilor*/
func_declarations         :    BGINFUNC decl_funcs ENDFUNC
                          |    
                          ;

/*Declararea de functii*/
decl_funcs                :    def_func
                          |    decl_funcs def_func
                          ;

/*Definirea unei functii*/
def_func  : TYPE ID '(' list_param ')'
                    {   
                        if(!currentTable->existsFunc($2)) {
                            currentTable->addFunc(currentFunction.returnType, $2, currentParams);
                        }
                        currentTable = new SymTable($2, currentTable); // Crează un tabel de simboluri pentru funcția curentă
                        tables.push_back(currentTable);  // Adaugă tabelul global
                        currentFunction.name = $2; 
                    }  
                    function_definition
                    {
                        currentParams.clear();
                        currentTable = currentTable->prev; // Revenire la scopul părinte
                    }
                    | TYPE ID '(' list_param ')' ';' {
                        if(!currentTable->existsFunc($2)) {
                            currentTable->addFunc(currentFunction.returnType, $2, currentParams);
                        }
                        currentTable = new SymTable($2, currentTable); // Crează un tabel de simboluri pentru funcția curentă
                        tables.push_back(currentTable);  // Adaugă tabelul global

                    }
                    {
                        currentParams.clear();
                        currentTable = currentTable->prev; // Revenire la scopul părinte
                    }
                    ;

function_definition : '{' fblock '}' 
                    ;


list_param 
    : param
    | list_param ',' param
    |
    ;


/*Parametrii pentru functii*/
param                     :     TYPE ID  
                                {
                                    currentVariable.name = $2;
                                    currentVariable.type.isArray = false;

                                    currentParams.push_back(currentVariable);
                                    
                                }
                          |     TYPE ID '[' list_array ']' 
                                {
                                    currentVariable.name = $2;
                                    currentVariable.type.isArray = true;

                                    currentParams.push_back(currentVariable);
                                }
                          ;


/*Definirea unui array: uni, multi dimensional*/
list_array                :    list_array ',' IVAL
                          |    IVAL
                          ;

/*Blocul unei functii*/
fblock
    :  fblock statement 
    | 
    ;

/*main function*/
main
    : BGIN { 
        // Creare tabelă pentru scopul main
        currentTable = new SymTable("main", currentTable);
        tables.push_back(currentTable);
    } list END {
        // Revenire la scopul global
        currentTable = currentTable->prev;
    }
    ;

/*Expresiile acceptate in main, pentru a fi recursive!*/
list                      :     statement 
                          |     list statement 
                          ;

/*Tot felul de expresii din interiorul programului*/
statement
    : call_func ';'
    | decl_var ';'
    | ID ID ';'
    | ID ID '{' init_instante '}' ';'
    | IF '(' bool_expr ')' '{' list '}' 
    {
    
    currentTable = new SymTable("if", currentTable);
    tables.push_back(currentTable);
    } 
    {  
    // Revenire la scopul părinte
    currentTable = currentTable->prev;
    }
    | IF '(' bool_expr ')' '{' list '}' ELSE '{' list '}'
    {
        // Creează o tabelă pentru scopul IF
        currentTable = new SymTable("if", currentTable);
        tables.push_back(currentTable);
    }
    {
        // Revenire la scopul părinte
        currentTable = currentTable->prev;
    }
    | WHILE 
    {
        // Creează o tabelă pentru scopul WHILE
        currentTable = new SymTable("while", currentTable);
        tables.push_back(currentTable);
    }
    '(' bool_expr ')' '{' list '}'   
    {
        // Revenire la scopul părinte
        currentTable = currentTable->prev;
    }
    | DO 
    {
        // Creează o tabelă pentru scopul WHILE
        currentTable = new SymTable("do-while", currentTable);
        tables.push_back(currentTable);
    } 
    '{' list '}' WHILE '(' bool_expr ')' 
    {
        // Revenire la scopul părinte
        currentTable = currentTable->prev;
    }
    | LOOP 
    {
        // Creează o tabelă pentru scopul LOOP
        currentTable = new SymTable("loop", currentTable);
        tables.push_back(currentTable);
    }
    '{' list '}' 
    {
        // Revenire la scopul părinte
        currentTable = currentTable->prev;
    }
    | FOR 
    {
        // Creează o tabelă pentru scopul FOR
        currentTable = new SymTable("for", currentTable);
        tables.push_back(currentTable);
    } 
    '(' assignment_stmt ';' bool_expr ';' assignment_stmt ')' '{' list '}'  
    {
        // Revenire la scopul părinte
        currentTable = currentTable->prev;
    }
    | CONTINUE ';'
    | BREAK ';'
    | RETURN bool_expr ';'
    ;


/*Asignari ale membrilor unei clase*/
init_instante             :     ID ASSIGN expr ';'
                          |     init_instante ID ASSIGN expr ';'
                          ;
/*Expresii de asignare pentru variabile, clase si array-uri*/
assignment_stmt           :     left_hand_side ASSIGN expr
                          |     TYPE ID '[' list_array ']' ASSIGN '{' init_list '}'
                          ;

left_hand_side            :     ID '.' ID
                          |     ID
                          |     TYPE ID
                          |     ID '[' list_array ']'
                          ;

/*Apeluri de functii*/
call_func                 : ID '(' call_list ')' {
                                if (!currentTable->existsId($1)) { // Verifică dacă funcția există
                                    errorCount++;
                                    yyerror(("Function " + std::string($1) + " not declared at line: " + std::to_string(yylineno)).c_str());
                                }
                           }
                           | ID '(' ')'
                           | PRINT '(' expr ')'
                           | TYPEOF '(' expr ')'
                           ;


/*Parametrii de apel al unei functii*/
call_list                 :     call_list ',' expr
                          |     expr
                          ;

/*Expresii boolene (true sau false)*/
bool_expr                 :     '(' bool_expr AND bool_expr ')'
                          |     bool_expr AND bool_expr
                          |     '(' bool_expr OR bool_expr ')'
                          |     bool_expr OR bool_expr
                          |     '(' NOT bool_expr ')'
                          |     BVAL
                          |     NOT bool_expr
                          |     expr
                          ;

logical_expr              :     expr EQ expr
                          |     '(' expr EQ expr ')'
                          |     expr NEQ expr
                          |     '(' expr NEQ expr ')'
                          |     expr GT expr
                          |     '(' expr GT expr ')'
                          |     expr GTE expr
                          |     '(' expr GTE expr ')'
                          |     expr LT expr
                          |     '(' expr LT expr ')'
                          |     expr LTE expr
                          |     '(' expr LTE expr ')'
                          ;
                          
expr                      :     arithm_expr
                          |     logical_expr
                          |    '(' expr ')'
                          |     '-' expr
                          |     ID
                          |     IVAL
                          |     FVAL
                          |     CVAL
                          |     SVAL
                          |     ID '[' list_array ']'
                          |     ID '.' ID
                          |     ID '.' call_func
                          |     call_func
                          ;

/*Expresii aritmetice*/
arithm_expr               :     expr '+' expr  
                          |     expr '*' expr    
                          |     expr '/' expr
                          |     expr '-' expr
                          |     expr '%' expr
                          ;
                          


%%
void yyerror(const char * s){
     cout << "error: " << s << " at line: " << yylineno << endl;
}

int main(int argc, char** argv) {
    #ifdef YYDEBUG
    yydebug = 1;
    #endif
    yyin = fopen(argv[1], "r");
    //current = new SymTable("global");
    //tables.push_back(current);
    yyparse();

    //std::cout << "Variables in global scope:" << std::endl;
    for(auto table : tables) {
        table->printTable("scope.txt");
    }
    
    // Eliberare memorie pentru toate tabelele
    for (SymTable* table : tables) {
        delete table;
    }

    if (errorCount > 0) {
        std::cout << "There were " << errorCount << " errors." << std::endl;
    } else {
        std::cout << "The program is correct!" << std::endl;
    }

    return 0;
}
