%{
#include <litesql/int.h>
#include <litesql/parser.h>
#include <litesql/mcxt.h>
#include <litesql/elog.h>
#include "gram.hpp"

using namespace db;
typedef PARSER_STYPE YYSTYPE;
typedef PARSER_LTYPE YYLTYPE;
%}

%option reentrant
%option bison-bridge
%option bison-locations
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
%option prefix="scanner_"

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

void *scanner_alloc (yy_size_t size,yyscan_t yyscanner) {
    if (size >0) {
        return Malloc(size);
    } else {
        return NULL;
    }
}

void scanner_free(void *ptr, yyscan_t yyscanner) {
    if (ptr) {
        Free(ptr);
    }
}

void *scanner_realloc (void* ptr, yy_size_t size, yyscan_t yyscanner) {
    if (ptr) {
        void* ret = Malloc(size);
        memcpy(ret, ptr, size);
        return ret;
    } else {
        return NULL;
    }
}

namespace db
{

void ScannerInit(Scanner* scanner, char* query, size_t queryLen) {
    void* flex;
    if (scanner_lex_init(&flex) != 0 ) {
         eReport(ERROR, "scanner_lex_init failed");
    }
    scanner_set_extra(scanner, flex);
    scanner->scanBufLen = queryLen;
    scanner->scanBuf[queryLen] = scanner->scanBuf[queryLen + 1] = YY_END_OF_BUFFER_CHAR;
    scanner->flex = flex;

    scanner__scan_buffer(scanner->scanBuf, queryLen + 2, flex);
}

void ScannerFinish(Scanner* scanner) {

}

int ScannerPosition(yyscan_t yyscanner) {

}

}
