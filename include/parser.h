#ifndef PARSER_H
#define PARSER_H

#define MAX_ENTRIES 64
#define MAX_KEY_LEN 256
#define MAX_VAL_LEN 256
#define MAX_DEPTH   16

typedef struct {
    char key[MAX_KEY_LEN];
    char value[MAX_VAL_LEN];
} XmlEntry;

typedef struct {
    XmlEntry entries[MAX_ENTRIES];
    int count;
} XmlMap;

int parse(const char *filepath, XmlMap *out);
const char *xml_get(const XmlMap *map, const char *key);

#endif /* PARSER_H */
