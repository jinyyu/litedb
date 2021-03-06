#ifndef LITEDB_PARSER_KEYWORDS_H_
#define LITEDB_PARSER_KEYWORDS_H_
#include <litedb/int.h>
namespace db {

/* Keyword categories --- should match lists in gram.y */
#define UNRESERVED_KEYWORD        0
#define RESERVED_KEYWORD          1

struct ScanKeyword {
  const char* name;      /* in lower case */
  u16 value;             /* grammar's token code */
  u16 category;          /* see codes above */
};

void InitKeyword();

/*
 * ScanKeywordLookup - see if a given word is a keyword
 */
const ScanKeyword* ScanKeywordLookup(const char* text);

}
#endif //LITEDB_PARSER_KEYWORDS_H_
