#ifndef LITESQL_LITESQL_PARSER_H_
#define LITESQL_LITESQL_PARSER_H_
#include <litesql/int.h>
#include <litesql/mcxt.h>
#include <litesql/nodes.h>
#include <list>

namespace db {

int parser_parse();
int parser_lex();
void parser_error(const char* msg);

struct Parser : Object {
  static void Parse(const char* query, std::list<RawStmt*>* list);

private:
  explicit Parser(const char* query);
};
}
#endif //LITESQL_LITESQL_PARSER_H_
