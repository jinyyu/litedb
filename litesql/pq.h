#ifndef LITESQL_LITESQL_PQ_H_
#define LITESQL_LITESQL_PQ_H_
#include <litesql/int.h>
#include <stdio.h>

namespace db {

#define PG_DIAG_SEVERITY              'S'
#define PG_DIAG_SEVERITY_NONLOCALIZED 'V'
#define PG_DIAG_SQLSTATE              'C'
#define PG_DIAG_MESSAGE_PRIMARY       'M'
#define PG_DIAG_MESSAGE_DETAIL        'D'
#define PG_DIAG_MESSAGE_HINT          'H'
#define PG_DIAG_STATEMENT_POSITION    'P'
#define PG_DIAG_INTERNAL_POSITION     'p'
#define PG_DIAG_INTERNAL_QUERY        'q'
#define PG_DIAG_CONTEXT               'W'
#define PG_DIAG_SCHEMA_NAME           's'
#define PG_DIAG_TABLE_NAME            't'
#define PG_DIAG_COLUMN_NAME           'c'
#define PG_DIAG_DATATYPE_NAME         'd'
#define PG_DIAG_CONSTRAINT_NAME       'n'
#define PG_DIAG_SOURCE_FILE           'F'
#define PG_DIAG_SOURCE_LINE           'L'
#define PG_DIAG_SOURCE_FUNCTION       'R'

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

/* --------------------------------
 *		PQ_BeginMessage		- initialize for sending a message
 * --------------------------------
 */
void PQ_BeginMessage(char msgType);

/* --------------------------------
 *		PQ_SendU8	- append raw data to  buffer
 * --------------------------------
 */
void PQ_SendU8(u8 i);

/* --------------------------------
 *		PQ_SendString	- append a null-terminated text string
 * --------------------------------
 */
void PQ_SendString(const char* str);

/* --------------------------------
 *		PQ_EndMessage	- send the completed message to the frontend
 * --------------------------------
 */
void PQ_EndMessage();

/* --------------------------------
 *		PQ_Flush		- flush data to frontend
 *
 *		returns 0 if OK, EOF if trouble
 * --------------------------------
 */
int PQ_Flush(int fd);

}

#endif //LITESQL_LITESQL_PQ_H_
