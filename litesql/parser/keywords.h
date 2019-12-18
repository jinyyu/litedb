#ifndef LITESQL_SRC_PARSER_KEYWORDS_H_
#define LITESQL_SRC_PARSER_KEYWORDS_H_
#include <litesql/int.h>
namespace db {

/* Keyword categories --- should match lists in gram.y */
#define UNRESERVED_KEYWORD        0
#define COL_NAME_KEYWORD          1
#define TYPE_FUNC_NAME_KEYWORD    2
#define RESERVED_KEYWORD          3

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
#endif //LITESQL_SRC_PARSER_KEYWORDS_H_
