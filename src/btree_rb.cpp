#include <litesql/btree.h>
#include <litesql/int.h>
#include <string.h>
#include <stdlib.h>
#include <litesql/elog.h>
#include <unordered_map>
#include <map>
#include <vector>
#include <assert.h>

namespace db {

/*
** Legal values for BtCursor.eSkip.
*/
#define SKIP_NONE     0   /* Always step the cursor */
#define SKIP_NEXT     1   /* The next BtCursor.Next() is a no-op */
#define SKIP_PREV     2   /* The next BtCursor.Previous() is a no-op */
#define SKIP_INVALID  3   /* Calls to Next() and Previous() are invalid */

/*
** Legal values for RBtree.transState.
*/
#define TRANS_NONE           0  // No transaction is in progress
#define TRANS_INTRANSACTION  1  // A transaction is in progress
#define TRANS_INCHECKPOINT   2  // A checkpoint is in progress
#define TRANS_ROLLBACK       3  // We are currently rolling back a checkpoint or transaction

/*
** Legal values for BtRollbackOp.op:
*/
#define ROLLBACK_INSERT 1 /* Insert a record */
#define ROLLBACK_DELETE 2 /* Delete a record */
#define ROLLBACK_CREATE 3 /* Create a table */
#define ROLLBACK_DROP   4 /* Drop a table */

/*
 * During each transaction (or checkpoint), a linked-list of
 * "rollback-operations" is accumulated. If the transaction is rolled back,
 * then the list of operations must be executed (to restore the database to
 * it's state before the transaction started). If the transaction is to be
 * committed, just delete the list.
 *
 * Each operation is represented as follows, depending on the value of op:
 *
 * ROLLBACK_INSERT  ->  Need to insert (key, data) into table tab.
 * ROLLBACK_DELETE  ->  Need to delete the record (key) into table tab.
 * ROLLBACK_CREATE  ->  Need to create table tab.
 * ROLLBACK_DROP    ->  Need to drop table tab.
 */
struct BtRollbackOp {
  u8 op;
  int tab;
  int keyLen;
  void* key;
  int dataLen;
  void* data;
  BtRollbackOp* next;
};

/*
** Delete a linked list of BtRollbackOp structures.
*/
static void deleteRollbackList(BtRollbackOp* op) {
  while (op) {
    BtRollbackOp* tmp = op->next;
    if (op->data) free(op->data);
    if (op->key) free(op->key);
    free(op);
    op = tmp;
  }
}

struct BtRbTree;
typedef std::unordered_map<int, BtRbTree*> TableHash;
struct Slice {
  void* data;
  int len;
};

/*
 * The key-compare function for the red-black trees. Returns as follows:
 *
 * (key1 < key2)             -1
 * (key1 == key2)             0
 * (key1 > key2)              1
 *
 * Keys are compared using memcmp(). If one key is an exact prefix of the
 * other, then the shorter key is less than the longer key.
 */
static int key_compare(void const* pKey1, int nKey1, void const* pKey2, int nKey2) {
  int mcmp = memcmp(pKey1, pKey2, (nKey1 <= nKey2) ? nKey1 : nKey2);
  if (mcmp == 0) {
    if (nKey1 == nKey2) return 0;
    return ((nKey1 < nKey2) ? -1 : 1);
  }
  return ((mcmp > 0) ? 1 : -1);
}

static bool operator==(const Slice& a, const Slice& b) {
  return key_compare(a.data, a.len, b.data, b.len) == 0;
}
static bool operator<(const Slice& a, const Slice& b) {
  return key_compare(a.data, a.len, b.data, b.len) == -1;
}
static bool operator<=(const Slice& a, const Slice& b) {
  return key_compare(a.data, a.len, b.data, b.len) != 1;
}

typedef std::map<Slice, Slice> TreeData;

struct RBtree : public Btree {
  int metaData[BTREE_META];
  int nextIdx;                // next available table index
  TableHash tableHash;        // All created tables, by index
  u8 isAnonymous;             // True if this RBtree is to be deleted when closed
  u8 transState;              // State of this RBtree wrt transactions

  TreeData treeData;

  BtRollbackOp* transRollback;
  BtRollbackOp* checkRollback;
  BtRollbackOp* checkRollbackTail;

  static int Open(RBtree** btree) {
    RBtree* tree = new RBtree();

    /* Create a binary tree for the MASTER table at location 2 */
    tree->createTable(2);
    tree->nextIdx = 3;

    //Set file type to 4
    tree->metaData[2] = 4;
    *btree = tree;
    return STATUS_OK;
  }

