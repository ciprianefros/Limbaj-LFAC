%{
#include <iostream>
#include <vector>
#include "utils.h"
extern FILE* yyin;
extern char* yytext;
extern int yylineno;
extern int yylex();

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
                                       //std::cout << "The program is correct!" << std::endl; 
                                }
                          ;

/*Global scope*/
global_declarations       :     BGINGLOBAL   {
                                    currentTable = globalTable;
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

F_TYPE : INT 
     {
          currentFunction.returnType = TYPE_INT;
     }
     | FLOAT 
     {
          currentFunction.returnType = TYPE_FLOAT;
     }
     | STRING 
     {
          currentFunction.returnType = TYPE_STRING;
     }
     | BOOL 
     {
          currentFunction.returnType = TYPE_BOOL;
     }
     | CHAR  
     {
          currentFunction.returnType = TYPE_CHAR;
     }
     ;

V_TYPE : INT 
     {
          currentVariable.type.typeName = TYPE_INT;
     }
     | FLOAT 
     {
          currentVariable.type.typeName = TYPE_FLOAT;
     }
     | STRING 
     {
          currentVariable.type.typeName = TYPE_STRING;
     }
     | BOOL 
     {
          currentVariable.type.typeName = TYPE_BOOL;
     }
     | CHAR  
     {
          currentVariable.type.typeName = TYPE_CHAR;
     }
     ;

/*Declararea de variabile sau array-uri*/
decl_var                  :    V_TYPE ID  
                                {
                                    if(!exists_or_add($2, false)) {

                                    }
                                }
                          |    V_TYPE ID '[' list_array ']' 
                                {
                                   if(!exists_or_add($2, true)) {

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
        if (currentTable->existsClass($2)) { // Verifică dacă clasa există deja
            errorCount++;
            yyerror(("Class already defined at line: " + std::to_string(yylineno)).c_str());
        } else {
            currentClass.name = $2;
            currentTable->addClass($2);
            currentTable = new SymTable($2, currentTable);// Creează un nou tabel de simboluri pentru clasă
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
class_memb                :    ID ':' V_TYPE ';'
                               {
                                   exists_or_add($1, false);
                               }
                          |    ID '[' list_array ']' ':'  V_TYPE ';'
                               {
                                   exists_or_add($1, true);
                               }
                          ;
/*Permite definirea oricator metode la o clasa*/
methods_list              :    methods_list method
                          |    
                          ;

/*Definirea unei metode într-o clasă*/
method
    : F_TYPE ID '(' list_param ')' '{'  {
        // Verifică dacă funcția este deja definită
        if (currentTable->existsFunc($2)) 
        { 
            errorCount++;
            yyerror(("Function " + std::string($2) + " already defined at line: " + std::to_string(yylineno)).c_str());
        } else {
            // Adaugă funcția în tabela simbolurilor a clasei
            currentTable->addFunc(currentFunction.returnType, $2, currentParams, currentClass.name);
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
def_func  : F_TYPE ID '(' list_param ')'
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
                    | F_TYPE ID '(' list_param ')' ';' {
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
    : param {
        currentParams.push_back(currentVariable);
    }
    | list_param ',' param {
        currentParams.push_back(currentVariable);
    }
    |
    ;


/*Parametrii pentru functii*/
param                     :     V_TYPE ID  
                                {
                                    currentVariable.name = $2;
                                    currentVariable.type.isArray = false;
                                    
                                }
                          |     V_TYPE ID '[' list_array ']' 
                                {
                                    currentVariable.name = $2;
                                    currentVariable.type.isArray = true;
                                }
                          ;


/*Definirea unui array: uni, multi dimensional*/
list_array                :    list_array ',' IVAL {currentArraySizes.push_back($3)}
                          |    IVAL { currentArraySizes.push_back($1)}
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
    | call_method ';'
    | decl_var ';'
    | ID ID ';' {
            exists_or_add_for_custom_type($2, $1);
    }
    | ID ID '{' init_instante '}' ';' {
            exists_or_add_for_custom_type($2, $1);
        }
    | IF 
        {
            addScopeName("if");
        } 
        '(' bool_expr ')' '{' list '}'
        else_statement   
        {  
            // Revenire la scopul părinte
            scopeStack.pop();
            currentTable = currentTable->prev;
        }
    | WHILE 
        {
            addScopeName("while");
        }
        '(' bool_expr ')' '{' list '}'   
        {
            scopeStack.pop();
            currentTable = currentTable->prev;
        }
    | DO 
        {
            addScopeName("do-while");
        } 
        '{' list '}' WHILE '(' bool_expr ')' 
        {
            scopeStack.pop();
            currentTable = currentTable->prev;
        }
    | LOOP 
        {
            addScopeName("loop");
        }
        '{' list '}' 
        {
            scopeStack.pop();
            currentTable = currentTable->prev;
        }
    | FOR 
        {
            addScopeName("for");
        } 
        '(' assignment_stmt ';' bool_expr ';' assignment_stmt ')' '{' list '}'  
        {
            scopeStack.pop();
            currentTable = currentTable->prev;
        }
    | CONTINUE ';'
    | BREAK ';'
    | RETURN bool_expr ';'
    ;

else_statement  : 
                | ELSE 
                {
                    addScopeName("else");
                } 
                '{' list '}'
                {
                    scopeStack.pop();
                    currentTable = currentTable->prev;
                }
                ;


/*Asignari ale membrilor unei clase*/
init_instante             :     ID ASSIGN expr ';'
                          |     init_instante ID ASSIGN expr ';'
                          ;
/*Expresii de asignare pentru variabile, clase si array-uri*/
assignment_stmt           :     left_hand_side ASSIGN expr 
                                {
                                    if(FindToBeModifiedVar(variableToAssign)) {
                                        expr1 = stiva.back();
                                        SetNewValue(modifiedVariable, expr1);
                                    }

                                    stiva.pop_back();
                                }
                          |     V_TYPE ID '[' list_array ']' 
                                {
                                    variableToAssign.varName = $2;
                                    variableToAssign.varType = 1;
                                } ASSIGN '{' init_list '}' {/*exists_or_add($1, 1);*/}
                          ;

left_hand_side            :     ID '.' ID 
                                { 
                                    checkObject($1, $3);
                                    variableToAssign.varName = $1;
                                    variableToAssign.varType = 2;
                                    variableToAssign.varField = $3;
                                }
                          |     ID
                          {
                                variableToAssign.varName = $1;
                                variableToAssign.varType = 0;
                          }
                          |     V_TYPE ID 
                                {
                                exists_or_add($2, 0);
                                variableToAssign.varName = $2;
                                variableToAssign.varType = 0;
                                }
                          |     ID '[' list_array ']' 
                                {
                                    variableToAssign.varName = $1;
                                    variableToAssign.varType = 1;
                                }
                          ;

/*Apeluri de functii*/
call_func                 : ID '(' call_list ')' 
                            {
                                checkFunction($1);
                            }
                           | ID '(' ')'
                           {
                                checkFunction($1);
                           }
                           | PRINT '(' bool_expr ')' {runPrint()}
                           | TYPEOF '(' bool_expr ')' {runTypeOf()}
                           ;

call_method               : ID '.' ID '(' call_list ')' 
                            {
                                setCurrentClassName($1);
                                checkMethod($3);
                            }
                          | ID '.' ID '(' ')'
                            {
                                setCurrentClassName($1);
                                checkMethod($3);
                            } 
                           ;

/*Parametrii de apel al unei functii*/
call_list                 :     call_list ',' expr
                          { 
                                currentParams.push_back(currentVariable);
                          }
                          |     expr
                          {                                
                                currentParams.push_back(currentVariable);
                          }
                          ;

/*Expresii boolene (true sau false)*/
bool_expr                 :     '(' bool_expr AND bool_expr ')' {Operation_on_stack(BAND);}
                          |     bool_expr AND bool_expr         {Operation_on_stack(BAND);}
                          |     '(' bool_expr OR bool_expr ')'  {Operation_on_stack(BOR);}
                          |     bool_expr OR bool_expr          {Operation_on_stack(BOR);}
                          |     '(' NOT bool_expr ')'           {
                                                                    expr1 = stiva.back();
                                                                    stiva.pop_back();
                                                                    stiva.push_back(new ASTNode(BNOT, expr1));
                                                                }
                          |     NOT bool_expr                   {
                                                                    expr1 = stiva.back();
                                                                    stiva.pop_back();
                                                                    stiva.push_back(new ASTNode(BNOT, expr1));
                                                                }
                          |     expr
                          ;

logical_expr              :     expr EQ expr            {Operation_on_stack(BEQ);}
                          |     '(' expr EQ expr ')'    {Operation_on_stack(BEQ);}
                          |     expr NEQ expr           {Operation_on_stack(BNEQ);}
                          |     '(' expr NEQ expr ')'   {Operation_on_stack(BNEQ);}
                          |     expr GT expr            {Operation_on_stack(BGT);}
                          |     '(' expr GT expr ')'    {Operation_on_stack(BGT);}
                          |     expr GTE expr           {Operation_on_stack(BGTE);}
                          |     '(' expr GTE expr ')'   {Operation_on_stack(BGTE);}
                          |     expr LT expr            {Operation_on_stack(BLT);}
                          |     '(' expr LT expr ')'    {Operation_on_stack(BLT);}
                          |     expr LTE expr           {Operation_on_stack(BLTE);}
                          |     '(' expr LTE expr ')'   {Operation_on_stack(BLTE);}
                          ;
                          
expr                      :     arithm_expr
                          |     logical_expr
                          |    '(' expr ')'
                          |     '-' expr
                          |     ID 
                          {
                                setCurrentVariableType($1);
                                variableFromExpr.varName = $1;
                                variableFromExpr.varType = 0;
                                PushVariableToStack();
                          }
                          |     IVAL
                          {
                                currentVariable.type.typeName = 0;
                                stiva.push_back(new ASTNode($1));
                          }
                          |     FVAL
                          {
                                currentVariable.type.typeName = 1;
                                stiva.push_back(new ASTNode($1));
                          }
                          |     CVAL 
                          {
                                currentVariable.type.typeName = 2;
                                stiva.push_back(new ASTNode($1))
                          }
                          |     BVAL
                          {
                                currentVariable.type.typeName = 3;
                                stiva.push_back(new ASTNode($1));
                          }
                          |     SVAL 
                          {
                                currentVariable.type.typeName = 4;
                                stiva.push_back(new ASTNode($1))
                          }
                          |     ID '[' list_array ']'
                          {
                                setCurrentVariableType($1);
                                variableFromExpr.varName = $1;
                                variableFromExpr.varType = 1;
                                variableFromExpr.varIndex[0] = currentArraySizes[0];
                                PushVariableToStack();
                          }
                          |     ID '.' ID {
                                setObjectMemberReturnType($1, $3);
                                currentVariable.type.typeName = objectMemberReturnType;
                                variableFromExpr.varName = $1;
                                variableFromExpr.varType = 2;
                                variableFromExpr.varField = $3;
                                PushVariableToStack();

                          }
                          |     call_method {
                                currentVariable.type.typeName = functionReturnType;
                                stiva.push_back(new ASTNode(0, functionReturnType));
                          }
                          |     call_func {
                                currentVariable.type.typeName = functionReturnType;
                                stiva.push_back(new ASTNode(0, functionReturnType));
                          }
                          ;

/*Expresii aritmetice*/
arithm_expr               :     expr '+' expr  {Operation_on_stack(ADD);}
                          |     expr '*' expr  {Operation_on_stack(MUL);} 
                          |     expr '/' expr  {Operation_on_stack(DIV);}
                          |     expr '-' expr  {Operation_on_stack(SUB);}
                          |     expr '%' expr  {Operation_on_stack(MOD);}
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
        std::cout << printToScreen << endl;
        std::cout << "The program is correct!" << std::endl;
    }

    return 0;
}
