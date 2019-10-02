%{
#include <litesql/parser.h>
#include "gram.hpp"
#define YYERROR_VERBOSE
using namespace db;

static int parser_lex(PARSER_STYPE* yylval, PARSER_LTYPE* yylloc, Parser* parser);
static int parser_error(PARSER_LTYPE* yylloc, Parser* parser, const char* msg);

%}

%pure-parser
%expect 0
%define api.prefix {parser_}
%locations

%parse-param {db::Parser* parser}
%lex-param   {db::Parser* parser}

%union
{
    int ival;
}

%token GET_P
%token IDENT_P

%type <ival>	stmtblock

/* Grammar follows */
%%

/*
 *	The target production for the whole parse.
 */
stmtblock:    {
              };

%%

int parser_lex(PARSER_STYPE* yylval, PARSER_LTYPE* yylloc, Parser* parser) {

}

int parser_error(PARSER_LTYPE* yylloc, Parser* parser, const char* msg) {

}