  ~RBtree() final {
    for (auto& it : tableHash) {
      free(it.second);
    }
  }

  int SetCacheSize(int) final {
    return STATUS_OK;
  }

  int SetSafetyLevel(int) final {
    return STATUS_OK;
  }

  int BeginTrans() final {
    if (transState != TRANS_NONE)
      return STATUS_ERROR;

    assert(!transRollback);
    transState = TRANS_INTRANSACTION;
    return STATUS_OK;
  }

  int Commit() final {
    // Just delete transRollback and checkRollback
    deleteRollbackList(transRollback);
    deleteRollbackList(checkRollback);
    transRollback = nullptr;
    checkRollback = nullptr;
    checkRollbackTail = nullptr;
    transState = TRANS_NONE;
    return STATUS_OK;
  }

  int Rollback() final {
    transState = TRANS_ROLLBACK;
    executeRollbackList(checkRollback);
    executeRollbackList(transRollback);
    transRollback = nullptr;
    checkRollback = nullptr;
    checkRollbackTail = nullptr;
    transState = TRANS_NONE;
    return STATUS_OK;
  }

  int BeginCkpt() final {
    if (transState != TRANS_INTRANSACTION) {
      return STATUS_ERROR;
    }
    assert(!checkRollback);
    assert(!checkRollbackTail);
    transState = TRANS_INCHECKPOINT;
    return STATUS_OK;
  }

  int CommitCkpt() final {
    if (transState == TRANS_INCHECKPOINT) {
      if (checkRollback) {
        checkRollbackTail->next = transRollback;
        transRollback = checkRollback;
        checkRollback = nullptr;
        checkRollbackTail = nullptr;
      }
      transState = TRANS_INTRANSACTION;
    }
    return STATUS_OK;
  }

  int RollbackCkpt() final {
    if (transState != TRANS_INCHECKPOINT) {
      return STATUS_OK;
    }
    transState = TRANS_ROLLBACK;
    executeRollbackList(checkRollback);
    checkRollback = nullptr;
    checkRollbackTail = nullptr;
    transState = TRANS_INTRANSACTION;
    return STATUS_OK;
  }

  /*
   * Create a new table in the supplied RBtree. Set *tab to the new table number.
   * Return STATUS_OK if the operation is a success.
   */
  int CreateTable(int* tab) final {
    assert(transState != TRANS_NONE);

    *tab = nextIdx++;
    createTable(*tab);

    /* Set up the rollback structure (if we are not doing this as part of a
     * rollback) */
    if (transState != TRANS_ROLLBACK) {
      BtRollbackOp* rollbackOp = (BtRollbackOp*) malloc(sizeof(BtRollbackOp));
      memset(rollbackOp, 0, sizeof(BtRollbackOp));

      rollbackOp->op = ROLLBACK_DROP;
      rollbackOp->tab = *tab;
      logRollbackOp(rollbackOp);
    }

    return STATUS_OK;
  }

  int CreateIndex(int* idx) final {
    return CreateTable(idx);
  }

  /*
   * Delete table n from the supplied RBtree.
   */
  int DropTable(int tab) final;

  int ClearTable(int tab) final {
    for (auto& it : treeData) {
      void* key = it.first.data;
      int keyLen = it.first.len;
      void* data = it.second.data;
      int dataLen = it.second.len;

      if (transState == TRANS_ROLLBACK) {
        free(key);
        free(data);
      } else {
        BtRollbackOp* op = (BtRollbackOp*) malloc(sizeof(BtRollbackOp));
        op->op = ROLLBACK_INSERT;
        op->tab = tab;
        op->keyLen = keyLen;
        op->key = key;
        op->data = data;
        op->dataLen = dataLen;
        logRollbackOp(op);
      }
    }
    treeData.clear();
    return STATUS_OK;
  }

  /*
   * Get a new cursor for table iTable of the supplied Rbtree. The wrFlag
   * parameter indicates that the cursor is open for writing.
   *
   * Note that RbtCursor.eSkip and RbtCursor.pNode both initialize to 0.
   */
  int Cursor(int iTable, int wrFlag, BtCursor** cursor) final;

  int GetMeta(int* meta) final {
    memcpy(meta, metaData, sizeof(int) * BTREE_META);
    return STATUS_OK;
  }

  int UpdateMeta(int* meta) final {
    memcpy(metaData, meta, sizeof(int) * BTREE_META);
    return STATUS_OK;
  }

