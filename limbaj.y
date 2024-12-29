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
%token BGIN END ASSIGN NR BGINGLOBAL ENDGLOBAL BGINVARS ENDVARS BGINCLASS ENDCLASS BGINFUNC ENDFUNC CLASS CONST
%token EQ NEQ GT LT GTE LTE AND OR NOT
%token PRINT TYPEOF EVAL IF ELSE WHILE FOR DO LOOP BREAK CONTINUE RETURN
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
global_declarations       :     BGINGLOBAL declarations ENDGLOBAL  {
                                    current = globalTable;
                                    tables.push_back(globalTable);
                                }
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
                                   if (!currentTable->existsId($2)) {
                                       currentTable->addVar($1, $2);
                                   } else {
                                       errorCount++; 
                                       yyerror(("Variable already defined at line: " + std::to_string(yylineno)).c_str());
                                   }
                               }
                          |    TYPE ID '[' list_array ']' {
                                   if (!currentTable->existsId($2)) {
                                       currentTable->addVar($1, $2);
                                   } else {
                                       errorCount++; 
                                       yyerror(("Variable already defined at line: " + std::to_string(yylineno)).c_str());
                                   }
                               }
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
                          ;

/*Permite definirea oricator metode la o clasa*/
methods_list              :    methods_list method
                          |    
                          ;

/*Definirea unei metode într-o clasă*/
method
    : TYPE ID '(' list_param ')' '{' {
        // Verifică dacă funcția este deja definită
        if (currentTable->existsId($2)) { 
            errorCount++;
            yyerror(("Function " + std::string($2) + " already defined at line: " + std::to_string(yylineno)).c_str());
        } else {
            // Adaugă funcția în tabela simbolurilor a clasei
            currentTable->addFunc($1, $2);
        }
    } fblock '}' {
        // Revenire la tabelul simbolurilor al clasei
        currentTable = currentTable->prev;
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
def_func
    : TYPE ID '(' list_param ')' '{' {
        if(!currentTable->existsFunc($2)) {
            currentTable->addFunc($1, $2);
        }
        currentTable = new SymTable($2, currentTable); // Crează un tabel de simboluri pentru funcția curentă
        tables.push_back(currentTable);  // Adaugă tabelul global
    } fblock '}' {
        currentTable = currentTable->prev; // Revenire la scopul părinte
    }
    | TYPE ID '(' list_param ')' ';' {
        if(!currentTable->existsFunc($2)) {
            currentTable->addFunc($1, $2);
        }
        currentTable = new SymTable($2, currentTable); // Crează un tabel de simboluri pentru funcția curentă
        tables.push_back(currentTable);  // Adaugă tabelul global
    }
    {
        currentTable = currentTable->prev; // Revenire la scopul părinte
    }
    ;

/*Lista de parametri pentru functii*/
list_param                :    param
                          |    list_param ',' param
                          |    
                          ;  
/*Parametrii pentru functii*/
param                     :     TYPE ID
                          ;

/*Definirea unui array: uni, multi dimensional*/
list_array                :    list_array ',' INT
                          |    INT
                          ;

/*Blocul unei functii*/
fblock
    :  fblock   
    {
        // Creează un nou scop pentru acest bloc
        currentTable = new SymTable("block", currentTable);
        tables.push_back(currentTable); // Salvează tabelă în lista globală
    } statement {
        // Revenire la scopul părinte după încheierea blocului
        currentTable = currentTable->prev;
    }
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
        // Creează o tabelă pentru scopul IF
        currentTable = new SymTable("if", currentTable);
        tables.push_back(currentTable);
    }  
    {
        // Revenire la scopul părinte
        currentTable = currentTable->prev;
    }
    | IF '(' bool_expr ')' '{' list '}' ELSE '{' list '}' {
        // Creează o tabelă pentru scopul IF
        currentTable = new SymTable("if", currentTable);
        tables.push_back(currentTable);
    } 
    {
        // Revenire la scopul părinte
        currentTable = currentTable->prev;
    }
    | WHILE '(' bool_expr ')' '{' list '}' {
        // Creează o tabelă pentru scopul WHILE
        currentTable = new SymTable("while", currentTable);
        tables.push_back(currentTable);
    }  
    {
        // Revenire la scopul părinte
        currentTable = currentTable->prev;
    }
    | DO '{' list '}' WHILE '(' bool_expr ')' 
    {
        // Creează o tabelă pentru scopul WHILE
        currentTable = new SymTable("while", currentTable);
        tables.push_back(currentTable);
    } 
    {
        // Revenire la scopul părinte
        currentTable = currentTable->prev;
    }
    | LOOP '{' list '}' {
        // Creează o tabelă pentru scopul LOOP
        currentTable = new SymTable("loop", currentTable);
        tables.push_back(currentTable);
    }
    {
        // Revenire la scopul părinte
        currentTable = currentTable->prev;
    }
    | FOR '(' assignment_stmt ';' bool_expr ';' assignment_stmt ')' '{' list '}' {
        // Creează o tabelă pentru scopul FOR
        currentTable = new SymTable("for", currentTable);
        tables.push_back(currentTable);
    }  
    {
        // Revenire la scopul părinte
        currentTable = currentTable->prev;
    }
    | assignment_stmt ';'
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
                          |     left_hand_side ASSIGN STRING
                          ;

left_hand_side            :     ID '.' ID
                          |     ID '[' list_array ']'
                          |     TYPE ID  
                          |     ID
                          ;

/*Apeluri de functii*/
call_func                 : ID '(' call_list ')' {
                                if (!currentTable->existsId($1)) { // Verifică dacă funcția există
                                    errorCount++;
                                    yyerror(("Function " + std::string($1) + " not declared at line: " + std::to_string(yylineno)).c_str());
                                }
                           }
                           | ID '(' ')'
                           | PRINT '(' STRING ')'
                           | TYPEOF '(' ID ')'
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
                          |     BOOL
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
                          |     INT
                          |     FLOAT
                          |     CHAR
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
    current = new SymTable("global");
    tables.push_back(current);
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
