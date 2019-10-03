#ifndef LITESQL_LITESQL_PARSER_H_
#define LITESQL_LITESQL_PARSER_H_
#include <litesql/int.h>
#include <litesql/mcxt.h>
#include <litesql/nodes.h>
#include <list>

#define PARSER_LTYPE int
union PARSER_STYPE;

namespace db {

struct Scanner {
  char* scanBuf;
  size_t scanBufLen;
  void* flex;
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
