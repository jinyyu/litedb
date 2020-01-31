#include <litedb/storage/tuple.h>
#include <gtest/gtest.h>
#include <limits>

using namespace db;

TEST(tuple, construct) {

  std::vector<TupleMeta> entries;

  u8 v1 = std::numeric_limits<u8>::max();
  entries.emplace_back(CHAROID, (char*) &v1, sizeof(v1));

  u16 v2 = std::numeric_limits<u16>::max();
  entries.emplace_back(INT2OID, (char*) &v2, sizeof(v2));

  u32 v3 = std::numeric_limits<u32>::max();
  entries.emplace_back(INT4OID, (char*) &v3, sizeof(v3));

  u64 v4 = std::numeric_limits<u64>::max();
  entries.emplace_back(INT8OID, (char*) &v4, sizeof(v4));

  std::string v5 = "i am v5";
  entries.emplace_back(TEXTOID, v5.c_str(), v5.size());

  std::string v6 = "i am v6";
  entries.emplace_back(TEXTOID, v6.c_str(), v6.size());

  TuplePtr tuple = Tuple::Construct(entries);
  ASSERT_EQ(v1, tuple->GetBasicType<u8>(0));
  ASSERT_EQ(CHAROID, tuple->GetType(0));

  ASSERT_EQ(v2, tuple->GetBasicType<u16>(1));
  ASSERT_EQ(INT2OID, tuple->GetType(1));

  ASSERT_EQ(v3, tuple->GetBasicType<u32>(2));
  ASSERT_EQ(INT4OID, tuple->GetType(2));

  ASSERT_EQ(v4, tuple->GetBasicType<u64>(3));
  ASSERT_EQ(INT8OID, tuple->GetType(3));

  ASSERT_EQ(v5, tuple->GetSlice(4).to_string());
  ASSERT_EQ(TEXTOID, tuple->GetType(4));

  ASSERT_EQ(v6, tuple->GetSlice(5).to_string());
  ASSERT_EQ(TEXTOID, tuple->GetType(4));

  ASSERT_EQ(tuple->columns(), entries.size());

  entries.resize(1);
  tuple = Tuple::Construct(entries);
  ASSERT_EQ(tuple->columns(), entries.size());
}

TEST(tuple, construct_null) {
  std::vector<TupleMeta> entries;
  u8 v1 = std::numeric_limits<u8>::max();
  entries.emplace_back(CHAROID, (char*) &v1, sizeof(v1));

  entries.emplace_back(TEXTOID, (char*) nullptr, 0);
  entries.emplace_back(TEXTOID, (char*) nullptr, 0);

  std::string v4 = "i am v4";
  entries.emplace_back(TEXTOID, v4.c_str(), v4.size());
  entries.emplace_back(TEXTOID, (char*) nullptr, 0);

  TuplePtr tuple = Tuple::Construct(entries);

  ASSERT_EQ(v1, tuple->GetBasicType<u8>(0));

  ASSERT_EQ(0, tuple->GetSlice(1).size());
  ASSERT_EQ(nullptr, tuple->GetSlice(1).data());

  ASSERT_EQ(0, tuple->GetSlice(2).size());
  ASSERT_EQ(nullptr, tuple->GetSlice(2).data());

  ASSERT_EQ(v4, tuple->GetSlice(3).to_string());

  ASSERT_EQ(0, tuple->GetSlice(4).size());
  ASSERT_EQ(nullptr, tuple->GetSlice(4).data());

  ASSERT_ANY_THROW(tuple->GetSlice(5));

}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
