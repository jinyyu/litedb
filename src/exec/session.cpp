#include <litesql/exec/session.h>
#include <litesql/utils/memctx.h>
#include <litesql/utils/pq.h>
#include <litesql/utils/elog.h>
#include <litesql/parser/parser.h>
#include <litesql/parser/analyze.h>
#include <litesql/portal.h>

namespace db {

thread_local Session* CurSession = nullptr;

Session::Session(int fd, const char* peerAddr, u16 peerPort)
    : forceClose(false),
      fd(fd),
      port(peerPort) {
  strcpy(peer, peerAddr);
  fprintf(stderr, "new connection %s:%d\n", peer, port);
}

Session::~Session() {
  fprintf(stderr, "session closed %s:%d\n", peer, port);
  close(fd);
}

void Session::Loop() {
  int status;
  CurSession = this;
  MemoryContext::Init();
  status = ProcessStartupPacket();
  if (status != STATUS_OK) {
    goto cleanup;
  }

  status = ClientAuthentication();
  if (status != STATUS_OK) {
    goto cleanup;
  }

  while (!forceClose) {
    try {

      // Release storage left over from prior query cycle
      MemoryContext::SwitchTo(MessageContext);
      MessageContext->Reset();

      ReadyForQuery();

      u8 firstChar;
      u32 commandLen;

      //read a command (loop blocks here)
      if (PQ_GetBytes(fd, &firstChar, 1)) {
        elog(COMMERROR, "unexpected EOF on client connection");
        break;
      }

      if (PQ_GetBytes(fd, (u8*) &commandLen, 4)) {
        elog(COMMERROR, "unexpected EOF on client connection");
        break;
      }
      commandLen = be32toh(commandLen);
      if (commandLen < 4 || commandLen > 64 * 1024) {
        elog(COMMERROR, "invalid packet");
        break;
      }

      switch (firstChar) {
        case 'Q': {
          u32 sqlLen = commandLen - 4;
          char* query = (char*) Malloc(sqlLen + 1); // 额外多申请一字节
          if (PQ_GetBytes(fd, (u8*) query, sqlLen) != 0) {
            elog(COMMERROR, "unexpected EOF on client connection");
            goto cleanup;
          }
          size_t strLen = strlen(query);
          if (strLen > sqlLen) {
            elog(COMMERROR, "invalid sql len");
            goto cleanup;
          }
          query[sqlLen] = 0;
          ExecSimpleQuery(query, strLen);
          break;
        }
        case 'X': {
          forceClose = true;
          break;
        }
        default: {
          elog(ERROR, "invalid frontend message type %c", firstChar);
        }
      }
    } catch (Exception& e) {

      /* Make sure libpq is in a good state */
      sendBuffer.clear();
      PQ_Reset();

      EmitErrorReport();

      FlushErrorState();

      MemoryContext::SwitchTo(TopMemoryContext);

      if (e.level > ERROR) {
        forceClose = true;
        break;
      }
    }
  }

cleanup:
  MemoryContext::Release();
  this->sessionCloseCallback();
  CurSession = nullptr;
}

int Session::ProcessStartupPacket() {
  StartupPacket packet;
  int ret = PQ_GetBytes(fd, (u8*) &packet, sizeof(packet));
  if (ret == EOF) {
    elog(COMMERROR, "incomplete startup packet");
    return STATUS_ERROR;
  }
  u32 len = be32toh(packet.length);
  if (len < sizeof(packet) || len > MAX_STARTUP_PACKET_LENGTH) {
    elog(COMMERROR, "invalid length of startup packet");
    return STATUS_ERROR;
  }
  u32 paramLen = len - sizeof(packet);
  if (paramLen > 0) {
    char* data = (char*) Malloc(paramLen);
    char* ptr = data;
    ret = PQ_GetBytes(fd, (u8*) ptr, paramLen);

    if (ret == EOF) {
      elog(COMMERROR, "incomplete startup packet");
      return STATUS_ERROR;
    }

    while (paramLen > 0) {
      if (*ptr == '\0') {
        break;
      }

      size_t keyLen = strlen(ptr) + 1;
      if (keyLen > paramLen) {
        elog(COMMERROR, "invalid startup packet");
        return STATUS_ERROR;
      }
      char* key = ptr;
      ptr += keyLen;
      paramLen -= keyLen;

      size_t valueLen = strlen(ptr) + 1;
      if (valueLen > paramLen) {
        elog(COMMERROR, "invalid startup packet");
        return STATUS_ERROR;
      }
      char* value = ptr;
      ptr += valueLen;
      paramLen -= valueLen;

      if (strcmp(key, "database") == 0) {
        database = value;
        database = ":memory:";
        elog(DEBUG, "database:%s", value);
      } else if (strcmp(key, "user") == 0) {
        user = value;
        elog(DEBUG, "user:%s", value);
      } else if (strcmp(key, "client_encoding") == 0) {
        client_encoding = value;
        elog(DEBUG, "client_encoding:%s", value);
      }
    }

    Free(data);
  }
  return STATUS_OK;
}

int Session::ClientAuthentication() {
  sendBuffer.clear();
  MemoryContext* old = MemoryContext::SwitchTo(TopTransactionContext);

  {
    //Authentication request
    u32 authReq = AUTH_REQ_OK;
    PQMessage* msg = MakePQMessage('R', sizeof(authReq));
    msg->PutU32(authReq, 0);
    PQ_Append(sendBuffer, msg);
  }

  {
    const char* name = "server_version";
    size_t nameLen = strlen(name) + 1;
    const char* value = "liteSQL";
    size_t valueLen = strlen(value) + 1;
    PQMessage* msg = MakePQMessage('S', nameLen + valueLen);
    u32 offset = 0;
    offset = msg->PutData(offset, (u8*) name, nameLen);
    msg->PutData(offset, (u8*) value, valueLen);
    PQ_Append(sendBuffer, msg);
  }
  {
    const char* name = "client_encoding";
    size_t nameLen = strlen(name) + 1;
    const char* value = client_encoding.c_str();
    size_t valueLen = strlen(value) + 1;
    PQMessage* msg = MakePQMessage('S', nameLen + valueLen);
    u32 offset = 0;
    offset = msg->PutData(offset, (u8*) name, nameLen);
    msg->PutData(offset, (u8*) value, valueLen);
    PQ_Append(sendBuffer, msg);
  }

  int status = PQ_Flush(fd, sendBuffer);
  sendBuffer.clear();
  if (status != 0) {
    status = STATUS_ERROR;
  } else {
    status = STATUS_OK;
  }

  MemoryContext::SwitchTo(old);
  return status;
}

void Session::FlushErrorState() {
  ErrorContext->Reset();
}

void Session::ReadyForQuery() {
  MemoryContext* old = MemoryContext::SwitchTo(MessageContext);
  PQMessage* msg = MakePQMessage('Z', 1);
  msg->PutU8(0, 'I');
  PQ_Append(sendBuffer, msg);
  if (PQ_Flush(fd, sendBuffer) != 0) {
    elog(FATAL, "send msg error");
  };
  sendBuffer.clear();
  MemoryContext::SwitchTo(old);
}

void Session::SendCommand(char c, const char* command, int len) {
  MemoryContext* old = MemoryContext::SwitchTo(MessageContext);
  PQMessage* msg = MakePQMessage(c, len);
  msg->PutData(0, (u8*) command, len);
  PQ_Append(sendBuffer, msg);
  MemoryContext::SwitchTo(old);
}

void Session::ExecSimpleQuery(char* queryString, size_t queryLen) {
  /*
   * Switch to appropriate context for constructing parsetrees.
   */
  MemoryContext* old = MemoryContext::SwitchTo(MessageContext);

  NodeList* list = Parser::Parse(queryString, queryLen);

  /*
   * Switch back to transaction context to enter the loop.
   */
  MemoryContext::SwitchTo(old);
  if (list) {
    for (Node* parseTree : list->nodes) {
      Query* query = ParseAnalyze(parseTree, queryString);
      SendCommand('C', "ok", strlen("ok") + 1);
    }
  }
}

}
