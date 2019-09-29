#include <litesql/elog.h>
#include <litesql/mcxt.h>
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
    "INFO",
    "NOTICE",
    "WARNING",
    "ERROR",
    "FATAL",
    "PANIC",
};

static const char* ErrorSeverity(int level) {
  assert(level <= FATAL);
  return ErrorLevelStrings[level];
}

static void SendMessageToServer(ErrorData* data) {
  fprintf(stderr, "%s [%s:%d] %s\n",
          ErrorSeverity(data->level), data->filename, data->lineno, data->message);
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
  MemoryContext* old = MemoryContext::SwitchTo(errorData.ctx);

  va_list args;
  va_start(args, fmt);
  char* tmp = g_strdup_vprintf(fmt, args);
  va_end(args);

  size_t len = strlen(tmp) + 1;
  errorData.message = (char*) Malloc(len);
  memcpy(errorData.message, tmp, len);

  g_free(tmp);

  if (level == ERROR) {
    /*
     * Note that we leave CurrentMemoryContext set to ErrorContext. The
     * handler should reset it to something else soon.
     */
    throw Exception(level);
  }

  EmitErrorReport();

  if (errorData.message) {
    Free(errorData.message);
  }

  /* Exit error-handling context */
  MemoryContext::SwitchTo(old);

  if (level == FATAL) {
    fflush(stdout);
    fflush(stderr);

    //todo: 只退出当前线程
    exit(0);
  }
}

void EmitErrorReport() {
  MemoryContext* old = MemoryContext::SwitchTo(errorData.ctx);

  SendMessageToServer(&errorData);

  MemoryContext::SwitchTo(old);
}

}