#include <litedb/parser/keywords.h>
#include <litedb/parser/parser.h>
#include <string.h>
#include <stdlib.h>
#include "gram.hpp"

namespace db {

static ScanKeyword ScanKeywords[] = {
    {"abort", ABORT_P, UNRESERVED_KEYWORD},
    {"abort", ABORT_P, UNRESERVED_KEYWORD},
    {"all", ALL, RESERVED_KEYWORD},
    {"and", AND, RESERVED_KEYWORD},
    {"as", AS, RESERVED_KEYWORD},
    {"asc", ASC, RESERVED_KEYWORD},
    {"autoincrement", AUTOINCREMENT, RESERVED_KEYWORD},
    {"between", BETWEEN, RESERVED_KEYWORD},
    {"check", CHECK, RESERVED_KEYWORD},
    {"conflict", CONFLICT, UNRESERVED_KEYWORD},
    {"constraint", CONSTRAINT, RESERVED_KEYWORD},
    {"create", CREATE, RESERVED_KEYWORD},
    {"default", DEFAULT, RESERVED_KEYWORD},
    {"desc", DESC, RESERVED_KEYWORD},
    {"distinct", DISTINCT , RESERVED_KEYWORD},
    {"exists", EXISTS, UNRESERVED_KEYWORD},
    {"false", FALSE_P, RESERVED_KEYWORD},
    {"from", FROM, RESERVED_KEYWORD},
    {"if", IF_P, UNRESERVED_KEYWORD},
    {"int", INT_P, RESERVED_KEYWORD},
    {"integer", INTEGER, RESERVED_KEYWORD},
    {"is", IS, RESERVED_KEYWORD},
    {"isnull", ISNULL, UNRESERVED_KEYWORD},
    {"ignore", IGNORE, UNRESERVED_KEYWORD},
    {"not", NOT, RESERVED_KEYWORD},
    {"null", NULL_P, RESERVED_KEYWORD},
    {"key", KEY, UNRESERVED_KEYWORD},
    {"on", ON_P, UNRESERVED_KEYWORD},
    {"or", OR, RESERVED_KEYWORD},
    {"primary", PRIMARY, RESERVED_KEYWORD},
    {"replace", REPLACE, UNRESERVED_KEYWORD},
    {"rollback", ROLLBACK, UNRESERVED_KEYWORD},
    {"select", SELECT, RESERVED_KEYWORD},
    {"table", TABLE, RESERVED_KEYWORD},
    {"temp", TEMP, UNRESERVED_KEYWORD},
    {"temporary", TEMPORARY, UNRESERVED_KEYWORD},
    {"text", TEXT, UNRESERVED_KEYWORD},
    {"true", TRUE_P, RESERVED_KEYWORD},
    {"unique", UNIQUE, RESERVED_KEYWORD},
    {"varchar", VARCHAR, RESERVED_KEYWORD},
    {"where", WHERE, RESERVED_KEYWORD},
};
static const int NumScanKeywords = sizeof(ScanKeywords) / sizeof(ScanKeyword);

static int keyCompare(const void* ap, const void* bp) {
  const ScanKeyword* a = (ScanKeyword*) ap;
  const ScanKeyword* b = (ScanKeyword*) bp;
  return strcmp(a->name, b->name);
}

void InitKeyword() {
  qsort(ScanKeywords, NumScanKeywords, sizeof(ScanKeyword), keyCompare);
}

const ScanKeyword* ScanKeywordLookup(const char* text) {
  char word[NAMEDATALEN];
  size_t len = strlen(text);
  if (len > NAMEDATALEN) {
    return nullptr;
  }

  for (size_t i = 0; i < len; i++) {
    char ch = text[i];

    if (ch >= 'A' && ch <= 'Z')
      ch += 'a' - 'A';
    word[i] = ch;
  }
  word[len] = '\0';
  ScanKeyword key{
      .name = word,
  };
  return (const ScanKeyword*) bsearch(&key, ScanKeywords, NumScanKeywords, sizeof(ScanKeyword), keyCompare);
}

}