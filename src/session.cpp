#include <thread>
#include <litesql/session.h>
#include <litesql/mcxt.h>
#include <litesql/pq.h>
#include <litesql/elog.h>

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

void Session::Start() {
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
      sendBuffer.clear();
      PQMessage* msg = MakePQMessage('Z', 1);
      msg->PutU8('I', 0);
      PQ_Append(sendBuffer, msg);
      PQ_Flush(fd, sendBuffer);
      sleep(30);
      eReport(ERROR, "TEST");
    } catch (Exception& e) {
      EmitErrorReport();

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
    eReport(COMMERROR, "incomplete startup packet");
    return STATUS_ERROR;
  }
  u32 len = be32toh(packet.length);
  if (len < sizeof(packet) || len > MAX_STARTUP_PACKET_LENGTH) {
    eReport(COMMERROR, "invalid length of startup packet");
    return STATUS_ERROR;
  }
  u32 paramLen = len - sizeof(packet);
  if (paramLen > 0) {
    char* data = (char*) Malloc(paramLen);
    char* ptr = data;
    ret = PQ_GetBytes(fd, (u8*) ptr, paramLen);

    if (ret == EOF) {
      eReport(COMMERROR, "incomplete startup packet");
      return STATUS_ERROR;
    }

    while (paramLen > 0) {
      if (*ptr == '\0') {
        break;
      }

      size_t keyLen = strlen(ptr) + 1;
      if (keyLen > paramLen) {
        eReport(COMMERROR, "invalid startup packet");
        return STATUS_ERROR;
      }
      char* key = ptr;
      ptr += keyLen;
      paramLen -= keyLen;

      size_t valueLen = strlen(ptr) + 1;
      if (valueLen > paramLen) {
        eReport(COMMERROR, "invalid startup packet");
        return STATUS_ERROR;
      }
      char* value = ptr;
      ptr += valueLen;
      paramLen -= valueLen;

      if (strcmp(key, "database") == 0) {
        database = value;
        eReport(DEBUG, "database:%s", value);
      } else if (strcmp(key, "user") == 0) {
        user = value;
        eReport(DEBUG, "user:%s", value);
      } else if (strcmp(key, "client_encoding") == 0) {
        client_encoding = value;
        eReport(DEBUG, "client_encoding:%s", value);
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

  int status = PQ_Flush(fd, sendBuffer);
  if (status != 0) {
    status = STATUS_ERROR;
  } else {
    status = STATUS_OK;
  }

  MemoryContext::SwitchTo(old);
  return status;
}

}
