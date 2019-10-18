%{
#include <litesql/parser.h>
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
    char*          str;
    const char     *keyword;
    std::list<db::RawStmt*>*  list;
}

%token GET_P
%token IDENT_P

%token <str>    FCONST BCONST IDENT SCONST XCONST OP
%token <ival>	ICONST PARAM
%token  EQUALS_GREATER LESS_EQUALS GREATER_EQUALS NOT_EQUALS

%type <list>    stmtblock stmtmulti

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

/* Grammar follows */
%%

/*
 *	The target production for the whole parse.
 */
stmtblock:  stmtmulti
              {
                $$ = $1;
              };

stmtmulti:	stmtmulti ';' stmt
				{

				}
			| stmt
				{

				}
		;

stmt: AND
              {

              };
%%

void help() {

}