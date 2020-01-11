#include <glib.h>
#include <litedb/bin/server.h>
#include <litedb/bin/initdb.h>

void help(GOptionContext* context) {
  char* helpStr = g_option_context_get_help(context, true, NULL);
  fprintf(stderr, "%s", helpStr);
  free(helpStr);
  exit(EXIT_FAILURE);
}

int main(int argc, char* argv[]) {
  int port = 5432;
  const char* database = nullptr;
  bool initdb = false;

  GOptionEntry entries[] = {
      {"port", 'p', 0, G_OPTION_ARG_INT, &port, "listen port", NULL},
      {"database", 'd', 0, G_OPTION_ARG_STRING, &database, "location for the database", NULL},
      {"initdb", 'i', 0, G_OPTION_ARG_NONE, &initdb, "initial the database", NULL},
      {NULL}
  };

  GError* error = NULL;
  GOptionContext* context = g_option_context_new("usage");
  g_option_context_add_main_entries(context, entries, NULL);
  if (!g_option_context_parse(context, &argc, &argv, &error)) {
    fprintf(stderr, "option parsing failed: %s\n", error->message);
    exit(EXIT_FAILURE);
  }

  if (!database) {
    help(context);
  }

  g_option_context_free(context);

  if (initdb) {
    return db::InitDBMain(database);
  }

  return db::Server::Main(database, port);
}