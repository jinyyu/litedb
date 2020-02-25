%{
#include <assert.h>
#include <litedb/parser/parser.h>
#include <litedb/nodes/parsenodes.h>
#include <litedb/nodes/execnodes.h>
#include <litedb/nodes/value.h>
#include "gram.hpp"

#define YYERROR_VERBOSE



namespace db {

static Node* makeIntConst(int i);
static Node* makeStringConst(char* str);
static Node* makeFloatConst(char* str);

}

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
    db::Typename*        typnam;
}

%token IDENT_P

%token <str>  FCONST BCONST IDENT SCONST XCONST OP
%token <ival> ICONST PARAM
%token EQUALS_GREATER LESS_EQUALS GREATER_EQUALS NOT_EQUALS

%type <keyword> unreserved_keyword
%type <str>     name opt_alias_clause
%type <list>    stmtblock stmtmulti
%type <list>    column_def_list column_constraint_list table_constraint column_list
                target_list from_clause from_list
%type <ival>    opt_type_modifiers
%type <typnam>  Typename
%type <node>    stmt CreateTableStmt column_constraint constraint column_def
%type <node>    a_expr b_expr c_expr AexprConst columnref
%type <boolean> OptTemp distinct_clause
%type <node>    SelectStmt where_clause
%type <target>  target_el
%type <range>	table_ref

%token <keyword>
        ABORT_P ALL AND AS ASC AUTOINCREMENT

        BETWEEN

        CHECK CONFLICT CONSTRAINT CREATE

        DEFAULT DISTINCT DESC

        EXISTS

        FALSE_P FROM

        IGNORE IF_P INT_P INTEGER IS ISNULL

        NOT NULL_P

        KEY

        ON_P OR

        PRIMARY

        REPLACE ROLLBACK

        SELECT

        TABLE TEMP TEMPORARY TEXT TRUE_P

        UNIQUE

        VARCHAR

        WHERE


