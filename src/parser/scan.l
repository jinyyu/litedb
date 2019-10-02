%{

%}

%option reentrant
%option 8bit
%option never-interactive
%option nodefault
%option noinput
%option nounput
%option noyywrap
%option noyyalloc
%option noyyrealloc
%option noyyfree
%option warn
%option prefix="scaner_"

space         [ \t]
whitespace		{space}+
digit			    [0-9]
integer       {digit}+
ident_start   [A-Za-z\200-\377_]
ident_cont    [A-Za-z\200-\377_0-9\$]

identifier    {ident_start}{ident_cont}*
%%

{integer}	   {
               fprintf(stderr, "integer");
             }

{whitespace} { /* ignore */ }

{identifier} {

             }

<<EOF>>      { yyterminate(); }

%%

void help() {
}
