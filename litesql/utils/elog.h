#ifndef LITESQL_LITESQL_ELOG_H_
#define LITESQL_LITESQL_ELOG_H_
#include <exception>

namespace db {

#define COMMERROR   0            /* Client communication problems; same as
									 * LOG for server reporting, but never
									 * sent to client. */
#define DEBUG       1
#define INFO        2            /* Messages specifically requested by user (eg
								   * VACUUM VERBOSE output); always sent to
								   * client regardless of client_min_messages,
								   * but by default not sent to server log. */
#define NOTICE      3            /* Helpful messages to users about query
								   * operation; sent to client and not to server
								   * log by default. */
#define WARNING     4            /* Warnings.  NOTICE is for expected messages
								   * like implicit sequence creation by SERIAL.
								   * WARNING is for unexpected messages. */
#define ERROR       5            /* user error - abort transaction; return to
								   * known state */
#define FATAL       6            /* fatal error - abort process */

void logStartLocation(int level, const char* filename, int lineno, int location);
void logStart(int level, const char* filename, int lineno);
void logFinish(const char* fmt, ...);

struct Exception : public std::exception {
  explicit Exception(int level) : level(level) {}
  int level;
};

#define elog(level, format, ...) do   \
{                                        \
    logStart(level, __FILE__, __LINE__); \
    logFinish(format,  ##__VA_ARGS__);   \
    if (level >= ERROR) __builtin_unreachable();  \
} while(0)

#define elogLocation(level, location, format, ...) do   \
{                                        \
    logStartLocation(level, __FILE__, __LINE__, location); \
    logFinish(format,  ##__VA_ARGS__);   \
    if (level >= ERROR) __builtin_unreachable();  \
} while(0)


void EmitErrorReport();

} //db
#endif //LITESQL_LITESQL_ELOG_H_
