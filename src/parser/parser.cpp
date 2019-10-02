#include <litesql/parser.h>
#include <litesql/elog.h>
#include <string.h>
#include "gram.hpp"

namespace db {

extern void ScannerInit(Scanner* scanner, char* query, size_t queryLen);
extern void ScannerFinish(Scanner* scanner);
extern int ScannerPosition(void* fex);

Parser::Parser(char* query, size_t queryLen) {
  ScannerInit(&scanner, query, queryLen);
}

Parser::~Parser() {
  ScannerFinish(&scanner);
}

void Parser::Parse(char* query, size_t queryLen, std::list<RawStmt*>* list) {
  Parser* parser = new Parser(query, queryLen);
  parser->list = list;
}

}

