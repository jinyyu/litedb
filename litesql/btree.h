#ifndef LITESQL_LITESQL_BTREE_RB_H_
#define LITESQL_LITESQL_BTREE_RB_H_
namespace db {

struct BtCursor;
struct Pager;

struct Btree {
  static int Open(const char* fileName, int mode, int nPg, Btree** btree);
  static int Close(Btree* tree);

  virtual ~Btree() = default;
  virtual int SetCacheSize(int) = 0;
  virtual int SetSafetyLevel(int) = 0;
  virtual int BeginTrans() = 0;
  virtual int Commit() = 0;
  virtual int Rollback() = 0;
  virtual int BeginCkpt() = 0;
  virtual int CommitCkpt() = 0;
  virtual int RollbackCkpt() = 0;
  virtual int CreateTable(int* tab) = 0;
  virtual int CreateIndex(int* idx) = 0;
  virtual int DropTable(int tab) = 0;
  virtual int ClearTable(int tab) = 0;
  virtual int Cursor(int iTable, int wrFlag, BtCursor** cursor) = 0;
  virtual int GetMeta(int* meta) = 0;
  virtual int UpdateMeta(int* meta) = 0;
  virtual char* IntegrityCheck(int*, int) = 0;
  virtual const char* GetFilename() = 0;
  virtual int Copyfile(Btree*) = 0;
  virtual struct Pager* Pager(Btree*) = 0;
  virtual int PageDump(int, int) = 0;
};

struct BtCursor {
  virtual int Moveto(const void* key, int keyLen, int* res) = 0;
  virtual int Delete() = 0;
  virtual int Insert(const void* key, int keyLen, const void* data, int dataLen) = 0;
  virtual int First(int* res) = 0;
  virtual int Last(int* res) = 0;
  virtual int Next(int* res) = 0;
  virtual int Previous(int* res) = 0;
  virtual int KeySize(int* size) = 0;
  virtual int Key(int offset, int amt, char* zBuf) = 0;
  virtual int KeyCompare(const void* key, int keyLen, int nIgnore, int* res) = 0;
  virtual int DataSize(int* size) = 0;
  virtual int Data(int offset, int amt, char* zBuf) = 0;
  virtual int CloseCursor() = 0;
  virtual int CursorDump(int*) = 0;
};

#define BTREE_META 10

}
#endif //LITESQL_LITESQL_BTREE_RB_H_
