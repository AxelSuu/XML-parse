#include "unity.h"
#include "../include/parser.h"

void setUp(void) {}
void tearDown(void) {}

// --- Tests ---

void test_parser(void) {
    XmlMap map = {0};
    TEST_ASSERT_EQUAL(1, parse("../configs/test.xml", &map));
    TEST_ASSERT_EQUAL(2, map.count);
    TEST_ASSERT_EQUAL_STRING("config.stage", map.entries[0].key);
    TEST_ASSERT_EQUAL_STRING("test", map.entries[0].value);
    TEST_ASSERT_EQUAL_STRING("config.script", map.entries[1].key);
    TEST_ASSERT_EQUAL_STRING("echo \"This is a test\"", map.entries[1].value);
}

void test_parser_invalid_file(void) {
    XmlMap map = {0};
    TEST_ASSERT_EQUAL(0, parse("../configs/nonexistent.xml", &map));
}

void test_parser_null_filepath(void) {
    XmlMap map = {0};
    TEST_ASSERT_EQUAL(0, parse(NULL, &map));
}

void test_parser_null_out(void) {
    TEST_ASSERT_EQUAL(0, parse("../configs/test.xml", NULL));
}

void test_xml_get(void) {
    XmlMap map = {0};
    TEST_ASSERT_EQUAL(1, parse("../configs/test.xml", &map));
    TEST_ASSERT_EQUAL_STRING("test", xml_get(&map, "config.stage"));
    TEST_ASSERT_EQUAL_STRING("echo \"This is a test\"", xml_get(&map, "config.script"));
    TEST_ASSERT_NULL(xml_get(&map, "nonexistent"));
    TEST_ASSERT_NULL(xml_get(&map, "stage"));
    TEST_ASSERT_NULL(xml_get(&map, "script"));
}

void test_parser_flat_keys(void) {
    XmlMap map = {0};
    TEST_ASSERT_EQUAL(1, parse("../configs/flat.xml", &map));
    TEST_ASSERT_EQUAL_STRING("bar", xml_get(&map, "flat.foo"));
    TEST_ASSERT_EQUAL_STRING("42",  xml_get(&map, "flat.num"));
}

void test_parser_deep_nesting(void) {
    XmlMap map = {0};
    TEST_ASSERT_EQUAL(1, parse("../configs/deep.xml", &map));
    TEST_ASSERT_EQUAL_STRING("test", xml_get(&map, "a.b.c"));
}

void test_parser_mixed(void) {
    XmlMap map = {0};
    TEST_ASSERT_EQUAL(1, parse("../configs/mixed.xml", &map));
    TEST_ASSERT_EQUAL_STRING("hello", xml_get(&map, "mixed.top"));
    TEST_ASSERT_EQUAL_STRING("world", xml_get(&map, "mixed.nest.key"));
}

void test_xml_get_partial_key_returns_null(void) {
    XmlMap map = {0};
    TEST_ASSERT_EQUAL(1, parse("../configs/test.xml", &map));
    TEST_ASSERT_NULL(xml_get(&map, "config"));
}

void test_list_simple(void) {
    XmlMap map = {0};
    TEST_ASSERT_EQUAL(1, parse("../configs/list.xml", &map));
    TEST_ASSERT_EQUAL(3, map.count);
    TEST_ASSERT_EQUAL_STRING("apple",  xml_get(&map, "list.item[0]"));
    TEST_ASSERT_EQUAL_STRING("banana", xml_get(&map, "list.item[1]"));
    TEST_ASSERT_EQUAL_STRING("cherry", xml_get(&map, "list.item[2]"));
}

void test_list_objects(void) {
    XmlMap map = {0};
    TEST_ASSERT_EQUAL(1, parse("../configs/list2.xml", &map));
    TEST_ASSERT_EQUAL_STRING("30",                xml_get(&map, "users.user[0].@age"));
    TEST_ASSERT_EQUAL_STRING("Alice",             xml_get(&map, "users.user[0].name"));
    TEST_ASSERT_EQUAL_STRING("alice@example.com", xml_get(&map, "users.user[0].email"));
    TEST_ASSERT_EQUAL_STRING("20",                xml_get(&map, "users.user[1].@age"));
    TEST_ASSERT_EQUAL_STRING("Axel",              xml_get(&map, "users.user[1].name"));
    TEST_ASSERT_EQUAL_STRING("axel@example.com",  xml_get(&map, "users.user[1].email"));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_parser);
    RUN_TEST(test_parser_invalid_file);
    RUN_TEST(test_parser_null_filepath);
    RUN_TEST(test_parser_null_out);
    RUN_TEST(test_xml_get);
    RUN_TEST(test_parser_flat_keys);
    RUN_TEST(test_parser_deep_nesting);
    RUN_TEST(test_parser_mixed);
    RUN_TEST(test_xml_get_partial_key_returns_null);
    RUN_TEST(test_list_simple);
    RUN_TEST(test_list_objects);
    return UNITY_END();
}
