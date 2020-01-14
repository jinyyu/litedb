#include <litedb/storage/tuple.h>
#include <gtest/gtest.h>
#include <limits>

using namespace db;


TEST(tuple, construct) {

  std::vector<Entry> entries;

  u8 v1 = std::numeric_limits<u8>::max();
  entries.emplace_back(&v1, sizeof(v1));

  u16 v2 = std::numeric_limits<u16>::max();
  entries.emplace_back(&v2, sizeof(v2));

  u32 v3 = std::numeric_limits<u32>::max();
  entries.emplace_back(&v3, sizeof(v3));

  u64 v4 = std::numeric_limits<u64>::max();
  entries.emplace_back(&v4, sizeof(v4));

  std::string v5 = "i am v5";
  entries.emplace_back(v5.c_str(), v5.size());

  std::string v6 = "i am v6";
  entries.emplace_back(v6.c_str(), v6.size());

  TuplePtr tuple = Tuple::Construct(entries);
  ASSERT_EQ(v1, tuple->GetInt<u8>(0));
  ASSERT_EQ(v2, tuple->GetInt<u16>(1));
  ASSERT_EQ(v3, tuple->GetInt<u32>(2));
  ASSERT_EQ(v4, tuple->GetInt<u64>(3));

  ASSERT_EQ(v5, tuple->GetSlice(4).to_string());
  ASSERT_EQ(v6, tuple->GetSlice(5).to_string());
}

TEST(tuple, construct_null) {
  std::vector<Entry> entries;
  u8 v1 = std::numeric_limits<u8>::max();
  entries.emplace_back(&v1, sizeof(v1));

  entries.emplace_back((char*)nullptr, 0);
  entries.emplace_back((char*)nullptr, 0);

  std::string v4 = "i am v4";
  entries.emplace_back(v4.c_str(), v4.size());
  entries.emplace_back((char*)nullptr, 0);

  TuplePtr tuple = Tuple::Construct(entries);

  ASSERT_EQ(v1, tuple->GetInt<u8>(0));

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