  char* IntegrityCheck(int*, int) final {
    return strdup("not implemented");
  }

  const char* GetFilename() final {
    return nullptr;  /* A NULL return indicates there is no underlying file */
  }

  int Copyfile(Btree*) final {
    return STATUS_INTERNAL;  /* Not implemented */
  }

  struct Pager* Pager(Btree*) final {
    return nullptr;
  }

  int PageDump(int, int) final {
    return STATUS_OK;
  }

  explicit RBtree()
      : nextIdx(0),
        isAnonymous(0),
        transState(0),
        transRollback(nullptr),
        checkRollback(nullptr),
        checkRollbackTail(nullptr) {
    memset(metaData, 0, sizeof(metaData));
  }

  /*
   * Create table tbl in tree RBtree. Table tbl must not exist.
   */
  void createTable(int tbl);

  /*
   * Execute and delete the supplied rollback-list on RBtree.
   */
  void executeRollbackList(BtRollbackOp* list);

  /*
 * Log a single "rollback-op" for the given RBtree. See comments for struct
 * BtRollbackOp.
 */
  void logRollbackOp(BtRollbackOp* rollbackOp) {
    assert(transState == TRANS_INCHECKPOINT || transState == TRANS_INTRANSACTION);

    if (transState == TRANS_INTRANSACTION) {
      rollbackOp->next = transRollback;
      transRollback = rollbackOp;
    }

    if (transState == TRANS_INCHECKPOINT) {
      if (!checkRollback) {
        checkRollbackTail = rollbackOp;
      }
      rollbackOp->next = checkRollback;
      checkRollback = rollbackOp;
    }
  }
};

struct RbtCursor : public BtCursor {
  RBtree* rbTree;
  int iTree;               /* Index of pTree in rbTree */
  BtRbTree* tree;
  RbtCursor* shared;       /* List of all cursors on the same RBtree */
  u8 eSkip;                /* Determines if next step operation is a no-op */
  u8 wrFlag;               /* True if this cursor is open for writing */
  TreeData::iterator iter;

  explicit RbtCursor()
      : rbTree(nullptr),
        iTree(0),
        tree(nullptr),
        shared(nullptr),
        eSkip(0),
        wrFlag(0) {}

  virtual ~RbtCursor() {}

  /* Move the cursor so that it points to an entry near key.
   * Return a success code.
   *
   *     *res<0      The cursor is pointing at an entry that
   *                 is smaller than key or if the table is empty
   *                 and the cursor is therefore point to nothing.
   *
   *     *res==0     The cursor is pointing at an entry that
   *                 exactly matches key.
   *
   *     *res>0      The cursor is pointing at an entry that
   *                 is larger than key.
   */
  int Moveto(const void* key, int keyLen, int* res) final {
    *res = -1;
    eSkip = SKIP_NONE;
    if (rbTree->treeData.empty()) {
      iter = rbTree->treeData.end();
      return STATUS_OK;
    }

    Slice slice{.data = (void*) key, .len = keyLen};
    iter = rbTree->treeData.lower_bound(slice);
    if (iter->second == slice) {
      // cursor == key
      *res = 0;
      return STATUS_OK;
    }
    assert(iter != rbTree->treeData.begin());
    if (iter == rbTree->treeData.end()) {
      iter--;
      // cursor < key
      *res = -1;
      return STATUS_OK;
    }
    // cursor > key
    *res = 1;
    return STATUS_OK;
  }

