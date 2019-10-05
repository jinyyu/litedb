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
    void*  stmt;
}

%token GET_P
%token IDENT_P

%token <str>    FCONST BCONST IDENT SCONST XCONST Op
%token <ival>	ICONST PARAM
%token	EQUALS_GREATER LESS_EQUALS GREATER_EQUALS NOT_EQUALS

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

GetStmt: GET_P GET_P
             {
               $$ = NULL;
             }
  ;
%%

void help() {

}