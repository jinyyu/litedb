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
    db::ResTarget*       target;
    db::RangeVar*        range;
}

%token IDENT_P

%token <str>  FCONST BCONST IDENT SCONST XCONST OP
%token <ival> ICONST PARAM
%token EQUALS_GREATER LESS_EQUALS GREATER_EQUALS NOT_EQUALS

%type <keyword> unreserved_keyword
%type <str>     name typename opt_alias_clause
%type <list>    stmtblock stmtmulti
%type <list>    column_def_list column_constraint_list table_constraint column_list
                target_list from_clause from_list
%type <node>    stmt CreateTableStmt column_constraint type constraint column_def expr value
%type <boolean> OptTemp distinct_clause
%type <node>    SelectStmt where_clause
%type <target>  target_el
%type <range>	table_ref

%token <keyword>
        ABORT_P ALL AS ASC AUTOINCREMENT
        CHECK CONFLICT CONSTRAINT CREATE
        DEFAULT DISTINCT DESC
        EXISTS
        FROM
        IGNORE IF_P
        NOT NULL_P
        KEY
        ON_P
        PRIMARY
        REPLACE ROLLBACK
        SELECT
        TABLE TEMP TEMPORARY
        UNIQUE
        WHERE


/* Precedence: lowest to highest */
%nonassoc	SET
%left		UNION EXCEPT
%left		INTERSECT
%left		OR
%left		AND
%right		NOT
%nonassoc	IS ISNULL NOTNULL	/* IS sets precedence for IS NULL, etc */
%nonassoc	'<' '>' '=' LESS_EQUALS GREATER_EQUALS NOT_EQUALS
%nonassoc	BETWEEN IN_P LIKE ILIKE SIMILAR NOT_LA
%nonassoc	ESCAPE			/* ESCAPE must be just above LIKE/ILIKE/SIMILAR */
%left		POSTFIXOP		/* dummy for postfix Op rules */
/*
 * To support target_el without AS, we must give IDENT an explicit priority
 * between POSTFIXOP and Op.  We can safely assign the same priority to
 * various unreserved keywords as needed to resolve ambiguities (this can't
 * have any bad effects since obviously the keywords will still behave the
 * same as if they weren't keywords).  We need to do this:
 * for PARTITION, RANGE, ROWS, GROUPS to support opt_existing_window_name;
 * for RANGE, ROWS, GROUPS so that they can follow a_expr without creating
 * postfix-operator problems;
 * for GENERATED so that it can follow b_expr;
 * and for NULL so that it can follow b_expr in ColQualList without creating
 * postfix-operator problems.
 *
 * To support CUBE and ROLLUP in GROUP BY without reserving them, we give them
 * an explicit priority lower than '(', so that a rule with CUBE '(' will shift
 * rather than reducing a conflicting rule that takes CUBE as a function name.
 * Using the same precedence as IDENT seems right for the reasons given above.
 *
 * The frame_bound productions UNBOUNDED PRECEDING and UNBOUNDED FOLLOWING
 * are even messier: since UNBOUNDED is an unreserved keyword (per spec!),
 * there is no principled way to distinguish these from the productions
 * a_expr PRECEDING/FOLLOWING.  We hack this up by giving UNBOUNDED slightly
 * lower precedence than PRECEDING and FOLLOWING.  At present this doesn't
 * appear to cause UNBOUNDED to be treated differently from other unreserved
 * keywords anywhere else in the grammar, but it's definitely risky.  We can
 * blame any funny behavior of UNBOUNDED on the SQL standard, though.
 */
%nonassoc	UNBOUNDED		/* ideally should have same precedence as IDENT */
%nonassoc	IDENT GENERATED NULL_P PARTITION RANGE ROWS GROUPS PRECEDING FOLLOWING CUBE ROLLUP
%left		Op OPERATOR		/* multi-character ops and user-defined operators */
%left		'+' '-'
%left		'*' '/' '%'
%left		'^'
/* Unary Operators */
%left		AT				/* sets precedence for AT TIME ZONE */
%left		COLLATE
%right		UMINUS
%left		'[' ']'
%left		'(' ')'
%left		TYPECAST
%left		'.'
/*
 * These might seem to be low-precedence, but actually they are not part
 * of the arithmetic hierarchy at all in their use as JOIN operators.
 * We make them high-precedence to support their use as function names.
 * They wouldn't be given a precedence at all, were it not that we need
 * left-associativity among the JOIN rules themselves.
 */
