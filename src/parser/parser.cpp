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

List* Parser::Parse(char* query, size_t queryLen) {
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
  int cur_token;
  int next_token;
  Scanner& scanner = parser->scanner;

  /* Get next token --- we might already have it */
  if (scanner.have_lookahead) {
    cur_token = scanner.lookahead_token;
    scanner.have_lookahead = false;
  } else {
    cur_token = scanner_lex(yylval, yylloc, parser->scanner.flex);
  }

  /*
   * If this token isn't one that requires lookahead, just return it.  If it
   * does, determine the token length.  (We could get that via strlen(), but
   * since we have such a small set of possibilities, hardwiring seems
   * feasible and more efficient.)
   */
  switch (cur_token) {
    case NOT: {
      break;
    }
    default: {
      return cur_token;
    }
  }

  next_token = scanner_lex(yylval, yylloc, parser->scanner.flex);
  scanner.lookahead_token = next_token;
  scanner.have_lookahead = true;

  switch (cur_token)
  {
    case NOT:
      /* Replace NOT by NOT_LA if it's followed by BETWEEN, IN, etc */
      switch (next_token)
      {
        case BETWEEN:
        case IN_P:
          cur_token = NOT_LA;
          break;
      }
      break;
      break;
  }

  return cur_token;
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

