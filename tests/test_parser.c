#include "unity.h"
#include "../include/parser.h"

void setUp(void) {}
void tearDown(void) {}

// --- Tests ---

void test_parser(void) {
    XmlMap map = {0};
    TEST_ASSERT_EQUAL(XML_OK, parse("../configs/test.xml", &map));
    TEST_ASSERT_EQUAL(2, map.count);
    TEST_ASSERT_EQUAL_STRING("config.stage", map.entries[0].key);
    TEST_ASSERT_EQUAL_STRING("test", map.entries[0].value);
    TEST_ASSERT_EQUAL_STRING("config.script", map.entries[1].key);
    TEST_ASSERT_EQUAL_STRING("echo \"This is a test\"", map.entries[1].value);
}

void test_parser_invalid_file(void) {
    XmlMap map = {0};
    TEST_ASSERT_EQUAL(XML_ERR_IO, parse("../configs/nonexistent.xml", &map));
}

void test_parser_null_filepath(void) {
    XmlMap map = {0};
    TEST_ASSERT_EQUAL(XML_ERR_ARGS, parse(NULL, &map));
}

void test_parser_null_out(void) {
    TEST_ASSERT_EQUAL(XML_ERR_ARGS, parse("../configs/test.xml", NULL));
}

void test_xml_get(void) {
    XmlMap map = {0};
    TEST_ASSERT_EQUAL(XML_OK, parse("../configs/test.xml", &map));
    TEST_ASSERT_EQUAL_STRING("test", xml_get(&map, "config.stage"));
    TEST_ASSERT_EQUAL_STRING("echo \"This is a test\"", xml_get(&map, "config.script"));
    TEST_ASSERT_NULL(xml_get(&map, "nonexistent"));
    TEST_ASSERT_NULL(xml_get(&map, "stage"));
    TEST_ASSERT_NULL(xml_get(&map, "script"));
}

void test_parser_flat_keys(void) {
    XmlMap map = {0};
    TEST_ASSERT_EQUAL(XML_OK, parse("../configs/flat.xml", &map));
    TEST_ASSERT_EQUAL_STRING("bar", xml_get(&map, "flat.foo"));
    TEST_ASSERT_EQUAL_STRING("42",  xml_get(&map, "flat.num"));
}

void test_parser_deep_nesting(void) {
    XmlMap map = {0};
    TEST_ASSERT_EQUAL(XML_OK, parse("../configs/deep.xml", &map));
    TEST_ASSERT_EQUAL_STRING("test", xml_get(&map, "a.b.c"));
}

void test_parser_mixed(void) {
    XmlMap map = {0};
    TEST_ASSERT_EQUAL(XML_OK, parse("../configs/mixed.xml", &map));
    TEST_ASSERT_EQUAL_STRING("hello", xml_get(&map, "mixed.top"));
    TEST_ASSERT_EQUAL_STRING("world", xml_get(&map, "mixed.nest.key"));
}

void test_xml_get_partial_key_returns_null(void) {
    XmlMap map = {0};
    TEST_ASSERT_EQUAL(XML_OK, parse("../configs/test.xml", &map));
    TEST_ASSERT_NULL(xml_get(&map, "config"));
}

void test_list_simple(void) {
    XmlMap map = {0};
    TEST_ASSERT_EQUAL(XML_OK, parse("../configs/list.xml", &map));
    TEST_ASSERT_EQUAL(5, map.count);
    TEST_ASSERT_EQUAL_STRING("apple",      xml_get(&map, "shopping.item[0]"));
    TEST_ASSERT_EQUAL_STRING("banana",     xml_get(&map, "shopping.item[1]"));
    TEST_ASSERT_EQUAL_STRING("cherry",     xml_get(&map, "shopping.item[2]"));
    TEST_ASSERT_EQUAL_STRING("dates",      xml_get(&map, "shopping.item[3]"));
    TEST_ASSERT_EQUAL_STRING("elderberry", xml_get(&map, "shopping.item[4]"));
}

void test_list_objects(void) {
    XmlMap map = {0};
    TEST_ASSERT_EQUAL(XML_OK, parse("../configs/list2.xml", &map));
    TEST_ASSERT_EQUAL_STRING("30",                xml_get(&map, "users.user[0].@age"));
    TEST_ASSERT_EQUAL_STRING("Alice",             xml_get(&map, "users.user[0].name"));
    TEST_ASSERT_EQUAL_STRING("alice@example.com", xml_get(&map, "users.user[0].email"));
    TEST_ASSERT_EQUAL_STRING("admin",             xml_get(&map, "users.user[0].role"));
    TEST_ASSERT_EQUAL_STRING("20",                xml_get(&map, "users.user[1].@age"));
    TEST_ASSERT_EQUAL_STRING("Axel",              xml_get(&map, "users.user[1].name"));
    TEST_ASSERT_EQUAL_STRING("axel@example.com",  xml_get(&map, "users.user[1].email"));
    TEST_ASSERT_EQUAL_STRING("developer",         xml_get(&map, "users.user[1].role"));
    TEST_ASSERT_EQUAL_STRING("25",                xml_get(&map, "users.user[2].@age"));
    TEST_ASSERT_EQUAL_STRING("Bob",               xml_get(&map, "users.user[2].name"));
    TEST_ASSERT_EQUAL_STRING("bob@example.com",   xml_get(&map, "users.user[2].email"));
    TEST_ASSERT_EQUAL_STRING("viewer",            xml_get(&map, "users.user[2].role"));
}

void test_bad_unclosed(void) {
    XmlMap map = {0};
    TEST_ASSERT_EQUAL(XML_ERR_PARSE, parse("../configs/bad_unclosed.xml", &map));
}

void test_bad_mismatch(void) {
    XmlMap map = {0};
    TEST_ASSERT_EQUAL(XML_ERR_PARSE, parse("../configs/bad_mismatch.xml", &map));
}

void test_bad_amp(void) {
    XmlMap map = {0};
    TEST_ASSERT_EQUAL(XML_ERR_PARSE, parse("../configs/bad_amp.xml", &map));
}

void test_overflow(void) {
    /* capacity=1 with a 2-entry file must fail and leave the first entry intact */
    XmlMap map = { .capacity = 1 };
    TEST_ASSERT_EQUAL(XML_ERR_OVERFLOW, parse("../configs/test.xml", &map));
    TEST_ASSERT_EQUAL(1, map.count);
    TEST_ASSERT_EQUAL_STRING("test", xml_get(&map, "config.stage"));
    TEST_ASSERT_NULL(xml_get(&map, "config.script"));
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
    RUN_TEST(test_overflow);
    RUN_TEST(test_bad_unclosed);
    RUN_TEST(test_bad_mismatch);
    RUN_TEST(test_bad_amp);
    return UNITY_END();
}
