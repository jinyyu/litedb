#include <litedb/parser/parser.h>
#include <litedb/utils/elog.h>
#include <litedb/utils/list.h>
#include <string.h>
#include "gram.hpp"

extern int scanner_lex(PARSER_STYPE* yylval, PARSER_LTYPE* yylloc_param, void* yyscanner);

namespace db {

extern void ScannerInit(Scanner* scanner, char* query, size_t queryLen);
extern void ScannerFinish(Scanner* scanner);
extern int ScannerCurrentPosition(Scanner* scanner, char* token, size_t tokenSize);

Parser::Parser(char* query, size_t queryLen)
    : Object(),
      nodes(nullptr) {
  ScannerInit(&scanner, query, queryLen);
}

Parser::~Parser() {
  ScannerFinish(&scanner);
}

List<Node>* Parser::Parse(char* query, size_t queryLen) {
  Parser* parser = new Parser(query, queryLen);
  int result = parser_parse(parser);
  ScannerFinish(&parser->scanner);
  if (result) {
    //error
    return nullptr;
  }
  return parser->nodes;
}

int parser_lex(PARSER_STYPE* yylval, PARSER_LTYPE* yylloc, Parser* parser) {
  int token = scanner_lex(yylval, yylloc, parser->scanner.flex);
  return token;

}

void parser_error(PARSER_LTYPE*, Parser* parser, const char* msg) {
  char token[16];
  int position = ScannerCurrentPosition(&parser->scanner, token, sizeof(token));
  if (position == 0) {
    elog(ERROR, "%s at end of input", msg);
  } else {
    elogLocation(ERROR, position, "%s at or near \"%s\"", msg, token);
  }
}

}

