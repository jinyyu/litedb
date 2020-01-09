#include <litedb/utils/elog.h>
#include <litedb/utils/env.h>
#include <litedb/utils/pq.h>
#include <litedb/exec/session.h>
#include <string.h>
#include <assert.h>
#include <glib.h>

namespace db {

struct ErrorData {
  int level;             /* error level */
  const char* filename;  /* __FILE__ of elog() call */
  int lineno;            /* __LINE__ of elog() call */
  int cursorPos;         /* cursor index into query string */
  char* message;         /* primary error message */
};
static thread_local ErrorData errorData;
static const char* ErrorLevelStrings[] = {
    "COMMERROR",
    "DEBUG",
    "INFO",
    "NOTICE",
    "WARNING",
    "ERROR",
    "FATAL",
};

static const char* ErrorSeverity(int level) {
  assert(level <= FATAL);
  return ErrorLevelStrings[level];
}

static void SendMessageToServer(ErrorData* data) {
  fprintf(stderr, "%s [%s:%d] %s\n",
          ErrorSeverity(data->level),
          data->filename,
          data->lineno,
          data->message);
}

static void SendMessageToFrontend(ErrorData* data) {
  if (data->level == COMMERROR) {
    CurSession->forceClose = true;
    return;
  } else if (data->level < ERROR) {
    return;
  }

  std::vector<u8> buffer;
  const char* sev = ErrorSeverity(data->level);
  buffer.push_back(PG_DIAG_SEVERITY);
  buffer.insert(buffer.end(), sev, sev + strlen(sev) + 1);

  buffer.push_back(PG_DIAG_SEVERITY_NONLOCALIZED);
  buffer.insert(buffer.end(), sev, sev + strlen(sev) + 1);

  const char* str;
  if (data->message) {
    str = data->message;
  } else {
    str = "missing error text";
  }
  buffer.push_back(PG_DIAG_MESSAGE_PRIMARY);
  buffer.insert(buffer.end(), str, str + strlen(str) + 1);

  if (data->filename) {
    str = data->filename;
    buffer.push_back(PG_DIAG_SOURCE_FILE);
    buffer.insert(buffer.end(), str, str + strlen(str) + 1);
  }

  if (data->lineno > 0) {
    char lineno[32];
    snprintf(lineno, sizeof(lineno), "%d", data->lineno);
    buffer.push_back(PG_DIAG_SOURCE_LINE);
    buffer.insert(buffer.end(), lineno, lineno + strlen(lineno) + 1);
  }

  if (data->cursorPos > 0) {
    char pos[32];
    snprintf(pos, sizeof(pos), "%d", data->cursorPos);
    buffer.push_back(PG_DIAG_STATEMENT_POSITION);
    buffer.insert(buffer.end(), pos, pos + strlen(pos) + 1);
  }

  //terminator
  buffer.push_back('\0');

  PQMessage* pqMsg = MakePQMessage('E', buffer.size());
  pqMsg->PutData(0, buffer.data(), buffer.size());

  if (PQ_Flush(CurSession->fd, (u8*) pqMsg, buffer.size() + sizeof(PQMessage)) != 0) {
    CurSession->forceClose = true;
  }
}

void logStartLocation(int level, const char* filename, int lineno, int location) {
  logStart(level, filename, lineno);
  errorData.cursorPos = location;
}

void logStart(int level, const char* filename, int lineno) {
  memset(&errorData, 0, sizeof(errorData));
  errorData.level = level;
  const char* str = strrchr(filename, '/');
  if (str) {
    errorData.filename = str + 1;
  } else {
    errorData.filename = filename;
  }
  errorData.lineno = lineno;
}

void logFinish(const char* fmt, ...) {
  int level = errorData.level;

  va_list args;
  va_start(args, fmt);
  char* tmp = g_strdup_vprintf(fmt, args);
  va_end(args);

  size_t len = strlen(tmp) + 1;
  errorData.message = (char*) SessionEnv->Malloc(len);
  memcpy(errorData.message, tmp, len);

  g_free(tmp);

  if (level >= ERROR) {
    /*
     * Note that we leave CurrentMemoryContext set to ErrorContext. The
     * handler should reset it to something else soon.
     */
    throw Exception(level);
  }

  EmitErrorReport();
}

void EmitErrorReport() {
  SendMessageToServer(&errorData);
  SendMessageToFrontend(&errorData);
}

}