/* Precedence: lowest to highest */
%left		UNION
%left		OR
%left		AND
%right		NOT
%nonassoc	IS ISNULL NOTNULL	/* IS sets precedence for IS NULL, etc */
%nonassoc	'<' '>' '=' LESS_EQUALS GREATER_EQUALS NOT_EQUALS
%nonassoc	BETWEEN IN_P NOT_LA
%nonassoc	IDENT NULL_P
%left		'+' '-'
%left		'*' '/' '%'
%left		'^'
/* Unary Operators */
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
    name Typename
        {
            ColumnDef* n = makeNode(ColumnDef);
            n->columnName = $1;
            n->typeName = $2;
            $$ = (Node*) n;
        }
    | name Typename column_constraint_list
        {
            ColumnDef* n = makeNode(ColumnDef);
            n->columnName = $1;
            n->typeName = $2;
            n->constraints = $3;
            $$ = (Node*) n;
        }
    | name Typename CONSTRAINT name column_constraint_list
        {
            ColumnDef* n = makeNode(ColumnDef);
            n->columnName = $1;
            n->typeName = $2;
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
    | CHECK a_expr
        {
            ColumnConstraint* n = makeNode(ColumnConstraint);
            n->constraint = CONSTRAINT_CHECK,
            $$ = (Node*) n;
        }
    | DEFAULT b_expr
        {
            ColumnConstraint* n = makeNode(ColumnConstraint);
            n->constraint = CONSTRAINT_DEFAULT,
            n->defaultValue = (A_Expr*)$2;
            $$ = (Node*) n;
        }
    ;

name:
    IDENT                   { $$ = (char*) $1; }
    | unreserved_keyword    {   $$ = (char*) $1; }
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
    | UNIQUE  '(' column_list ')'
        {
            TableConstraint* n = makeNode(TableConstraint);
            n->constraint = CONSTRAINT_UNIQUE;
            n->columnList = $3;
            $$ = (Node*) n;
        }
    | CHECK a_expr
        {
            TableConstraint* n = makeNode(TableConstraint);
            n->constraint = CONSTRAINT_CHECK;
            n->expr = (A_Expr*) $2;
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

Typename:
    INT_P
        {
            $$ = makeNode(Typename);
            $$->name = (char*) "int4";
        }
    | INTEGER
        {
            $$ = makeNode(Typename);
            $$->name = (char*) "int4";
        }
    | TEXT
        {
             $$ = makeNode(Typename);
             $$->name = (char*) "text";
        }
    | VARCHAR opt_type_modifiers
        {
            $$ = makeNode(Typename);
            $$->name = (char*) "varchar";
            $$->typeMod = $2;
        }
    ;

opt_type_modifiers:
    '(' ICONST ')'      { $$ = $2; }
    | /*empty*/         { $$ = 0;  }

a_expr:
    c_expr      { $$ = $1; }
    | '+' a_expr %prec UMINUS
        {
            $$ = (Node*) makeA_Expr(AEXPR_OP, "+" , NULL, $2);
        }
    | '-' a_expr
        {
            $$ = (Node*) makeA_Expr(AEXPR_OP, "-" , NULL, $2);
        }
    | a_expr '+' a_expr
        {
            $$ = (Node*) makeA_Expr(AEXPR_OP, "+" , $1, $3);
        }
    | a_expr '-' a_expr
        {
            $$ = (Node*) makeA_Expr(AEXPR_OP, "-" , $1, $3);
        }
    | a_expr '*' a_expr
        {
            $$ = (Node*) makeA_Expr(AEXPR_OP, "*" , $1, $3);
        }
    | a_expr '/' a_expr
        {
            $$ = (Node*) makeA_Expr(AEXPR_OP, "/" , $1, $3);
        }
    | a_expr '<' a_expr
        {
            $$ = (Node*) makeA_Expr(AEXPR_OP, "<" , $1, $3);
        }
    | a_expr '>' a_expr
        {
            $$ = (Node*) makeA_Expr(AEXPR_OP, ">" , $1, $3);
        }
    | a_expr '=' a_expr
        {
            $$ = (Node*) makeA_Expr(AEXPR_OP, "=" , $1, $3);
        }
    | a_expr LESS_EQUALS a_expr
        {
            $$ = (Node*) makeA_Expr(AEXPR_OP, "<=" , $1, $3);
        }
    | a_expr GREATER_EQUALS a_expr
        {
             $$ = (Node*) makeA_Expr(AEXPR_OP, ">=" , $1, $3);
        }
    | a_expr NOT_EQUALS a_expr
        {
             $$ = (Node*) makeA_Expr(AEXPR_OP, "<>" , $1, $3);
        }
    | a_expr AND a_expr
        {
            $$ = NULL;
        }
    | a_expr OR a_expr
        {
            $$ = NULL;
        }
    | NOT a_expr
        {
            $$ = NULL;
        }
    | NOT_LA a_expr	%prec NOT
        {
            $$ = NULL;
        }
    | a_expr IS NULL_P	%prec IS
        {
			NullTest *n = makeNode(NullTest);
			n->arg = (Expr *) $1;
			n->nulltesttype = IS_NULL;
			$$ = (Node *)n;
        }
    | a_expr ISNULL
        {
			NullTest *n = makeNode(NullTest);
			n->arg = (Expr *) $1;
			n->nulltesttype = IS_NULL;
			$$ = (Node *)n;
        }
    | a_expr IS NOT NULL_P	%prec IS
        {
			NullTest *n = makeNode(NullTest);
			n->arg = (Expr *) $1;
			n->nulltesttype = IS_NOT_NULL;
			$$ = (Node *)n;
        }
    | a_expr NOTNULL
        {
			NullTest *n = makeNode(NullTest);
			n->arg = (Expr *) $1;
			n->nulltesttype = IS_NOT_NULL;
			$$ = (Node *)n;
        }
    | a_expr IS TRUE_P %prec IS
        {
			BooleanTest *b = makeNode(BooleanTest);
			b->arg = (Expr *) $1;
			b->booltesttype = IS_TRUE;
			$$ = (Node *)b;
        }
    | a_expr IS NOT TRUE_P %prec IS
        {
			BooleanTest *b = makeNode(BooleanTest);
			b->arg = (Expr *) $1;
			b->booltesttype = IS_NOT_TRUE;
			$$ = (Node *)b;
        }
    | a_expr IS FALSE_P	%prec IS
        {
			BooleanTest *b = makeNode(BooleanTest);
			b->arg = (Expr *) $1;
			b->booltesttype = IS_FALSE;
			$$ = (Node *)b;
        }
    | a_expr IS NOT FALSE_P	%prec IS
        {
			BooleanTest *b = makeNode(BooleanTest);
			b->arg = (Expr *) $1;
			b->booltesttype = IS_NOT_FALSE;
			$$ = (Node *)b;
        }
    | a_expr BETWEEN b_expr AND a_expr %prec BETWEEN
        {
            $$ = NULL;
        }
    | a_expr NOT_LA BETWEEN b_expr AND a_expr %prec NOT_LA
        {
            $$ = NULL;
        }
    ;

b_expr:
    c_expr      { $$ = $1; }
    | '+' b_expr %prec UMINUS
        {
            $$ = (Node*) makeA_Expr(AEXPR_OP, "+" , NULL, $2);
        }
    | '-' b_expr
        {
            $$ = (Node*) makeA_Expr(AEXPR_OP, "-" , NULL, $2);
        }
    | b_expr '+' b_expr
        {
            $$ = (Node*) makeA_Expr(AEXPR_OP, "+" , $1, $3);
        }
    | b_expr '-' b_expr
        {
            $$ = (Node*) makeA_Expr(AEXPR_OP, "-" , $1, $3);
        }
    | b_expr '*' b_expr
        {
            $$ = (Node*) makeA_Expr(AEXPR_OP, "*" , $1, $3);
        }
    | b_expr '/' b_expr
        {
            $$ = (Node*) makeA_Expr(AEXPR_OP, "/" , $1, $3);
        }
    | b_expr '<' b_expr
        {
            $$ = (Node*) makeA_Expr(AEXPR_OP, "<" , $1, $3);
        }
    | b_expr '>' b_expr
        {
            $$ = (Node*) makeA_Expr(AEXPR_OP, ">" , $1, $3);
        }
    | b_expr '=' b_expr
        {
            $$ = (Node*) makeA_Expr(AEXPR_OP, "=" , $1, $3);
        }
    | b_expr LESS_EQUALS b_expr
        {
            $$ = (Node*) makeA_Expr(AEXPR_OP, "<=" , $1, $3);
        }
    | b_expr GREATER_EQUALS b_expr
        {
             $$ = (Node*) makeA_Expr(AEXPR_OP, ">=" , $1, $3);
        }
    | b_expr NOT_EQUALS b_expr
        {
             $$ = (Node*) makeA_Expr(AEXPR_OP, "<>" , $1, $3);
        }
    ;

c_expr:
    AexprConst          { $$ = $1; }
    | columnref         { $$ = $1; }
    | '(' a_expr ')'    { $$ = $2; }
    ;

columnref:
    name
        {
            ColumnRef* ref = makeNode(ColumnRef);
            ref->fields = new List<Node>();
            ref->fields->Append((Node*) makeString($1));
            $$ = (Node*) ref;
        }
    ;

AexprConst:
    ICONST      { $$ = makeIntConst($1); }
    | SCONST    { $$ = makeStringConst($1); }
    | FCONST    { $$ = makeFloatConst($1); }
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
    a_expr AS name
        {
			$$ = (ResTarget*) makeNode(ResTarget);
			$$->name = $3;
			$$->val = (Node *) $1;
        }
    | a_expr IDENT
        {
  			$$ = (ResTarget*) makeNode(ResTarget);
  			$$->name = $2;
  			$$->val = (Node *) $1;
        }
    | a_expr
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
    WHERE a_expr	    { $$ = $2; }
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
    | ISNULL
    | TEXT
    ;
%%
namespace db
{

Node* makeIntConst(int i) {
  A_Const* n = makeNode(A_Const);
  n->val = makeInteger(i);
  return (Node*) n;
}

Node* makeStringConst(char* str) {
  A_Const* n = makeNode(A_Const);
  n->val = makeString(str);
  return (Node*) n;
}

Node* makeFloatConst(char* str) {
  A_Const* n = makeNode(A_Const);
  n->val = makeFloat(str);
  return (Node*) n;
}

}
