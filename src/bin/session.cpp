#include <litedb/bin/session.h>
#include <litedb/utils/pq.h>
#include <litedb/utils/elog.h>
#include <litedb/parser/parser.h>
#include <litedb/parser/analyze.h>
#include <litedb/utils/portal.h>
#include <litedb/plan/planner.h>
#include <litedb//utils/env.h>
#include <litedb/catalog/catalog.h>
#include <assert.h>

namespace db {

thread_local Session* CurSession = nullptr;

static List* AnalyzeAndRewrite(Node* parseTrees, const char* src);
static List* PlanQueries(List* queryTrees);

List* AnalyzeAndRewrite(Node* parseTrees, const char* src) {
  Query* query = ParseAnalyze(parseTrees, src);

  DisplayParseNode((Node*) query);

  List* ret = NULL;
  ret = lappend(ret, (Node*) query);
  return ret;
}

List* PlanQueries(List* queryTrees) {
  List* ret = NULL;
  ListCell* cell;

  foreach(cell, queryTrees) {
    Query* query = (Query*) lfirst(cell);
    assert(query->type == T_Query);

    PlannedStmt* stmt;
    if (query->commandType == CMD_CMD_UTILITY) {

      stmt = makeNode(PlannedStmt);
      stmt->commandType = CMD_CMD_UTILITY;
      stmt->utilityStmt = query->utilityStmt;
    } else {
      stmt = Planner(query);
    }
    if (stmt) {
      ret = lappend(ret, (Node*) stmt);
    }
  }
  return ret;
}

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

  SessionEnv = std::make_shared<Environment>();

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
      if (CurrentTransaction) {
        CurrentTransaction->Abort();
        CurrentTransaction = nullptr;
      }

      SessionEnv = std::make_shared<Environment>();

      ReadyForQuery();

      u8 firstChar;
      u32 commandLen;

      //read a command (loop blocks here)
      if (PQ_GetBytes(fd, &firstChar, 1)) {
        elog(BACKEND, "unexpected EOF on client connection");
        break;
      }

      if (PQ_GetBytes(fd, (u8*) &commandLen, 4)) {
        elog(BACKEND, "unexpected EOF on client connection");
        break;
      }
      commandLen = be32toh(commandLen);
      if (commandLen < 4 || commandLen > 64 * 1024) {
        elog(BACKEND, "invalid packet");
        break;
      }

      switch (firstChar) {
        case 'Q': {
          u32 sqlLen = commandLen - 4;
          char* query = (char*) SessionEnv->Malloc(sqlLen + 1); // 额外多申请一字节
          if (PQ_GetBytes(fd, (u8*) query, sqlLen) != 0) {
            elog(BACKEND, "unexpected EOF on client connection");
            goto cleanup;
          }
          size_t strLen = strlen(query);
          if (strLen > sqlLen) {
            elog(BACKEND, "invalid sql len");
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
    } catch (LevelException& e) {

      /* Make sure libpq is in a good state */
      sendBuffer.clear();
      PQ_Reset();

      EmitErrorReport();

      FlushErrorState();
      if (e.level > ERROR) {
        forceClose = true;
        break;
      }

      if (CurrentTransaction) {
        CurrentTransaction->Abort();
        CurrentTransaction = nullptr;
      }
    }
  }

cleanup:
  SessionEnv = nullptr;
  this->sessionCloseCallback();
  CurSession = nullptr;
}

int Session::ProcessStartupPacket() {
  StartupPacket packet;
  int ret = PQ_GetBytes(fd, (u8*) &packet, sizeof(packet));
  if (ret == EOF) {
    elog(BACKEND, "incomplete startup packet");
    return STATUS_ERROR;
  }
  u32 len = be32toh(packet.length);
  if (len < sizeof(packet) || len > MAX_STARTUP_PACKET_LENGTH) {
    elog(BACKEND, "invalid length of startup packet");
    return STATUS_ERROR;
  }
  u32 paramLen = len - sizeof(packet);
  if (paramLen > 0) {
    char* data = (char*) SessionEnv->Malloc(paramLen);
    char* ptr = data;
    ret = PQ_GetBytes(fd, (u8*) ptr, paramLen);

    if (ret == EOF) {
      elog(BACKEND, "incomplete startup packet");
      return STATUS_ERROR;
    }

    while (paramLen > 0) {
      if (*ptr == '\0') {
        break;
      }

      size_t keyLen = strlen(ptr) + 1;
      if (keyLen > paramLen) {
        elog(BACKEND, "invalid startup packet");
        return STATUS_ERROR;
      }
      char* key = ptr;
      ptr += keyLen;
      paramLen -= keyLen;

      size_t valueLen = strlen(ptr) + 1;
      if (valueLen > paramLen) {
        elog(BACKEND, "invalid startup packet");
        return STATUS_ERROR;
      }
      char* value = ptr;
      ptr += valueLen;
      paramLen -= valueLen;

      if (strcmp(key, "database") == 0) {
        database = value;
        database = "litedb";
        elog(DEBUG, "database:%s", value);
      } else if (strcmp(key, "user") == 0) {
        user = value;
        elog(DEBUG, "user:%s", value);
      } else if (strcmp(key, "client_encoding") == 0) {
        client_encoding = value;
        elog(DEBUG, "client_encoding:%s", value);
      }
    }

    SessionEnv->Free(data);
  }
  return STATUS_OK;
}

int Session::ClientAuthentication() {
  sendBuffer.clear();

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

  return status;
}

void Session::FlushErrorState() {
  SessionEnv->ReleaseMemory();
}

void Session::ReadyForQuery() {
  PQMessage* msg = MakePQMessage('Z', 1);
  msg->PutU8(0, 'I');
  PQ_Append(sendBuffer, msg);
  if (PQ_Flush(fd, sendBuffer) != 0) {
    elog(FATAL, "send msg error");
  };
  sendBuffer.clear();
}

void Session::SendCommand(char c, const char* command, int len) {
  PQMessage* msg = MakePQMessage(c, len);
  msg->PutData(0, (u8*) command, len);
  PQ_Append(sendBuffer, msg);
}

void Session::ExecSimpleQuery(char* queryString, size_t queryLen) {
  List* parseTrees = Parser::Parse(queryString, queryLen);

  if (parseTrees) {

    CurrentTransaction = CatalogDB->Begin();
    ListCell* cell;

    foreach(cell, parseTrees) {
      Node* parseTree = (Node*) lfirst(cell);
      List* queryTree = AnalyzeAndRewrite(parseTree, queryString);
      List* planTree = PlanQueries(queryTree);

      Portal portal(this, planTree);
      portal.Start();
      portal.Run();

      SendCommand('C', "ok", strlen("ok") + 1);
    }

    if (CurrentTransaction) {
      CurrentTransaction->Commit();
      CurrentTransaction = nullptr;
    }
  }
}

}
