%{
#include <assert.h>
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
    db::NodeList*      list;
}

%token IDENT_P

%token <str>  FCONST BCONST IDENT SCONST XCONST OP
%token <ival> ICONST PARAM
%token EQUALS_GREATER LESS_EQUALS GREATER_EQUALS NOT_EQUALS

%type <keyword> unreserved_keyword
%type <str> name typename column_name constraint_name_opt
%type <node> SchemaOptName

%type <list> stmtblock stmtmulti
%type <node> stmt CreateTableStmt column_constraint
%type <boolean> OptTemp
%type <ival> conflict_clause

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
        ABORT_P ASC AUTOINCREMENT
        CONFLICT CONSTRAINT CREATE
        DESC
        EXISTS
        FAIL
        IGNORE IF_P
        NOT
        KEY
        ON_P
        PRIMARY
        REPLACE ROLLBACK
        TABLE TEMP TEMPORARY

/* Grammar follows */
%%

/*
 *	The target production for the whole parse.
 */
stmtblock:
    stmtmulti
    {
        parser->nodes = $1;
    }
    ;

stmtmulti:
    stmtmulti ';' stmt
        {
            if (!$1 && !$3) {
                $$ = nullptr;
            } else if ($1 && $3) {
                $1->Append($3);
                $$ = $1;
            } else if ($1) {
                assert(!$3);
                $$ = $1;
            } else {
                assert(!$1);
                $$ = new NodeList($3);
            }
		}
	| stmt
		{
		    if ($1) {
		        $$ = new NodeList($1);
		    } else {
		        $$ = nullptr;
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
    CREATE OptTemp TABLE IF_P NOT EXISTS SchemaOptName '(' column_def_list ')'
        {
             $$ = NULL;
        }
    | CREATE OptTemp TABLE SchemaOptName '(' column_def_list ')'
        {
            $$ = NULL;
        }
    ;

OptTemp:
    TEMPORARY { $$ = true; }
    | TEMP    { $$ = true; }
    | /*empty*/  { $$ = false; }

unreserved_keyword:
    ABORT_P
    | CONFLICT
    | EXISTS
    | FAIL
    | IF_P
    | IGNORE
    | KEY
    | REPLACE
    | ROLLBACK
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

typename:
    IDENT
        {
             $$ = $1;
        }
    ;

column_name:
    name
         {
             $$ = $1;
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

column_def_list:
    column_def_list ',' column_def
        {
        }
    | column_def
        {
        }
    ;

column_def:
    column_name typename
        {
        }
    | column_name typename column_constraint
        {
        }
    ;

column_constraint:
    constraint_name_opt primary_key_constraint
        {
        }
    ;

constraint_name_opt:
    CONSTRAINT name
        {
             $$ = (char*) $1;
        }
    | /*empty*/
        {
            $$ = NULL;
        }
    ;

primary_key_constraint:
    PRIMARY KEY conflict_clause
        {
        }
    | PRIMARY KEY ASC conflict_clause
        {
        }
    | PRIMARY KEY DESC conflict_clause
        {
        }
    | PRIMARY KEY conflict_clause AUTOINCREMENT
        {
        }
    | PRIMARY KEY ASC conflict_clause AUTOINCREMENT
        {
        }
    | PRIMARY KEY DESC conflict_clause AUTOINCREMENT
        {
        }
    ;

conflict_clause:
    ON_P CONFLICT ROLLBACK
        {
        }
    | ON_P CONFLICT ABORT_P
        {
        }
    | ON_P CONFLICT FAIL
        {
        }
    | ON_P CONFLICT IGNORE
        {
        }
    | ON_P CONFLICT REPLACE
        {
        }
    | /*empty*/
        {
        }
    ;
%%

void help() {

}