#ifndef LITESQL_LITESQL_PARSER_H_
#define LITESQL_LITESQL_PARSER_H_
#include <litesql/int.h>
#include <litesql/mcxt.h>
#include <litesql/nodes.h>
#include <list>

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

}
#endif //LITESQL_LITESQL_PARSER_H_
