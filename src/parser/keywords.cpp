#include <litesql/keywords.h>
#include <string.h>
#include <stdlib.h>

namespace db {

static ScanKeyword ScanKeywords[] = {
    {}
};
static const int NumScanKeywords = sizeof(ScanKeywords);

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