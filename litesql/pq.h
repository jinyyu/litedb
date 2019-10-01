#ifndef LITESQL_LITESQL_PQ_H_
#define LITESQL_LITESQL_PQ_H_
#include <litesql/int.h>
#include <stdio.h>
#include <vector>

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

#define AUTH_REQ_OK          0    /* User is authenticated  */
#define AUTH_REQ_KRB4        1    /* Kerberos V4. Not supported any more. */
#define AUTH_REQ_KRB5        2    /* Kerberos V5. Not supported any more. */
#define AUTH_REQ_PASSWORD    3    /* Password */
#define AUTH_REQ_CRYPT       4    /* crypt password. Not supported any more. */
#define AUTH_REQ_MD5         5    /* md5 password */
#define AUTH_REQ_SCM_CREDS   6    /* transfer SCM credentials */
#define AUTH_REQ_GSS         7    /* GSSAPI without wrap() */
#define AUTH_REQ_GSS_CONT    8    /* Continue GSS exchanges */
#define AUTH_REQ_SSPI        9    /* SSPI negotiate without wrap() */
#define AUTH_REQ_SASL       10    /* Begin SASL authentication */
#define AUTH_REQ_SASL_CONT  11    /* Continue SASL authentication */
#define AUTH_REQ_SASL_FIN   12    /* Final SASL message */

#pragma pack(push, 1)

struct StartupPacket {
  u32 length;
  u16 majorVersion;
  u16 minorVersion;
  char Parameters[0];
};

struct PQMessage {
  u8 type;
  u32 len;     //消息长度, data长度+4，大端
  u8 data[0];

  // 往data写入数据, 返回写入后偏移
  u32 PutU8(u32 offset, u8 i);
  u32 PutU16(u32 offset, u16 i);
  u32 PutU32(u32 offset, u32 i);
  u32 PutU64(u32 offset, u64 i);
  u32 PutData(u32 offset, u8* dataPtr, u32 dataLen);
};
#pragma pack(pop)

PQMessage* MakePQMessage(u8 type, size_t dataLen);

#define MAX_STARTUP_PACKET_LENGTH 10000

void PQ_Reset();

/* --------------------------------
 *		PQ_GetBytes		- get a known number of bytes from connection
 *
 *		returns 0 if OK, EOF if trouble
 * --------------------------------
 */
int PQ_GetBytes(int socket, u8* buf, size_t len);

/* --------------------------------
 *		PQ_Append	- append a message to buffer
 * --------------------------------
 */
void PQ_Append(std::vector<u8>& buffer, PQMessage* msg);

/* --------------------------------
 *		PQ_Flush	- flush socket
 *
 *		returns 0 if OK, EOF if trouble
 * --------------------------------
 */
int PQ_Flush(int fd, const std::vector<u8>& buffer);

}

#endif //LITESQL_LITESQL_PQ_H_
