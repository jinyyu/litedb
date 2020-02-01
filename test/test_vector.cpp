#include <litedb/utils/vector.h>
#include <gtest/gtest.h>
#include <litedb/utils/env.h>
#include <litedb/catalog/sys_type.h>

using namespace db;

TEST(vector, vector) {
  Vector* vector = VectorAlloc(INT2OID, 5, sizeof(u16));
  VectorSet<u16>(vector, 100, 0);
  VectorSet<u16>(vector, 101, 1);
  VectorSet<u16>(vector, 102, 2);
  VectorSet<u16>(vector, 103, 3);
  VectorSet<u16>(vector, 104, 4);

  ASSERT_EQ(VectorGet<u16>(vector, 0), 100);
  ASSERT_EQ(VectorGet<u16>(vector, 1), 101);
  ASSERT_EQ(VectorGet<u16>(vector, 2), 102);
  ASSERT_EQ(VectorGet<u16>(vector, 3), 103);
  ASSERT_EQ(VectorGet<u16>(vector, 4), 104);

  VectorFree(vector);
}

int main(int argc, char* argv[]) {
  SessionEnv = std::make_shared<Environment>();
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