  /*
   * Delete the entry that the cursor is pointing to.
   *
   * The cursor is pointing at either the next or the previous
   * entry.  If the cursor is pointing to the next entry, then
   * the eSkip flag is set to SKIP_NEXT which forces the next call
   * to Next() to be a no-op.  That way, you can always call
   * Next() after a delete and the cursor will be  pointing
   * to the first entry after the deleted entry.  Similarly,
   * eSkip is set to SKIP_PREV is the cursor is pointing to
   * the entry prior to the deleted entry so that a subsequent call to
   * Previous() will always leave the cursor pointing at the
   * entry immediately before the one that was deleted.
   */
  int Delete() final {
    /* It is illegal to call sqliteRbtreeDelete() if we are not in a transaction */
    assert(rbTree->transState != TRANS_NONE);

    /* Make sure some other cursor isn't trying to read this same table */
    if (checkReadLocks()) {
      return STATUS_LOCKED; /* The table points to has a read lock */
    }

    if (rbTree->treeData.empty() || iter == rbTree->treeData.end()) {
      return STATUS_OK;
    }

    /* If we are not currently doing a rollback, set up a rollback op for this deletion */
    if (rbTree->transState != TRANS_ROLLBACK) {
      BtRollbackOp* op = (BtRollbackOp*) malloc(sizeof(BtRollbackOp));
      memset(op, 0, sizeof(BtRollbackOp));
      op->tab = this->iTree;
      // key
      op->keyLen = iter->first.len;
      op->key = iter->first.data;
      // data
      op->dataLen = iter->second.len;
      op->data = iter->second.data;
      op->op = ROLLBACK_INSERT;
      rbTree->logRollbackOp(op);
    }

    void* key = iter->first.data;
    void* data = iter->second.data;
    iter = rbTree->treeData.erase(iter);
    if (iter != rbTree->treeData.end()) {
      eSkip = SKIP_NEXT;
    } else {
      if (iter != rbTree->treeData.begin()) {
        iter--;
        eSkip = SKIP_PREV;
      } else {
        eSkip = SKIP_NEXT;
      }
    }
    if (rbTree->transState == TRANS_ROLLBACK) {
      if (key) free(key);
      if (data) free(data);
    }
    return STATUS_OK;
  }

  int Insert(const void* key, int keyLen, const void* data, int dataLen) final {
    /* It is illegal to call sqliteRbtreeInsert() if we are not in a transaction */
    assert(rbTree->transState != TRANS_NONE);

    /* Make sure some other cursor isn't trying to read this same table */
    if (checkReadLocks()) {
      return STATUS_LOCKED; /* The table pCur points to has a read lock */
    }
    /* Take a copy of the input data now, in case we need it for the
    * replace case */
    Slice sliceData;
    sliceData.data = malloc(dataLen);
    memcpy(sliceData.data, data, dataLen);

    Slice slice{.data = (void*) key, .len = keyLen};
    auto it = rbTree->treeData.find(slice);

    if (it == rbTree->treeData.end()) {
      Slice sliceKey;
      sliceKey.data = malloc(keyLen);
      memcpy(sliceKey.data, key, keyLen);
      sliceKey.len = keyLen;

      iter = rbTree->treeData.insert(std::make_pair(sliceKey, sliceData)).first;

      /* Set up a rollback-op in case we have to roll this operation back */
      if (rbTree->transState != TRANS_ROLLBACK) {
        BtRollbackOp* op = (BtRollbackOp*) malloc(sizeof(BtRollbackOp));
        op->op = ROLLBACK_DELETE;
        op->tab = iTree;
        op->keyLen = keyLen;
        op->key = malloc(keyLen);
        memcpy(op->key, key, keyLen);
        rbTree->logRollbackOp(op);
      }

    } else {
      /* No need to insert a new node in the tree, as the key already exists.
       * Just clobber the current nodes data.
       */

      /* Set up a rollback-op in case we have to roll this operation back */
      if (rbTree->transState != TRANS_ROLLBACK) {
        BtRollbackOp* op = (BtRollbackOp*) malloc(sizeof(BtRollbackOp));
        op->tab = iTree;
        op->keyLen = keyLen;
        op->key = malloc(keyLen);
        memcpy(op->key, key, op->keyLen);

        op->dataLen = it->second.len;
        op->data = it->second.data;
        op->op = ROLLBACK_INSERT;
        rbTree->logRollbackOp(op);
      } else {
        free(it->second.data);
      }

      it->second.data = sliceData.data;
      it->second.len = sliceData.len;
    }
    return STATUS_OK;
  }

  int First(int* res) final {
    if (!rbTree->treeData.empty()) {
      iter = rbTree->treeData.begin();
      *res = 0;
    } else {
      *res = 1;
    }
    eSkip = SKIP_NONE;
    return STATUS_OK;
  }

  int Last(int* res) final {
    if (!rbTree->treeData.empty()) {
      iter = rbTree->treeData.end();
      iter--;
      *res = 0;
    } else {
      *res = 1;
    }
    eSkip = SKIP_NONE;
    return STATUS_OK;
  }

  /*
   * Advance the cursor to the next entry in the database.  If
   * successful then set *res=0.  If the cursor
   * was already pointing to the last entry in the database before
   * this routine was called, then set *res=1.
   */
  int Next(int* res) final {
    if (eSkip != SKIP_NEXT) {
      if (iter != rbTree->treeData.end()) {
        iter++;
      }
    }
    if (rbTree->treeData.empty() || iter == rbTree->treeData.end()) {
      *res = 1;
    } else {
      *res = 0;
    }
    eSkip = SKIP_NONE;
    return STATUS_OK;
  }

