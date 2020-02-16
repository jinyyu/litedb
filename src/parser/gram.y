%{
#include <assert.h>
#include <litedb/parser/parser.h>
#include <litedb/nodes/parsenodes.h>
#include <litedb/nodes/value.h>
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
    int                  ival;
    bool                 boolean;
    char*                str;
    const char*          keyword;
    db::Node*            node;
    db::List<db::Node>*  list;
}

%token IDENT_P

%token <str>  FCONST BCONST IDENT SCONST XCONST OP
%token <ival> ICONST PARAM
%token EQUALS_GREATER LESS_EQUALS GREATER_EQUALS NOT_EQUALS

%type <keyword> unreserved_keyword
%type <str> name typename
%type <list> stmtblock stmtmulti
%type <list> column_def_list column_constraint_list table_constraint column_list
%type <node> stmt CreateTableStmt column_constraint type constraint column_def expr value
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
        CHECK CONFLICT CONSTRAINT CREATE
        DEFAULT DESC
        EXISTS
        FAIL
        IGNORE IF_P
        NOT NULL_P
        KEY
        ON_P
        PRIMARY
        REPLACE ROLLBACK
        TABLE TEMP TEMPORARY
        UNIQUE

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
                $$ = new List<Node>($3);
            }
		}
	| stmt
		{
		    if ($1) {
		        $$ = new List<Node>($1);
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
    CREATE OptTemp TABLE name '(' column_def_list ')'
        {
            CreateTableStmt* n = makeNode(CreateTableStmt);
            n->temp = $2;
            n->name = $4;
            n->columns = $6;
            $$ = (Node*) n;
        }
    | CREATE OptTemp TABLE name '(' column_def_list ',' table_constraint ')'
        {
            CreateTableStmt* n = makeNode(CreateTableStmt);
            n->temp = $2;
            n->name = $4;
            n->columns = $6;
            n->table_constraints = $8;
            $$ = (Node*) n;
        }
    ;

OptTemp:
    TEMPORARY { $$ = true; }
    | TEMP    { $$ = true; }
    | /*empty*/  { $$ = false; }
    ;

column_def_list:
    column_def_list ',' column_def
        {
            $1->Append($3);
            $$ = $1;
        }
    | column_def
        {
            $$ = new List<Node>($1);
        }
    ;

column_def:
    name type
        {
            ColumnDef* n = makeNode(ColumnDef);
            n->columnName = $1;
            n->typeName = (Typename*) $2;
            $$ = (Node*) n;
        }
    | name type column_constraint_list
        {
            ColumnDef* n = makeNode(ColumnDef);
            n->columnName = $1;
            n->typeName = (Typename*) $2;
            n->columnConstraints = $3;
            $$ = (Node*) n;
        }
    | name type CONSTRAINT name column_constraint_list
        {
            ColumnDef* n = makeNode(ColumnDef);
            n->columnName = $1;
            n->typeName = (Typename*) $2;
            n->constraintName = $4;
            n->columnConstraints = $5;
            $$ = (Node*) n;
        }
    ;

column_constraint_list:
    column_constraint_list column_constraint
        {
            $1->Append($2);
            $$ = $1;
        }
    | column_constraint
        {
            $$ = new List<Node>($1);
        }
    ;

column_constraint:
    NOT NULL_P
        {
            ColumnConstraint* n = makeNode(ColumnConstraint);
            n->constraint = CONSTRAINT_NOT_NULL,
            $$ = (Node*) n;
        }
    | NOT NULL_P conflict_clause
        {
            ColumnConstraint* n = makeNode(ColumnConstraint);
            n->constraint = CONSTRAINT_NOT_NULL,
            n->conflictAlgorithm = (ConflictAlgorithm) $3;
            $$ = (Node*) n;
        }
    | PRIMARY KEY
        {
            ColumnConstraint* n = makeNode(ColumnConstraint);
            n->constraint = CONSTRAINT_PRIMARY_KEY,
            $$ = (Node*) n;
        }
    | PRIMARY KEY conflict_clause
        {
            ColumnConstraint* n = makeNode(ColumnConstraint);
            n->constraint = CONSTRAINT_PRIMARY_KEY,
            n->conflictAlgorithm = (ConflictAlgorithm) $3;
            $$ = (Node*) n;
        }
    | UNIQUE
        {
            ColumnConstraint* n = makeNode(ColumnConstraint);
            n->constraint = CONSTRAINT_UNIQUE,
            $$ = (Node*) n;
        }
    | UNIQUE conflict_clause
        {
            ColumnConstraint* n = makeNode(ColumnConstraint);
            n->constraint = CONSTRAINT_UNIQUE,
            n->conflictAlgorithm = (ConflictAlgorithm) $2;
            $$ = (Node*) n;
        }
    | CHECK expr
        {
            ColumnConstraint* n = makeNode(ColumnConstraint);
            n->constraint = CONSTRAINT_CHECK,
            $$ = (Node*) n;
        }
    | CHECK expr conflict_clause
        {
            ColumnConstraint* n = makeNode(ColumnConstraint);
            n->constraint = CONSTRAINT_CHECK,
            n->conflictAlgorithm = (ConflictAlgorithm) $3;
            $$ = (Node*) n;
        }
    | DEFAULT value
        {
            ColumnConstraint* n = makeNode(ColumnConstraint);
            n->constraint = CONSTRAINT_DEFAULT,
            n->defaultValue = (Value*) $2;
            $$ = (Node*) n;
        }
    | DEFAULT value conflict_clause
        {
            ColumnConstraint* n = makeNode(ColumnConstraint);
            n->constraint = CONSTRAINT_DEFAULT,
            n->defaultValue = (Value*) $2;
            n->conflictAlgorithm = (ConflictAlgorithm) $3;
            $$ = (Node*) n;
        }
    ;

type:
    typename
        {
            Typename* n = makeNode(Typename);
            n->name = $1;
            $$ = (Node*) n;
        }
    | typename '(' ICONST ')'
        {
            Typename* n = makeNode(Typename);
            n->name = $1;
            n->leftPrecision = $3;
            $$ = (Node*) n;
        }
    | typename '(' ICONST ',' ICONST ')'
        {
            Typename* n = makeNode(Typename);
            n->name = $1;
            n->leftPrecision = $3;
            n->rightPrecision = $5;
            $$ = (Node*) n;
        }
    ;

value:
    ICONST
        {
            $$ = (Node*) makeInteger($1);
        }
    | SCONST
        {
            $$ = (Node*) makeString($1);
        }
    | NULL_P
        {
             $$ = (Node*) makeNull();
        }
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
    name    {  $$ = $1; }

conflict_clause:
    ON_P CONFLICT ROLLBACK
        {
            $$ = CONFLICT_ROLLBACK;
        }
    | ON_P CONFLICT ABORT_P
        {
            $$ = CONFLICT_ABORT;
        }
    | ON_P CONFLICT FAIL
        {
            $$ = CONFLICT_FAIL;
        }
    | ON_P CONFLICT IGNORE
        {
            $$ = CONFLICT_IGNORE;
        }
    | ON_P CONFLICT REPLACE
        {
            $$ = CONFLICT_REPLACE;
        }
    ;

table_constraint:
    table_constraint ','  constraint
        {
            $1->Append($3);
        }
    | constraint
        {
            $$ = new List<Node>($1);
        }
    ;

constraint:
    PRIMARY KEY '(' column_list ')'
        {
            TableConstraint* n = makeNode(TableConstraint);
            n->constraint = CONSTRAINT_PRIMARY_KEY;
            n->columnList = $4;
            $$ = (Node*) n;
        }
    | PRIMARY KEY '(' column_list ')' conflict_clause
        {
            TableConstraint* n = makeNode(TableConstraint);
            n->constraint = CONSTRAINT_PRIMARY_KEY;
            n->columnList = $4;
            n->conflictAlgorithm = (ConflictAlgorithm) $6;
            $$ = (Node*) n;
        }
    | UNIQUE  '(' column_list ')'
        {
            TableConstraint* n = makeNode(TableConstraint);
            n->constraint = CONSTRAINT_UNIQUE;
            n->columnList = $3;
            $$ = (Node*) n;
        }
    | UNIQUE  '(' column_list ')' conflict_clause
        {
            TableConstraint* n = makeNode(TableConstraint);
            n->constraint = CONSTRAINT_UNIQUE;
            n->columnList = $3;
            n->conflictAlgorithm = (ConflictAlgorithm) $5;
            $$ = (Node*) n;
        }
    | CHECK expr
        {
            TableConstraint* n = makeNode(TableConstraint);
            n->constraint = CONSTRAINT_CHECK;
            n->expr = (Expr*) $2;
            $$ = (Node*) n;
        }
    | CHECK expr conflict_clause
        {
            TableConstraint* n = makeNode(TableConstraint);
            n->constraint = CONSTRAINT_CHECK;
            n->expr = (Expr*) $2;
            n->conflictAlgorithm = (ConflictAlgorithm) $3;
            $$ = (Node*) n;
        }
    ;

column_list:
    column_list ',' name
        {
            $1->Append((Node*) makeString($3));
            $$ = $1;
        }
    | name
        {
            $$ = new List<Node>((Node*) makeString($1));
        }
    ;

expr:
    name { /*todo*/}

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
%%

void help() {

}