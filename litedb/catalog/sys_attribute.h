#ifndef LITEDB_CATALOG_SYS_ATTRIBUTE_H_
#define LITEDB_CATALOG_SYS_ATTRIBUTE_H_

#include <litedb/int.h>
namespace db {

struct SysAttribute {
  u64 attrelid;                /*relation containing this attribute*/
  u32 atttypid;                /*the id of the instance*/
  char attname[NAMEDATALEN];   /*name of attribute*/
  u16 attnum;                  /*attnum is the "attribute number" for the attribute*/
};


#define SysAttributeRelationName "sys_attribute"
#define SysAttributeRelationId 1249
#define Anum_sys_attribute_attrelid 1
#define Anum_sys_attribute_atttypid 2
#define Anum_sys_attribute_attname 3
#define Anum_sys_attribute_attnum 4

}

#endif //LITEDB_CATALOG_SYS_ATTRIBUTE_H_
