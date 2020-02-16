#ifndef LITEDB_PARSER_H_
#define LITEDB_PARSER_H_
#include <litedb/int.h>
#include <litedb/utils/env.h>
#include <litedb/nodes/value.h>
#include <litedb/nodes/parsenodes.h>
#include <litedb/utils/list.h>
#include <vector>

union PARSER_STYPE;
struct PARSER_LTYPE;

namespace db {

struct Scanner {
  char* scanBuf;
  size_t scanBufLen;
  void* flex;
  int xcdepth;           //depth of nesting in slash-star comments

  std::vector<char> literalBuf;   //literalbuf is used to accumulate literal
  // values when multiple rules are needed to parse a single literal
};

struct Parser : Object {
  ~Parser() final;
  static List<Node>* Parse(char* query, size_t queryLen);

  Scanner scanner;
  List<Node>* nodes;
 private:
  explicit Parser(char* query, size_t queryLen);
};

int parser_lex(PARSER_STYPE* yylval, PARSER_LTYPE* yylloc, Parser* parser);
void parser_error(PARSER_LTYPE* yylloc, Parser* parser, const char* msg);

}
#endif
