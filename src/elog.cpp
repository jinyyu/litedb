#include <litesql/elog.h>
#include <litesql/mcxt.h>
#include <litesql/pq.h>
#include <litesql/session.h>
#include <string.h>
#include <assert.h>
#include <glib.h>

namespace db {

struct ErrorData {
  int level;             /* error level */
  const char* filename;  /* __FILE__ of eReport() call */
  int lineno;            /* __LINE__ of eReport() call */
  int cursorPos;         /* cursor index into query string */
  char* message;         /* primary error message */
  MemoryContext* ctx;    /* context containing associated non-constant strings */
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
  if (data->level < ERROR) {
    return;
  }

  PQ_BeginMessage('E');
  const char* sev = ErrorSeverity(data->level);
  PQ_SendU8(PG_DIAG_SEVERITY);
  PQ_SendString(sev);
  PQ_BeginMessage(PG_DIAG_SEVERITY_NONLOCALIZED);
  PQ_SendString(sev);

  /* M field is required per protocol, so always send something */
  PQ_BeginMessage(PG_DIAG_MESSAGE_PRIMARY);
  if (data->message) {
    PQ_SendString(data->message);
  } else {
    PQ_SendString("missing error text");
  }

  PQ_SendU8('\0'); /* terminator */

  PQ_EndMessage();

  if (PQ_Flush(CurSession->fd) != 0) {
    CurSession->forceClose = true;
  }

}

void logStart(int level, const char* filename, int lineno) {
  int cursorPos = errorData.cursorPos;
  memset(&errorData, 0, sizeof(errorData));
  errorData.cursorPos = cursorPos;
  errorData.level = level;
  const char* str = strrchr(filename, '/');
  if (str) {
    errorData.filename = str + 1;
  } else {
    errorData.filename = filename;
  }
  errorData.lineno = lineno;
  errorData.ctx = ErrorContext;
}

void logFinish(const char* fmt, ...) {
  int level = errorData.level;
  MemoryContext::SwitchTo(errorData.ctx);

  va_list args;
  va_start(args, fmt);
  char* tmp = g_strdup_vprintf(fmt, args);
  va_end(args);

  size_t len = strlen(tmp) + 1;
  errorData.message = (char*) Malloc(len);
  memcpy(errorData.message, tmp, len);

  g_free(tmp);

  if (level >= ERROR) {
    /*
     * Note that we leave CurrentMemoryContext set to ErrorContext. The
     * handler should reset it to something else soon.
     */
    throw Exception(level);
  }
}

void EmitErrorReport() {
  MemoryContext* old = MemoryContext::SwitchTo(errorData.ctx);

  SendMessageToServer(&errorData);
  SendMessageToFrontend(&errorData);

  MemoryContext::SwitchTo(old);
}

}