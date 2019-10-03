#include <litesql/parser.h>
#include <litesql/elog.h>
#include <string.h>
#include "gram.hpp"

extern int scanner_lex(PARSER_STYPE* yylval, PARSER_LTYPE* yylloc_param, void* yyscanner);

namespace db {

extern void ScannerInit(Scanner* scanner, char* query, size_t queryLen);
extern void ScannerFinish(Scanner* scanner);

Parser::Parser(char* query, size_t queryLen)
    : Object(CurTransactionContext) {
  ScannerInit(&scanner, query, queryLen);
}

Parser::~Parser() {
  ScannerFinish(&scanner);
}

void Parser::Parse(char* query, size_t queryLen, std::list<RawStmt*>* list) {
  Parser* parser = new Parser(query, queryLen);
  parser->list = list;
  int result = parser_parse(parser);
  ScannerFinish(&parser->scanner);
  if (result) {
    //error
    list->clear();
  }
}

int parser_lex(PARSER_STYPE* yylval, PARSER_LTYPE* yylloc, Parser* parser) {
  int token = scanner_lex(yylval, yylloc, parser->scanner.flex);
  return token;

}

int parser_error(PARSER_LTYPE* yylloc, Parser* parser, const char* msg) {
  eReport(DEBUG, "--------------------error %d, %s", *yylloc, msg);
  return 0;
}

}

