#include <gtest/gtest.h>
#include <litedb/utils/env.h>
#include <litedb/utils/bitmapset.h>

using namespace db;

TEST(bitmapset, test_singleton) {
  for (int i = 0; i < 2048; ++i) {
    Bitmapset* bms = bms_make_singleton(i);
    Bitmapset* bms2 = bms_make_singleton(i);
    ASSERT_EQ(bms_membership(bms), BMS_SINGLETON);
    ASSERT_TRUE(bms_equal(bms, bms2));

    ASSERT_TRUE(bms_is_member(i, bms));

    bms_free(bms);
    bms_free(bms2);
  }
}

TEST(bitmapset, test_add_member) {
  for (int i = 0; i < 2048; ++i) {
    Bitmapset* a = bms_make_singleton(i);
    Bitmapset* b = bms_copy(a);

    int member = 77 + i;

    a = bms_add_member(a, member);
    ASSERT_TRUE(bms_is_subset(b, a));
    ASSERT_TRUE(bms_is_member(member, a));

    a = bms_del_member(a, member);

    ASSERT_TRUE(bms_equal(a, b));
    ASSERT_EQ(bms_compare(a, b), 0);

  }
}

TEST(bitmapset, test_bms_union) {
  for (int i = 0; i < 2048; ++i) {
    Bitmapset* a = bms_make_singleton(i);
    Bitmapset* b = bms_make_singleton(i + 57);

    Bitmapset* c = bms_union(a, b);
    ASSERT_TRUE(bms_is_member(i, c));
    ASSERT_TRUE(bms_is_member(i + 57, c));

    ASSERT_TRUE(bms_overlap(a, c));
    ASSERT_TRUE(bms_overlap(b, c));
    ASSERT_FALSE(bms_overlap(b, a));
  }
}

int main(int argc, char* argv[]) {
  SessionEnv = std::make_shared<Environment>();

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
