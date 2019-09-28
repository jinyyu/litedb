#ifndef LITESQL_LITESQL_PQ_H_
#define LITESQL_LITESQL_PQ_H_
#include <litesql/int.h>
#include <stdio.h>

namespace db {

#pragma pack(push, 1)

struct StartupPacket {
  u32 length;
  u16 majorVersion;
  u16 minorVersion;
  char Parameters[0];
};
#pragma pack(pop)

#define MAX_STARTUP_PACKET_LENGTH 10000


/* --------------------------------
 *		PQ_GetBytes		- get a known number of bytes from connection
 *
 *		returns 0 if OK, EOF if trouble
 * --------------------------------
 */
int PQ_GetBytes(int socket, u8* buf, size_t len);

}

#endif //LITESQL_LITESQL_PQ_H_
