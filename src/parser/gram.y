%{
#include <litesql/parser.h>
#include "gram.hpp"
#define YYERROR_VERBOSE
using namespace db;

/*
 * Location tracking support --- simpler than bison's default, since we only
 * want to track the start position not the end position of each nonterminal.
 */
#define YYLLOC_DEFAULT(Current, Rhs, N) \
	do { \
		if ((N) > 0) \
			(Current) = (Rhs)[1]; \
		else \
			(Current) = (-1); \
	} while (0)

%}

%pure-parser
%expect 0
%define api.prefix {parser_}
%locations

%parse-param {db::Parser* parser}
%lex-param   {db::Parser* parser}

%union
{
  void* stmt;
}

%token GET_P
%token IDENT_P

%type <stmt>	stmtblock stmt GetStmt

/* Grammar follows */
%%

/*
 *	The target production for the whole parse.
 */
stmtblock:  stmt
              {
                yylval.stmt = $1;
              };

stmt :
			GetStmt
			| /*EMPTY*/
				{ $$ = NULL; }
		;

GetStmt: IDENT_P
             {
               $$ = NULL;
             }
  ;
%%

void help() {

}