  int Previous(int* res) final {
    if (rbTree->treeData.empty() || iter == rbTree->treeData.begin()) {
      *res = 1;
    } else {
      *res = 0;
    }
    if (rbTree->transState != SKIP_PREV) {
      if (iter != rbTree->treeData.begin()) {
        iter--;
      }
    }
    eSkip = SKIP_NONE;
    return STATUS_OK;
  }
  int KeySize(int* size) final {
    if (iter != rbTree->treeData.end()) {
      *size = iter->first.len;
    } else {
      *size = 0;
    }
    return STATUS_OK;
  }

  int Key(int offset, int amt, char* zBuf) final {
    if (iter == rbTree->treeData.end()) {
      return 0;
    }

    if ((amt + offset) <= iter->first.len) {
      memcpy(zBuf, ((char*) iter->first.data) + offset, amt);
    } else {
      amt = iter->first.len - offset;
      memcpy(zBuf, ((char*) iter->first.data) + offset, amt);
    }
    return amt;
  }

  int KeyCompare(const void* key, int keyLen, int nIgnore, int* res) final {
    if (iter == rbTree->treeData.end()) {
      *res = -1;
    } else {
      if (iter->first.len - nIgnore < 0) {
        *res = -1;
      } else {
        *res = key_compare(iter->first.data, iter->first.len - nIgnore,
                           key, keyLen);
      }
    }
    return STATUS_OK;
  }

  int DataSize(int* size) final {
    if (iter != rbTree->treeData.end()) {
      *size = iter->second.len;
    } else {
      *size = 0;
    }
    return STATUS_OK;
  }

  int Data(int offset, int amt, char* zBuf) final {
    if (iter == rbTree->treeData.end()) {
      return 0;
    }

    if ((amt + offset) <= iter->second.len) {
      memcpy(zBuf, ((char*) iter->second.data) + offset, amt);
    } else {
      amt = iter->second.len - offset;
      memcpy(zBuf, ((char*) iter->second.data) + offset, amt);
    }
    return amt;
  }

  int CloseCursor() final {
    delete (this);
    return STATUS_OK;
  }

  int CursorDump(int*) final {
    return STATUS_OK;
  }

private:
  int checkReadLocks() {
    return STATUS_OK;
  }
};

struct BtRbTree {
  RbtCursor* cursors; // All cursors pointing to this Table
  RBtree* tree;      /* the tree, or NULL */
};

int RBtree::Cursor(int iTable, int wrFlag, BtCursor** btCursor) {
  RbtCursor* cur = new RbtCursor();
  cur->tree = this->tableHash.at(iTable);
  assert(cur->tree);
  cur->rbTree = this;
  cur->iTree = iTable;
  cur->wrFlag = wrFlag;
  cur->shared = cur->tree->cursors;
  cur->tree->cursors = cur;

  assert(cur->tree);
  *btCursor = cur;
  return STATUS_OK;
}

void RBtree::createTable(int tbl) {
  BtRbTree* table = (BtRbTree*) malloc(sizeof(BtRbTree));
  memset(table, 0, sizeof(BtRbTree));
  tableHash.insert(std::make_pair(tbl, table));
}

int RBtree::DropTable(int tab) {
  assert(transState != TRANS_NONE);

  ClearTable(tab);

  BtRbTree* tree = tableHash[tab];
  assert(tree);
  assert(!tree->cursors);
  free(tree);

  if (transState != TRANS_ROLLBACK) {
    BtRollbackOp* rollbackOp = (BtRollbackOp*) malloc(sizeof(BtRollbackOp));
    rollbackOp->op = ROLLBACK_CREATE;
    rollbackOp->tab = tab;
    logRollbackOp(rollbackOp);
  }
  return STATUS_OK;
}

void RBtree::executeRollbackList(BtRollbackOp* list) {

}

int Btree::Open(const char* fileName, int mode, int nPg, Btree** btree) {
  if (strcmp(fileName, ":memory:") == 0) {
    RBtree* tree;
    int ret = RBtree::Open(&tree);
    *btree = tree;
    return ret;
  }
  eReport(DEBUG, "btree not impl yet");
  return STATUS_ERROR;
}

int Btree::Close(Btree* tree) {
  delete (tree);
  return STATUS_OK;
}

}

