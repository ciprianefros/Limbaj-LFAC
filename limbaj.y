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
                                       yyerror(("Variable already defined at line: " + std::to_string(yylineno)).c_str());
                                   }
                               }
                          |    TYPE ID '[' list_array ']' {
                                   if (!current->existsId($2)) {
                                       current->addVar($1, $2);
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
        if (current->existsId($2)) { // Verifică dacă clasa există deja
            errorCount++;
            yyerror(("Class already defined at line: " + std::to_string(yylineno)).c_str());
        } else {
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
        currentTable = new SymTable($2, currentTable); // Crează un tabel de simboluri pentru funcția curentă
        tables.push_back(currentTable);  // Adaugă tabelul global
    } fblock '}' {
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
    : '{' {
        // Creează un nou scop pentru acest bloc
        currentTable = new SymTable("block", currentTable);
        tables.push_back(currentTable); // Salvează tabelă în lista globală
    } fblock statement ';' '}' {
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
list                      :     statement ';'
                          |     list statement ';'
                          ;

/*Tot felul de expresii din interiorul programului*/
statement
    : call_func
    | decl_var
    | ID ID
    | ID ID '{' init_instante '}'
    | IF '(' bool_expr ')' '{' {
        // Creează o tabelă pentru scopul IF
        currentTable = new SymTable("if", currentTable);
        tables.push_back(currentTable);
    } list '}' {
        // Revenire la scopul părinte
        currentTable = currentTable->prev;
    }
    | IF '(' bool_expr ')' '{' list '}' ELSE '{' list '}' {
        // Creează o tabelă pentru scopul IF
        currentTable = new SymTable("if", currentTable);
        tables.push_back(currentTable);
    } list '}' ELSE '{' list '}' {
        // Revenire la scopul părinte
        currentTable = currentTable->prev;
    }
    | WHILE '(' bool_expr ')' '{' {
        // Creează o tabelă pentru scopul WHILE
        currentTable = new SymTable("while", currentTable);
        tables.push_back(currentTable);
    } list '}' {
        // Revenire la scopul părinte
        currentTable = currentTable->prev;
    }
    | DO '{' list '}' WHILE '(' bool_expr ')'
    | LOOP '{' list '}' {
        // Creează o tabelă pentru scopul LOOP
        currentTable = new SymTable("loop", currentTable);
        tables.push_back(currentTable);
    } list '}' {
        // Revenire la scopul părinte
        currentTable = currentTable->prev;
    }
    | FOR '(' assignment_stmt ';' bool_expr ';' assignment_stmt ')' '{' {
        // Creează o tabelă pentru scopul FOR
        currentTable = new SymTable("for", currentTable);
        tables.push_back(currentTable);
    } list '}' {
        // Revenire la scopul părinte
        currentTable = currentTable->prev;
    }
    | assignment_stmt
    | CONTINUE
    | BREAK
    | RETURN bool_expr
    ;

/*Asignari ale membrilor unei clase*/
init_instante             :     ID ASSIGN arithm_expr ';'
                          |     init_instante ID ASSIGN arithm_expr ';'
                          ;
/*Expresii de asignare pentru variabile, clase si array-uri*/
assignment_stmt           :     TYPE ID ASSIGN arithm_expr
                          |     TYPE ID ASSIGN STRING
                          |     ID ASSIGN arithm_expr
                          |     ID ASSIGN STRING
                          |     ID '[' list_array ']' ASSIGN arithm_expr
                          |     ID '[' list_array ']' ASSIGN call_func
                          |     ID '[' list_array ']' ASSIGN STRING
                          |     ID '.' ID ASSIGN arithm_expr
                          |     ID '.' ID ASSIGN STRING
                          ;

/*Apeluri de functii*/
call_func                 : ID '(' call_list ')' {
                               if (!current->existsId($1)) { // Verifică dacă funcția există
                                   errorCount++;
                                   yyerror(("Function " + std::string($1) + " not declared at line: " + std::to_string(yylineno)).c_str());
                               }
                           }
                           | ID '(' ')'
                           | PRINT '(' STRING ')'
                           | TYPEOF '(' ID ')'
                           ;


/*Parametrii de apel al unei functii*/
call_list                 :     call_list ',' arithm_expr
                          |     call_list ',' ID '(' call_list ')'
                          |     arithm_expr
                          |     ID '(' call_list ')'
                          ;

/*Expresii boolene (true sau false)*/
bool_expr                 :     '(' bool_expr AND bool_expr ')'
                          |     bool_expr AND bool_expr
                          |     '(' bool_expr OR bool_expr ')'
                          |     bool_expr OR bool_expr
                          |     '(' NOT bool_expr ')'
                          |     BOOL
                          |     NOT bool_expr
                          |     expression
                          ;

expression                :     arithm_expr EQ arithm_expr
                          |     '(' arithm_expr EQ arithm_expr ')'
                          |     arithm_expr NEQ arithm_expr
                          |     '(' arithm_expr NEQ arithm_expr ')'
                          |     arithm_expr GT arithm_expr
                          |     '(' arithm_expr GT arithm_expr ')'
                          |     arithm_expr GTE arithm_expr
                          |     '(' arithm_expr GTE arithm_expr ')'
                          |     arithm_expr LT arithm_expr
                          |     '(' arithm_expr LT arithm_expr ')'
                          |     arithm_expr LTE arithm_expr
                          |     '(' arithm_expr LTE arithm_expr ')'
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
                          |     ID '[' list_array ']'
                          |     ID '.' ID
                          |     ID '.' call_func
                          |     call_func //genereaza conflicte
                          ;
                          


%%
void yyerror(const char * s){
     cout << "error: " << s << " at line: " << yylineno << endl;
}

int main(int argc, char** argv) {
    yyin = fopen(argv[1], "r");
    current = new SymTable("global");
    tables.push_back(current);
    yyparse();

    std::cout << "Variables in global scope:" << std::endl;
    globalTable->printVars();

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
