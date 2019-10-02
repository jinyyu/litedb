#ifndef LITESQL_LITESQL_LITESQLINT_H_
#define LITESQL_LITESQL_LITESQLINT_H_
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


#define STATUS_OK               (0)
#define STATUS_ERROR            (-1)
#define STATUS_EOF              (-2)
#define STATUS_FOUND            (1)
#define STATUS_WAITING          (2)

#define NAMEDATALEN 64

}
#endif //LITESQL_LITESQL_LITESQLINT_H_
