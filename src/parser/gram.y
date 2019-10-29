%{
#include <litesql/parser.h>
#include <litesql/nodes.h>
#include "gram.hpp"
#define YYERROR_VERBOSE
using namespace db;

%}

%pure-parser
%expect 0
%define api.prefix {parser_}
%locations

%parse-param {db::Parser* parser}
%lex-param   {db::Parser* parser}

%union
{
    int            ival;
    bool           boolean;
    char*          str;
    const char*    keyword;
    db::Node*      node;
    std::list<db::Node*>* list;
}

%token IDENT_P

%token <str>  FCONST BCONST IDENT SCONST XCONST OP
%token <ival> ICONST PARAM
%token EQUALS_GREATER LESS_EQUALS GREATER_EQUALS NOT_EQUALS

%type <keyword> unreserved_keyword
%type <str> name
%type <node> SchemaOptName

%type <list> stmtmulti
%type <node> stmt
%type <boolean> OptTemp
%type <node> CreateTableStmt


// Define operator precedence early so that this is the first occurance
// of the operator tokens in the grammer.  Keeping the operators together
// causes them to be assigned integer values that are close together,
// which keeps parser tables smaller.
//
%left   OR
%left   AND
%right  NOT
%left   EQ NE ISNULL NOTNULL IS LIKE GLOB BETWEEN IN
%left   GT GE LT LE
%left   BITAND BITOR LSHIFT RSHIFT
%left   PLUS MINUS
%left   STAR SLASH REM
%left   CONCAT
%right  UMINUS UPLUS BITNOT

%token <keyword>
        CREATE
        EXISTS
        IF_P
        NOT
        TABLE TEMP TEMPORARY

/* Grammar follows */
%%

stmtmulti:
    stmtmulti ';' stmt
        {
            if ($3) {
                $$->push_back($3);
            }
		}
	| stmt
		{
		    if ($1) {
		        $$->push_back($1);
		    }
	    }
	;

stmt:
    CreateTableStmt
        {
            $$ = $1;
        }
    | /*empty*/
        {
            $$ = NULL;
        }
     ;

CreateTableStmt:
    CREATE OptTemp TABLE IF_P NOT EXISTS SchemaOptName
        {
             $$ = NULL;
        }
    ;

OptTemp:
    TEMPORARY { $$ = true; }
    | TEMP    { $$ = true; }
    | /*empty*/  { $$ = false; }

unreserved_keyword:
    EXISTS
    | IF_P
    | TEMP
    | TEMPORARY
    ;

name:
    IDENT
        {
            $$ = (char*) $1;
        }
    | unreserved_keyword
        {
            $$ = (char*) $1;
        }
    ;

SchemaOptName:
    name
        {
            SchemaOptName* n = makeNode(SchemaOptName);
            n->name = $1;
            $$ = (Node*) n;
        }
    | name '.' name
        {
            SchemaOptName* n = makeNode(SchemaOptName);
            n->schema = $1;
            n->name = $3;
            $$ = (Node*) n;
        }
    ;


%%

void help() {

}