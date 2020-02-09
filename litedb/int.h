#ifndef LITEDB_INT_H_
#define LITEDB_INT_H_
#include <stdint.h>
#include <endian.h>

namespace db {

typedef int8_t i8;             /* 1-byte signed integer */
typedef uint8_t u8;            /* 1-byte unsigned integer */
typedef int16_t i16;           /* 2-byte signed integer */
typedef uint16_t u16;           /* 2-byte unsigned integer */
typedef int32_t i32;           /* 4-byte signed integer */
typedef uint32_t u32;          /* 4-byte unsigned integer */
typedef int64_t i64;           /* 8-byte signed integer */
typedef uint64_t u64;          /* 8-byte unsigned integer */


#define STATUS_OK           0   /* Successful result */
#define STATUS_ERROR        1   /* SQL error or missing database */
#define STATUS_INTERNAL     2   /* An internal logic error*/
#define STATUS_PERM         3   /* Access permission denied */
#define STATUS_ABORT        4   /* Callback routine requested an abort */
#define STATUS_BUSY         5   /* The database file is locked */
#define STATUS_LOCKED       6   /* A table in the database is locked */
#define STATUS_NOMEM        7   /* A malloc() failed */
#define STATUS_READONLY     8   /* Attempt to write a readonly database */
#define STATUS_INTERRUPT    9   /* Operation terminated*/
#define STATUS_IOERR       10   /* Some kind of disk I/O error occurred */
#define STATUS_CORRUPT     11   /* The database disk image is malformed */
#define STATUS_NOTFOUND    12   /* (Internal Only) Table or record not found */
#define STATUS_FULL        13   /* Insertion failed because database is full */
#define STATUS_CANTOPEN    14   /* Unable to open the database file */
#define STATUS_PROTOCOL    15   /* Database lock protocol error */
#define STATUS_EMPTY       16   /* (Internal Only) Database table is empty */
#define STATUS_SCHEMA      17   /* The database schema changed */
#define STATUS_TOOBIG      18   /* Too much data for one row of a table */
#define STATUS_CONSTRAINT  19   /* Abort due to contraint violation */
#define STATUS_MISMATCH    20   /* Data type mismatch */
#define STATUS_MISUSE      21   /* Library used incorrectly */
#define STATUS_NOLFS       22   /* Uses OS features not supported on host */
#define STATUS_AUTH        23   /* Authorization denied */
#define STATUS_FORMAT      24   /* Auxiliary database format error */
#define STATUS_RANGE       25   /* 2nd parameter to bind out of range */
#define STATUS_NOTADB      26   /* File opened that is not a database file */
#define STATUS_ROW         100  /* step() has another row ready */
#define STATUS_DONE        101  /* step() has finished executing */

#define NAMEDATALEN 64

#define INDEX_MAX_KEYS 32

#define FLEXIBLE_ARRAY_MEMBER 256

struct NameData {
  char data[NAMEDATALEN];
};

}
#endif //LITEDB_INT_H_
