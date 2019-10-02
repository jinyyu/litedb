#ifndef LITESQL_LITESQL_PARSER_H_
#define LITESQL_LITESQL_PARSER_H_
#include <litesql/int.h>
namespace db {

int parser_parse();
int parser_lex();
void parser_error(const char* msg);

struct Parser {

};

}
#endif //LITESQL_LITESQL_PARSER_H_
