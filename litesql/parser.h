#ifndef LITESQL_LITESQL_PARSER_H_
#define LITESQL_LITESQL_PARSER_H_
#include <litesql/int.h>
#include <litesql/mcxt.h>
#include <litesql/nodes.h>
#include <list>
#include <vector>

#define PARSER_LTYPE int
union PARSER_STYPE;

namespace db {

struct Scanner {
  char* scanBuf;
  size_t scanBufLen;
  void* flex;
  int xcdepth;           //depth of nesting in slash-star comments

  std::vector<char> literalBuf;   //literalbuf is used to accumulate literal values when multiple rules are needed to parse a single literal
};

struct Parser : Object {
  ~Parser() final;
  static void Parse(char* query, size_t queryLen, std::list<RawStmt*>* list);

  Scanner scanner;
  std::list<RawStmt*>* list;
private:
  explicit Parser(char* query, size_t queryLen);
};

int parser_lex(PARSER_STYPE* yylval, PARSER_LTYPE* yylloc, Parser* parser);
int parser_error(PARSER_LTYPE* yylloc, Parser* parser, const char* msg);

}
#endif //LITESQL_LITESQL_PARSER_H_
