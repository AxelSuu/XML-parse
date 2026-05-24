#ifndef PARSER_H
#define PARSER_H

#define MAX_ENTRIES 256
#define MAX_KEY_LEN 256
#define MAX_VAL_LEN 256
#define MAX_DEPTH   16

typedef enum {
    XML_OK           =  1,
    XML_ERR_ARGS     = -1,
    XML_ERR_IO       = -2,
    XML_ERR_PARSE    = -3,
    XML_ERR_OVERFLOW = -4,
} XmlStatus;

typedef struct {
    char key[MAX_KEY_LEN];
    char value[MAX_VAL_LEN];
} XmlEntry;

typedef struct {
    int      capacity;           /* max entries to use; 0 = MAX_ENTRIES */
    int      count;
    XmlEntry entries[MAX_ENTRIES];
} XmlMap;

XmlStatus   parse(const char *filepath, XmlMap *out);
const char *xml_get(const XmlMap *map, const char *key);

#endif /* PARSER_H */