%left		JOIN CROSS LEFT FULL RIGHT INNER_P NATURAL
/* kluge to keep xml_whitespace_option from causing shift/reduce conflicts */
%right		PRESERVE STRIP_P

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
    CreateTableStmt         { $$ = $1; }
    | SelectStmt            { $$ = $1; }
    | /*empty*/             { $$ = NULL; }
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
            n->constraints = $3;
            $$ = (Node*) n;
        }
    | name type CONSTRAINT name column_constraint_list
        {
            ColumnDef* n = makeNode(ColumnDef);
            n->columnName = $1;
            n->typeName = (Typename*) $2;
            n->constraintName = $4;
            n->constraints = $5;
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
    | PRIMARY KEY
        {
            ColumnConstraint* n = makeNode(ColumnConstraint);
            n->constraint = CONSTRAINT_PRIMARY_KEY,
            $$ = (Node*) n;
        }
    | UNIQUE
        {
            ColumnConstraint* n = makeNode(ColumnConstraint);
            n->constraint = CONSTRAINT_UNIQUE,
            $$ = (Node*) n;
        }
    | CHECK expr
        {
            ColumnConstraint* n = makeNode(ColumnConstraint);
            n->constraint = CONSTRAINT_CHECK,
            $$ = (Node*) n;
        }
    | DEFAULT value
        {
            ColumnConstraint* n = makeNode(ColumnConstraint);
            n->constraint = CONSTRAINT_DEFAULT,
            n->defaultValue = (Value*) $2;
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
    ICONST      { $$ = (Node*) makeInteger($1); }
    | SCONST    { $$ = (Node*) makeString($1); }
    | NULL_P    { $$ = (Node*) makeNull(); }
    ;

name:
    IDENT                   { $$ = (char*) $1; }
    | unreserved_keyword    {   $$ = (char*) $1; }
    ;

typename:
    name    {  $$ = $1; }

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
    | UNIQUE  '(' column_list ')'
        {
            TableConstraint* n = makeNode(TableConstraint);
            n->constraint = CONSTRAINT_UNIQUE;
            n->columnList = $3;
            $$ = (Node*) n;
        }
    | CHECK expr
        {
            TableConstraint* n = makeNode(TableConstraint);
            n->constraint = CONSTRAINT_CHECK;
            n->expr = (Expr*) $2;
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
    name
        {
            $$ = (Node*) makeString($1);
        }
    ;

SelectStmt:
    SELECT distinct_clause target_list from_clause
    where_clause
        {
            SelectStmt* stmt = (SelectStmt*) makeNode(SelectStmt);
            stmt->distinct = $2;
            stmt->targetList = $3;
            stmt->fromClause = $4;
            stmt->whereClause = $5;
            $$ = (Node*) stmt;
        }
    ;

distinct_clause:
    DISTINCT    { $$ = true; }
    | ALL       { $$ = false;  }
    | /*empty*/ { $$ = false; }
    ;

target_list:
    target_el
        {
            $$ = new List<Node>((Node*)$1);
        }
    | target_list ',' target_el
        {
            $1->Append((Node*) $3);
            $$ = $1;
        }
    ;

target_el:
    expr AS name
        {
			$$ = (ResTarget*) makeNode(ResTarget);
			$$->name = $3;
			$$->val = (Node *) $1;
        }
    | expr IDENT
        {
  			$$ = (ResTarget*) makeNode(ResTarget);
  			$$->name = $2;
  			$$->val = (Node *) $1;
        }
    | expr
        {
     		$$ = (ResTarget*) makeNode(ResTarget);
     		$$->name = NULL;
     		$$->val = (Node *) $1;
        }
    | '*'
        {
     		$$ = (ResTarget*) makeNode(ResTarget);
     		$$->name = NULL;
     		$$->val = (Node *) makeNode(A_Star);
        }
    ;

from_clause:
    FROM from_list  { $$ = $2; }
    | /*empty*/     { $$ = NULL; }
    ;

from_list:
    table_ref
        {
            $$ = new List<Node>((Node*)$1);
        }
    | from_list ',' table_ref
        {
            $1->Append((Node*) $3);
            $$ = $1;
        }

table_ref:
    name opt_alias_clause
        {
            $$ = makeNode(RangeVar);
            $$->relname = $1;
            $$->alias = $2;
        }
    ;

opt_alias_clause:
    name        { $$ = $1; }
    | /*empty*/ { $$ = NULL; }
    ;

where_clause:
    WHERE expr	    { $$ = $2; }
    | /*EMPTY*/		{ $$ = NULL; }
    ;

unreserved_keyword:
    ABORT_P
    | CONFLICT
    | EXISTS
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