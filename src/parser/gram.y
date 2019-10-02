%{
#include <litesql/parser.h>
#define YYERROR_VERBOSE
using namespace db;


%}

%union
{

}

%name-prefix="parser_"
%token GET_P
%token IDENT_P

%type <stmt>	stmtblock stmt GetStmt

/* Grammar follows */
%%

/*
 *	The target production for the whole parse.
 */
stmtblock:               {
              };

%%

void help() {
}